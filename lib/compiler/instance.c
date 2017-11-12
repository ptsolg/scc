#include "scc/compiler/instance.h"
#include "scc/compiler/args.h"
#include "scc/c/c-env.h"
#include "scc/c/c-info.h"
#include "scc/c/c-printer.h"
#include "scc/scl/file.h"

static bool scc_add_handler(
        scc_instance* self,
        const char* arg,
        void(*const fn)(scc_arg_handler*, aparser*))
{
        scc_arg_handler* h = scc_new_arg_handler(get_std_alloc(), fn, self, arg);
        if (!h)
                return false;

        if (S_FAILED(aparser_add_handler(&self->parser, arg, &h->base))
         || S_FAILED(dseq_append_ptr(&self->handlers, h)))
        {
                deallocate(get_std_alloc(), h);
                return false;
        }

        return true;
}

static bool scc_add_default_handler(scc_instance* self)
{
        scc_arg_handler* h = scc_new_arg_handler(get_std_alloc(), &scc_default, self, NULL);
        if (!h)
                return false;

        aparser_add_default_handler(&self->parser, &h->base);
        return true;
}

static serrcode scc_init_args(scc_instance* self)
{
        return scc_add_default_handler(self)
                && scc_add_handler(self, "-syntax-only", &scc_syntax_only)
                && scc_add_handler(self, "-lex-only", &scc_lex_only)
                && scc_add_handler(self, "-log", &scc_log)
                && scc_add_handler(self, "-o", &scc_o)
                && scc_add_handler(self, "-i", &scc_i)
                && scc_add_handler(self, "-print-eval-result", &scc_print_eval_result)
                && scc_add_handler(self, "-print-expr-value", &scc_print_expr_value)
                && scc_add_handler(self, "-print-expr-type", &scc_print_expr_type)
                && scc_add_handler(self, "-print-impl-casts", &scc_print_impl_casts)
                && scc_add_handler(self, "-force-brackets", &scc_force_brackets)
                && scc_add_handler(self, "-float-precision", &scc_float_precision)
                && scc_add_handler(self, "-double-precision", &scc_double_precision)
                && scc_add_handler(self, "-x32", &scc_x32)
                && scc_add_handler(self, "-x64", &scc_x64)
                ? S_NO_ERROR : S_ERROR;
}

extern serrcode scc_init(scc_instance* self, FILE* err, int argc, const char** argv)
{
        dseq_init_ptr(&self->input);
        dseq_init_ptr(&self->handlers);
        dseq_init_ptr(&self->include);
        aparser_init(&self->parser, argc, argv);

        self->output = NULL;
        self->mode = SRM_SYNTAX_ONLY;
        self->err = err;
        self->return_code = S_NO_ERROR;
        self->opts.x32 = true;
        self->opts.print_expr_type = false;
        self->opts.print_expr_value = false;
        self->opts.force_brackets = false;
        self->opts.print_impl_casts = false;
        self->opts.print_eval_result = false;
        self->opts.float_precision = 4;
        self->opts.double_precision = 4;

        return scc_init_args(self);
}

static void scc_print_help(scc_instance* self)
{
        const char* d =
                "Options:\n"
                " -syntax-only       Syntax analysis only\n"
                " -lex-only          Lexical analysis only\n"
                " -log <file>        Specify the log file\n"
                " -o <file>          Specify the output file\n"
                " -i <dir>           Specify the include directory\n"
                " -print-eval-result Print constant expression value\n"
                " -print-expr-value  Print expression value (lvalue or rvalue)\n"
                " -print-expr-type   Print expression type\n"
                " -print-impl-casts  Print implicit casts\n"
                " -force-brackets    Force bracket printing\n"
                " -float-precision   Specify the float precision, when printing\n"
                " -double-precision  Specify the double precision, when printing\n"
                " -x32               Set X86 target architecture\n"
                " -x64               Set X86-64 target architecture\n"
                ;
        fprintf(self->err, "%s", d);
}

static FILE* scc_open_output(scc_instance* self)
{
        char path[S_MAX_PATH_LEN];
        path_get_cd(path);
        path_join(path, self->output);

        FILE* out = fopen(path, "w");
        if (!out)
                fprintf(self->err, "cannot open %s\n", self->output);
        return out;
}

