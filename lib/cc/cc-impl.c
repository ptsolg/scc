#include "cc-impl.h"
#include "scc/c/c-env.h"
#include "scc/c/c-tree.h"
#include "scc/c/c-printer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/codegen/codegen.h"
#include <stdarg.h>

typedef struct
{
        tree_target_info target;
        tree_context tree;
        ccontext c;
        ssa_context ssa;
} cc_context;

static void cc_context_init(cc_context* self, cc_instance* cc, jmp_buf on_fatal_error)
{
        tree_init_target_info(&self->target,
                cc->opts.target == CTK_X86_32 ? TTARGET_X32 : TTARGET_X64);
        tree_init(&self->tree, &self->target);
        cinit(&self->c, &self->tree, on_fatal_error);
        ssa_init(&self->ssa, &self->tree, on_fatal_error);
}

static void cc_context_dispose(cc_context* self)
{
        ssa_dispose(&self->ssa);
        cdispose(&self->c);
        tree_dispose(&self->tree);
}

static file_entry** cc_sources_begin(cc_instance* self)
{
        return (file_entry**)dseq_begin_ptr(&self->input.sources);
}

static file_entry** cc_sources_end(cc_instance* self)
{
        return (file_entry**)dseq_end_ptr(&self->input.sources);
}

#define CC_FOREACH_SOURCE(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_sources_begin(PCC),\
                **ENDNAME = cc_sources_end(PCC); ITNAME != ENDNAME; ITNAME++)

static file_entry** cc_libs_begin(cc_instance* self)
{
        return (file_entry**)dseq_begin_ptr(&self->input.libs);
}

static file_entry** cc_libs_end(cc_instance* self)
{
        return (file_entry**)dseq_end_ptr(&self->input.libs);
}

extern void cc_error(cc_instance* self, const char* format, ...)
{
        fprintf(self->output.message, "scc: error: ");
        va_list args;
        va_start(args, format);
        vfprintf(self->output.message, format, args);
        fprintf(self->output.message, "\n");
}

extern void cc_unable_to_open(cc_instance* self, const char* path)
{
        cc_error(self, "unable to open '%s'", path);
}

extern void cc_file_doesnt_exit(cc_instance* self, const char* file)
{
        cc_error(self, "no such file or directory '%s'", file);
}

extern FILE* cc_open_file(cc_instance* self, const char* file, const char* mode)
{
        FILE* f = fopen(file, mode);
        if (f)
                return f;

        char path[S_MAX_PATH_LEN + 1];
        if (S_FAILED(path_get_cd(path)
                || S_FAILED(path_join(path, file))) || !(f = fopen(path, mode)))
        {
                cc_file_doesnt_exit(self, file);
                return NULL;
        }

        return f;
}

static bool cc_check_single_input(cc_instance* self)
{
        if (dseq_size(&self->input.sources) > 1)
        {
                cc_error(self, "multiple input for single output");
                return false;
        }
        return true;
}

static void cc_set_cprinter_opts(cc_instance* self, cprinter* printer)
{
        printer->opts.print_eval_result = self->opts.cprint.print_eval_result;
        printer->opts.print_expr_type = self->opts.cprint.print_expr_type;
        printer->opts.print_expr_value = self->opts.cprint.print_expr_value;
        printer->opts.print_impl_casts = self->opts.cprint.print_impl_casts;
        printer->opts.force_brackets = self->opts.cprint.force_brackets;
}

static void cc_print_tokens(
        cc_instance* self,
        cc_context* context,
        csource_manager* source_manager,
        FILE* output,
        const dseq* tokens)
{
        fwrite_cb write;
        fwrite_cb_init(&write, output);
        cprinter printer;
        cprinter_init(&printer, fwrite_cb_base(&write), &context->c, source_manager);
        cc_set_cprinter_opts(self, &printer);
        cprint_tokens(&printer, tokens);
        cprinter_dispose(&printer);
}

extern serrcode cc_dump_tokens(cc_instance* self)
{
        if (!cc_check_single_input(self))
                return S_ERROR;

        serrcode result = S_ERROR;
        jmp_buf fatal;
        cc_context context;
        cenv env;
        dseq tokens;

        cc_context_init(&context, self, fatal);
        cenv_init(&env, &context.c, &self->input.source_lookup, self->output.message);
        dseq_init_ex_ptr(&tokens, self->alloc);

        if (setjmp(fatal))
                goto cleanup;
        if (S_FAILED(cenv_lex_source(&env, *cc_sources_begin(self), &tokens)))
        {
                goto cleanup;
        }

        result = S_NO_ERROR;
        if (self->output.file)
                cc_print_tokens(self, &context, &env.source_manager, 
                        self->output.file, &tokens);

cleanup:
        dseq_dispose(&tokens);
        cenv_dispose(&env);
        cc_context_dispose(&context);
        return result;
}

