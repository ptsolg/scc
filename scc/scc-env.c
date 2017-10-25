#include "scc-env.h"
#include "scc-args.h"

#include <libc/c-env.h>
#include <libc/c-info.h>
#include <libc/c-printer.h>
#include <libscl/file.h>

static bool scc_add_handler(
        scc_env*    self,
        const char* arg,
        void(*const fn)(scc_arg_handler*, aparser*),
        const char* description)
{
        scc_arg_handler* h = scc_new_arg_handler(get_std_alloc(), fn, self, arg, description);
        if (!h)
                return false;

        if (S_FAILED(aparser_add_handler(&self->parser, arg, &h->base))
         || S_FAILED(objgroup_push_back(&self->handlers, h)))
        {
                deallocate(get_std_alloc(), h);
                return false;
        }

        return true;
}

static bool scc_add_default_handler(scc_env* self)
{
        scc_arg_handler* h = scc_new_arg_handler(get_std_alloc(), &scc_default, self, NULL, NULL);
        if (!h)
                return false;

        aparser_add_default_handler(&self->parser, &h->base);
        return true;
}

static serrcode scc_init_args(scc_env* self)
{
        return scc_add_default_handler(self)
                && scc_add_handler(self, "-syntax-only",      &scc_syntax_only,      "Syntax analysis only")
                && scc_add_handler(self, "-lex-only",         &scc_lex_only,         "Lexical analysis only")
                && scc_add_handler(self, "-log",              &scc_log,              "Specify the log file")
                && scc_add_handler(self, "-o",                &scc_o,                "Specify the output file")
                && scc_add_handler(self, "-i",                &scc_i,                "Specify the include directory")
                && scc_add_handler(self, "-print-exp-value",  &scc_print_exp_value,  "Print expression value")
                && scc_add_handler(self, "-print-exp-type",   &scc_print_exp_type,   "Print expression type")
                && scc_add_handler(self, "-print-impl-casts", &scc_print_impl_casts, "Print implicit casts")
                && scc_add_handler(self, "-force-brackets",   &scc_force_brackets,   "Force bracket printing")
                && scc_add_handler(self, "-float-precision",  &scc_float_precision,  "Specify the float precision, when printing")
                && scc_add_handler(self, "-double-precision", &scc_double_precision, "Specify the double precision, when printing")
                && scc_add_handler(self, "-x32",              &scc_x32,              "Compile as x32")
                && scc_add_handler(self, "-x64",              &scc_x64,              "Compile as x64")
                ? S_NO_ERROR : S_ERROR;
}

extern serrcode scc_init(scc_env* self, FILE* err, int argc, const char** argv)
{
        objgroup_init(&self->input);
        objgroup_init(&self->handlers);
        objgroup_init(&self->include);
        aparser_init(&self->parser, argc, argv);

        self->output                = NULL;
        self->mode                  = SRM_SYNTAX_ONLY;
        self->err                   = err;
        self->return_code           = S_NO_ERROR;
        self->opts.x32              = true;
        self->opts.print_exp_type   = false;
        self->opts.print_exp_value  = false;
        self->opts.force_brackets   = false;
        self->opts.print_impl_casts = false;
        self->opts.float_precision  = 4;
        self->opts.double_precision = 4;

        return scc_init_args(self);
}

static void scc_print_help(scc_env* self)
{
        fprintf(self->err, "Options:\n");
        ssize max_len = 0;
        OBJGROUP_FOREACH(&self->handlers, scc_arg_handler**, it)
        {
                ssize len = strlen((*it)->arg);
                if (len > max_len)
                        max_len = len;
        }
        
        OBJGROUP_FOREACH(&self->handlers, scc_arg_handler**, it)
        {
                char spaces[100];
                ssize len = strlen((*it)->arg);
                sstrfill(spaces, ' ', max_len - len);

                fprintf(self->err, "  %s%s    %s\n", (*it)->arg, spaces, (*it)->description);
        }
}

static FILE* scc_open_output(scc_env* self)
{
        char path[S_MAX_PATH_LEN];
        path_get_cd(path);
        path_join(path, self->output);

        FILE* out = fopen(path, "w");
        if (!out)
                fprintf(self->err, "cannot open %s\n", self->output);
        return out;
}

static void scc_set_printer_opts(const scc_env* self, cprinter_opts* opts)
{
        opts->double_precision = self->opts.double_precision;
        opts->float_precision  = self->opts.float_precision;
        opts->force_brackets   = self->opts.force_brackets;
        opts->print_exp_type   = self->opts.print_exp_type;
        opts->print_exp_value  = self->opts.print_exp_value;
        opts->print_impl_casts = self->opts.print_impl_casts;
}