static void scc_set_printer_opts(const scc_instance* self, cprinter_opts* opts)
{
        opts->double_precision = self->opts.double_precision;
        opts->float_precision = self->opts.float_precision;
        opts->force_brackets = self->opts.force_brackets;
        opts->print_eval_result = self->opts.print_eval_result;
        opts->print_expr_type = self->opts.print_expr_type;
        opts->print_expr_value = self->opts.print_expr_value;
        opts->print_impl_casts = self->opts.print_impl_casts;
}

static serrcode scc_init_cenv(scc_instance* self, cenv* env, jmp_buf* fatal)
{
        cenv_init(env, self->err, fatal);
        for (ssize i = 0; i < dseq_size(&self->include); i++)
                if (S_FAILED(cenv_add_lookup(env, dseq_get_ptr(&self->include, i))))
                        return S_ERROR;
        return S_NO_ERROR;
}

static serrcode _scc_perform_syntax_analysis(scc_instance* self, cenv* env)
{
        tree_target_info info;
        tree_init_target_info(&info, self->opts.x32 ? TTARGET_X32 : TTARGET_X64);
        tree_module* m = cparse_file(env, dseq_first_ptr(&self->input), &info);
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
                &env->id_policy,
                &env->source_manager,
                &info);

        fwrite_cb_init(&w, out);
        scc_set_printer_opts(self, &p.opts);
        cprint_module(&p, m);
        cprinter_dispose(&p);
        fclose(out);
        return S_NO_ERROR;
}

static serrcode scc_perform_syntax_analysis(scc_instance* self)
{
        jmp_buf fatal;
        if (setjmp(fatal))
        {
                fprintf(self->err, "Fatal error happened");
                return S_ERROR;
        }

        cenv env;
        if (S_FAILED(scc_init_cenv(self, &env, &fatal)))
                return S_ERROR;

        serrcode res = _scc_perform_syntax_analysis(self, &env);
        cenv_dispose(&env);
        return res;
}

static serrcode _scc_perform_lexical_analysis(scc_instance* self, cenv* env, dseq* tokens)
{
        if (S_FAILED(clex_file(env, dseq_first_ptr(&self->input), tokens)))
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
                &env->id_policy,
                &env->source_manager,
                NULL);

        fwrite_cb_init(&w, out);
        scc_set_printer_opts(self, &p.opts);
        cprint_tokens(&p, tokens);
        cprinter_dispose(&p);
        fclose(out);
        return S_NO_ERROR;
}

static serrcode scc_perform_lexical_analysis(scc_instance* self)
{
        jmp_buf fatal;
        if (setjmp(fatal))
        {
                fprintf(self->err, "Fatal error happened");
                return S_ERROR;
        }

        cenv env;
        dseq tokens;
        if (S_FAILED(scc_init_cenv(self, &env, &fatal)))
                return S_ERROR;

        dseq_init_ptr(&tokens);
        serrcode res = _scc_perform_lexical_analysis(self, &env, &tokens);
        dseq_dispose(&tokens);
        cenv_dispose(&env);
        return res;
}

extern serrcode scc_run(scc_instance* self)
{
        if (!aparser_args_remain(&self->parser))
        {
                scc_print_help(self);
                return S_NO_ERROR;
        }
        aparse(&self->parser);
        
        if (!dseq_size(&self->input))
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

extern void scc_dispose(scc_instance* self)
{
        fclose(self->err);
}

extern serrcode scc_add_lookup_directory(scc_instance* self, const char* dir)
{
        char* copy = allocate(get_std_alloc(), strlen(dir) + 1);
        if (!copy)
                return S_ERROR;

        strcpy(copy, dir);
        if (S_FAILED(dseq_append_ptr(&self->include, copy)))
        {
                deallocate(get_std_alloc(), copy);
                return S_NO_ERROR;
        }
        return S_NO_ERROR;
}

extern serrcode scc_add_input(scc_instance* self, const char* file)
{
        char* copy = allocate(get_std_alloc(), strlen(file) + 1);
        if (!copy)
                return S_ERROR;

        strcpy(copy, file);
        if (S_FAILED(dseq_append_ptr(&self->input, copy)))
        {
                deallocate(get_std_alloc(), copy);
                return S_NO_ERROR;
        }
        return S_NO_ERROR;
}