static void cc_print_tree_module(cc_instance* self,
        cc_context* context, FILE* output, const tree_module* module)
{
        fwrite_cb write;
        fwrite_cb_init(&write, output);
        cprinter printer;
        cprinter_init(&printer, fwrite_cb_base(&write), &context->c, NULL);
        cc_set_cprinter_opts(self, &printer);
        cprint_module(&printer, module);
        cprinter_dispose(&printer);
}

static tree_module* cc_parse_file(cc_instance* self, cc_context* context, file_entry* file)
{
        return cparse_source(&context->c,
                &self->input.source_lookup, file, self->output.message);
}

extern serrcode cc_dump_tree(cc_instance* self)
{
        if (!cc_check_single_input(self))
                return S_ERROR;

        serrcode result = S_ERROR;
        jmp_buf fatal;
        cc_context context;
        cc_context_init(&context, self, fatal);

        if (setjmp(fatal))
                goto cleanup;

        tree_module* module = cc_parse_file(self, &context, *cc_sources_begin(self));
        if (!module)
                goto cleanup;

        result = S_NO_ERROR;
        if (self->output.file)
                cc_print_tree_module(self, &context, self->output.file, module);

cleanup:
        cc_context_dispose(&context);
        return result;
}

extern serrcode cc_perform_syntax_analysis(cc_instance* self)
{
        jmp_buf fatal;
        cc_context context;

        cc_context_init(&context, self, fatal);
        if (setjmp(fatal))
        {
                cc_context_dispose(&context);
                return S_ERROR;
        }

        serrcode result = S_NO_ERROR;
        CC_FOREACH_SOURCE(self, it, end)
                if (!cparse_source(&context.c,
                        &self->input.source_lookup, *it, self->output.message))
                {
                        result = S_ERROR;
                        break;
                }

        cc_context_dispose(&context);
        return result;
}

extern serrcode cc_generate_obj(cc_instance* self)
{
        return S_ERROR;
}

extern serrcode cc_generate_asm(cc_instance* self)
{
        return S_ERROR;
}

static void cc_set_ssa_optimizer_opts(cc_instance* self, ssa_optimizer_opts* opts)
{
        opts->eliminate_dead_code = self->opts.optimization.eliminate_dead_code;
        opts->fold_constants = self->opts.optimization.fold_constants;
}

static serrcode cc_codegen_file(cc_instance* self,
        codegen_output_kind kind, file_entry* file, FILE* output)
{
        serrcode result = S_ERROR;
        jmp_buf fatal;
        cc_context context;

        cc_context_init(&context, self, fatal);
        if (setjmp(fatal))
                goto cleanup;

        tree_module* module = cc_parse_file(self, &context, file);
        if (!module)
                goto cleanup;

        fwrite_cb write;
        fwrite_cb_init(&write, output);

        ssa_optimizer_opts opts;
        cc_set_ssa_optimizer_opts(self, &opts);

        result = codegen_module(fwrite_cb_base(&write), &context.ssa, module, kind, &opts);

cleanup:
        cc_context_dispose(&context);
        return result;
}

static serrcode cc_codegen(cc_instance* self, codegen_output_kind kind)
{
        if (self->output.file)
        {
                if (!cc_check_single_input(self))
                        return S_ERROR;

                return cc_codegen_file(self, kind,
                        *cc_sources_begin(self), self->output.file);
        }

        const char* ext = kind == CGOK_SSA ? "ssa" : "ll";
        CC_FOREACH_SOURCE(self, it, end)
        {
                file_entry* file = *it;
                char path[S_MAX_PATH_LEN + 1];
                strncpy(path, file_get_path(file), S_MAX_PATH_LEN);
                if (S_FAILED(path_change_ext(path, ext)))
                        return S_ERROR;
                
                FILE* output = fopen(path, "w");
                if (!output)
                        return S_ERROR;

                serrcode result = cc_codegen_file(self, kind, file, output);
                fclose(output);
                if (S_FAILED(result))
                        return S_ERROR;
        }
        return S_NO_ERROR;
}

extern serrcode cc_generate_ssa(cc_instance* self)
{
        return cc_codegen(self, CGOK_SSA);
}

extern serrcode cc_generate_llvm_ir(cc_instance* self)
{
        return cc_codegen(self, CGOK_LLVM);
}

extern serrcode cc_generate_exec(cc_instance* self)
{
        return S_ERROR;
}