static serrcode scc_init_cenv(scc_env* self, cenv* env)
{
        cenv_init(env, self->err);
        OBJGROUP_FOREACH(&self->include, char**, it)
                if (S_FAILED(cenv_add_lookup(env, *it)))
                        return S_ERROR;
        return S_NO_ERROR;
}

static serrcode _scc_perform_syntax_analysis(scc_env* self, cenv* env)
{
        tree_platform_info info;
        tree_init_platform_info(&info, self->opts.x32 ? TPK_X32 : TPK_X64);
        tree_module* m = cparse_file(env, objgroup_first(&self->input), &info);
        if (!m)
                return S_ERROR;
        if (!self->output)
                return S_NO_ERROR;

        FILE* out = scc_open_output(self);
        if (!out)
                return S_ERROR;

        cprinter p;
        fwrite_cb w;
        cprinter_init(
                &p,
                fwrite_cb_base(&w),
                ctree_context_base(&env->context),
                &env->source_manager);
        fwrite_cb_init(&w, out);
        scc_set_printer_opts(self, &p.opts);
        cprint_module(&p, m);
        cprinter_dispose(&p);
        fclose(out);
        return S_NO_ERROR;
}

static serrcode scc_perform_syntax_analysis(scc_env* self)
{
        cenv env;
        if (S_FAILED(scc_init_cenv(self, &env)))
                return S_ERROR;

        serrcode res = _scc_perform_syntax_analysis(self, &env);
        cenv_dispose(&env);
        return res;
}

static serrcode _scc_perform_lexical_analysis(scc_env* self, cenv* env, objgroup* tokens)
{
        if (S_FAILED(clex_file(env, objgroup_first(&self->input), tokens)))
                return S_ERROR;
        if (!self->output)
                return S_NO_ERROR;

        FILE* out = scc_open_output(self);
        if (!out)
                return S_ERROR;

        cprinter p;
        fwrite_cb w;
        cprinter_init(
                &p,
                fwrite_cb_base(&w),
                ctree_context_base(&env->context),
                &env->source_manager);
        fwrite_cb_init(&w, out);
        scc_set_printer_opts(self, &p.opts);
        cprint_tokens(&p, tokens);
        cprinter_dispose(&p);
        fclose(out);
        return S_NO_ERROR;
}

static serrcode scc_perform_lexical_analysis(scc_env* self)
{
        cenv env;
        objgroup tokens;
        if (S_FAILED(scc_init_cenv(self, &env)))
                return S_ERROR;
        objgroup_init(&tokens);
        serrcode res = _scc_perform_lexical_analysis(self, &env, &tokens);
        objgroup_dispose(&tokens);
        cenv_dispose(&env);
        return res;
}

extern serrcode scc_run(scc_env* self)
{
        if (!aparser_args_remain(&self->parser))
        {
                scc_print_help(self);
                return S_NO_ERROR;
        }
        aparse(&self->parser);
        
        if (!objgroup_size(&self->input))
        {
                fprintf(self->err, "Please specify input file.\n");
                return S_ERROR;
        }

        if (self->mode == SRM_SYNTAX_ONLY)
                return scc_perform_syntax_analysis(self);
        else if (self->mode == SRM_LEX_ONLY)
                return scc_perform_lexical_analysis(self);
        else
                ;

        
        return self->return_code;
}

extern void scc_dispose(scc_env* self)
{
        fclose(self->err);
}

extern serrcode scc_add_lookup_directory(scc_env* self, const char* dir)
{
        char* copy = allocate(get_std_alloc(), strlen(dir) + 1);
        if (!copy)
                return S_ERROR;

        strcpy(copy, dir);
        if (S_FAILED(objgroup_push_back(&self->include, copy)))
        {
                deallocate(get_std_alloc(), copy);
                return S_NO_ERROR;
        }
        return S_NO_ERROR;
}

extern serrcode scc_add_input(scc_env* self, const char* file)
{
        char* copy = allocate(get_std_alloc(), strlen(file) + 1);
        if (!copy)
                return S_ERROR;

        strcpy(copy, file);
        if (S_FAILED(objgroup_push_back(&self->input, copy)))
        {
                deallocate(get_std_alloc(), copy);
                return S_NO_ERROR;
        }
        return S_NO_ERROR;
}