#include "cc-impl.h"
#include "scc/c/c-env.h"
#include "scc/c/c-context.h"
#include "scc/c/c-printer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/codegen/codegen.h"
#include "scc/cc/llvm.h"
#include <stdarg.h>

#define LL_EXT "ll"
#define SSA_EXT "ssa"
#if OS_WIN
#define OBJ_EXT "obj"
#elif OS_OSX
#define OBJ_EXT "o"
#else
#error
#endif
#define ASM_EXT "s"

typedef struct
{
        tree_target_info target;
        tree_context tree;
        c_context c;
        ssa_context ssa;
} cc_context;

static void cc_context_init(cc_context* self, cc_instance* cc, jmp_buf on_fatal_error)
{
        tree_init_target_info(&self->target,
                cc->opts.target == CTK_X86_32 ? TTAK_X86_32 : TTAK_X86_64);
        tree_init(&self->tree, &self->target);
        c_context_init(&self->c, &self->tree, &cc->input.source_lookup, on_fatal_error);
        self->c.lang_opts.ext.tm_enabled = cc->opts.ext.enable_tm;
        ssa_init(&self->ssa, &self->tree, on_fatal_error);
}

static void cc_context_dispose(cc_context* self)
{
        ssa_dispose(&self->ssa);
        c_context_dispose(&self->c);
        tree_dispose(&self->tree);
}

static file_entry** cc_sources_begin(cc_instance* self)
{
        return (file_entry**)dseq_begin(&self->input.sources);
}

static file_entry** cc_sources_end(cc_instance* self)
{
        return (file_entry**)dseq_end(&self->input.sources);
}

#define CC_FOREACH_SOURCE(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_sources_begin(PCC),\
                **ENDNAME = cc_sources_end(PCC); ITNAME != ENDNAME; ITNAME++)

static file_entry** cc_libs_begin(cc_instance* self)
{
        return (file_entry**)dseq_begin(&self->input.libs);
}

static file_entry** cc_libs_end(cc_instance* self)
{
        return (file_entry**)dseq_end(&self->input.libs);
}

#define CC_FOREACH_LIB(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_libs_begin(PCC),\
                **ENDNAME = cc_libs_end(PCC); ITNAME != ENDNAME; ITNAME++)

extern void cc_error(cc_instance* self, const char* format, ...)
{
        fprintf(self->output.message, "%s: error: ", self->opts.name);
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

static bool cc_check_single_input(cc_instance* self)
{
        if (dseq_size(&self->input.sources) > 1)
        {
                cc_error(self, "multiple input for single output");
                return false;
        }
        return true;
}

static void cc_set_cprinter_opts(cc_instance* self, c_printer* printer)
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
        FILE* output,
        const dseq* tokens)
{
        fwrite_cb write;
        fwrite_cb_init(&write, output);
        c_printer printer;
        c_printer_init(&printer, fwrite_cb_base(&write), &context->c);
        cc_set_cprinter_opts(self, &printer);
        c_print_tokens(&printer, tokens);
        c_printer_dispose(&printer);
}

extern errcode cc_dump_tokens(cc_instance* self)
{
        if (!cc_check_single_input(self))
                return EC_ERROR;

        errcode result = EC_ERROR;
        jmp_buf fatal;
        cc_context context;
        c_env env;
        dseq tokens;

        cc_context_init(&context, self, fatal);
        c_env_init(&env, &context.c, self->output.message);
        dseq_init_alloc(&tokens, self->alloc);

        if (setjmp(fatal))
                goto cleanup;
        if (EC_FAILED(c_env_lex_source(&env, *cc_sources_begin(self), &tokens)))
                goto cleanup;

        result = EC_NO_ERROR;
        if (self->output.file)
                cc_print_tokens(self, &context, self->output.file, &tokens);

cleanup:
        dseq_dispose(&tokens);
        c_env_dispose(&env);
        cc_context_dispose(&context);
        return result;
}

static void cc_print_tree_module(cc_instance* self,
        cc_context* context, FILE* output, const tree_module* module)
{
        fwrite_cb write;
        fwrite_cb_init(&write, output);
        c_printer printer;
        c_printer_init(&printer, fwrite_cb_base(&write), &context->c);
        cc_set_cprinter_opts(self, &printer);
        c_print_module(&printer, module);
        c_printer_dispose(&printer);
}

static tree_module* cc_parse_file(cc_instance* self, cc_context* context, file_entry* file)
{
        return c_parse_source(&context->c, file, self->output.message);
}

extern errcode cc_dump_tree(cc_instance* self)
{
        if (!cc_check_single_input(self))
                return EC_ERROR;

        errcode result = EC_ERROR;
        jmp_buf fatal;
        cc_context context;
        cc_context_init(&context, self, fatal);

        if (setjmp(fatal))
                goto cleanup;

        tree_module* module = cc_parse_file(self, &context, *cc_sources_begin(self));
        if (!module)
                goto cleanup;

        result = EC_NO_ERROR;
        if (self->output.file)
                cc_print_tree_module(self, &context, self->output.file, module);

cleanup:
        cc_context_dispose(&context);
        return result;
}

extern errcode cc_perform_syntax_analysis(cc_instance* self)
{
        jmp_buf fatal;
        cc_context context;

        cc_context_init(&context, self, fatal);
        if (setjmp(fatal))
        {
                cc_context_dispose(&context);
                return EC_ERROR;
        }

        errcode result = EC_NO_ERROR;
        CC_FOREACH_SOURCE(self, it, end)
                if (!c_parse_source(&context.c, *it, self->output.message))
                {
                        result = EC_ERROR;
                        break;
                }

        cc_context_dispose(&context);
        return result;
}

static void cc_set_ssa_optimizer_opts(cc_instance* self, ssa_optimizer_opts* opts)
{
        opts->eliminate_dead_code = self->opts.optimization.eliminate_dead_code;
        opts->fold_constants = self->opts.optimization.fold_constants;
}

static errcode cc_codegen_file_ex(cc_instance* self,
        codegen_output_kind kind, file_entry* file, FILE* output)
{
        errcode result = EC_ERROR;
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
        if (kind == CGOK_LLVM)
        {
                opts.fold_constants = true;
                opts.eliminate_dead_code = true;
        }

        result = codegen_module(fwrite_cb_base(&write), &context.ssa, module, kind, &opts);

cleanup:
        cc_context_dispose(&context);
        return result;
}

static errcode get_file_as(char* buffer, const file_entry* file, const char* ext)
{
        strncpy(buffer, file_get_path(file), MAX_PATH_LEN);
        return path_change_ext(buffer, ext);
}

static errcode cc_codegen_file(cc_instance* self,
        codegen_output_kind kind, file_entry* file)
{
        const char* ext = kind == CGOK_SSA ? SSA_EXT : LL_EXT;
        char out[MAX_PATH_LEN + 1];
        if (EC_FAILED(get_file_as(out, file, ext)))
                return EC_ERROR;

        FILE* fout = fopen(out, "w");
        if (!fout)
                return EC_ERROR;

        errcode result = cc_codegen_file_ex(self, kind, file, fout);
        fclose(fout);
        return result;
}

static void cc_cleanup_codegen(cc_instance* self, codegen_output_kind kind)
{
        CC_FOREACH_SOURCE(self, it, end)
        {
                const char* ext = kind == CGOK_SSA ? SSA_EXT : LL_EXT;
                char file[MAX_PATH_LEN + 1];
                if (EC_FAILED(get_file_as(file, *it, ext)))
                        return;

                path_delete_file(file);
        }
}

static errcode cc_codegen(cc_instance* self, codegen_output_kind kind)
{
        CC_FOREACH_SOURCE(self, it, end)
                if (EC_FAILED(cc_codegen_file(self, kind, *it)))
                {
                        cc_cleanup_codegen(self, kind);
                        return EC_ERROR;
                }
        return EC_NO_ERROR;
}

static errcode cc_check_return_code(cc_instance* self, const char* tool, int code)
{
        if (code)
        {
                cc_error(self, "%s returned %d", tool, code);
                return EC_ERROR;
        }
        return EC_NO_ERROR;
}

static errcode cc_compile_file(cc_instance* self,
        file_entry* file, llvm_compiler_output_kind output_kind, const char* output)
{
        if (EC_FAILED(cc_codegen_file(self, CGOK_LLVM, file)))
                return EC_ERROR;

        char ll_file[MAX_PATH_LEN + 1];
        if (EC_FAILED(get_file_as(ll_file, file, LL_EXT)))
                return EC_ERROR;

        int exit_code;
        llvm_compiler llc;
        llvm_compiler_init(&llc, self->input.llc_path);
        // todo: set optimization opts
        llc.file = ll_file;
        llc.output_kind = output_kind;
        llc.arch = self->opts.target == CTK_X86_32 ? LCAK_X86 : LCAK_X86_64;
        llc.output = output;

        bool failed = EC_FAILED(llvm_compile(&llc, &exit_code));
        path_delete_file(ll_file);
        if (failed)
                return EC_ERROR;

        return cc_check_return_code(self, "llc", exit_code);
}

static void cc_cleanup_compilation(cc_instance* self, llvm_compiler_output_kind output_kind)
{
        const char* ext = output_kind == LCOK_ASM ? ASM_EXT : OBJ_EXT;
        CC_FOREACH_SOURCE(self, it, end)
        {
                char file[MAX_PATH_LEN + 1];
                strncpy(file, file_get_path(*it), MAX_PATH_LEN);
                if (EC_FAILED(path_change_ext(file, ext)))
                        continue;

                path_delete_file(file);
        }
}

static errcode cc_compile(cc_instance* self, llvm_compiler_output_kind output_kind)
{
        CC_FOREACH_SOURCE(self, it, end)
                if (EC_FAILED(cc_compile_file(self, *it, output_kind, NULL)))
                {
                        cc_cleanup_compilation(self, output_kind);
                        return EC_ERROR;
                }
        return EC_NO_ERROR;
}

static void cc_close_output_stream(cc_instance* self)
{
        if (self->output.file)
        {
                fclose(self->output.file);
                self->output.file = NULL;
        }
}

extern errcode cc_generate_obj(cc_instance* self)
{
        if (self->output.file)
        {
                if (!cc_check_single_input(self))
                        return EC_ERROR;

                cc_close_output_stream(self);
                return cc_compile_file(self,
                        *cc_sources_begin(self), LCOK_OBJ, self->output.file_path);
        }

        return cc_compile(self, LCOK_OBJ);
}

extern errcode cc_generate_asm(cc_instance* self)
{
        if (self->output.file)
        {
                if (!cc_check_single_input(self))
                        return EC_ERROR;

                cc_close_output_stream(self);
                return cc_compile_file(self,
                        *cc_sources_begin(self), LCOK_ASM, self->output.file_path);
        }

        return cc_compile(self, LCOK_ASM);
}

extern errcode cc_generate_ssa(cc_instance* self)
{
        if (self->output.file)
        {
                if (!cc_check_single_input(self))
                        return EC_ERROR;

                return cc_codegen_file_ex(self,
                        CGOK_SSA, *cc_sources_begin(self), self->output.file);
        }

        return cc_codegen(self, CGOK_SSA);
}

extern errcode cc_generate_llvm_ir(cc_instance* self)
{
        if (self->output.file)
        {
                if (!cc_check_single_input(self))
                        return EC_ERROR;

                return cc_codegen_file_ex(self,
                        CGOK_LLVM, *cc_sources_begin(self), self->output.file);
        }

        return cc_codegen(self, CGOK_LLVM);
}

static errcode cc_link(cc_instance* self, llvm_linker* lld)
{
        char obj_file[MAX_PATH_LEN + 1];
        CC_FOREACH_SOURCE(self, it, end)
        {
                if (EC_FAILED(get_file_as(obj_file, *it, OBJ_EXT)))
                        return EC_ERROR;
                if (EC_FAILED(llvm_linker_add_file(lld, obj_file)))
                        return EC_ERROR;
        }

        CC_FOREACH_LIB(self, it, end)
                if (EC_FAILED(llvm_linker_add_file(lld, file_get_path(*it))))
                        return EC_ERROR;

        FLOOKUP_FOREACH_DIR(&self->input.lib_lookup, it, end)
                if (EC_FAILED(llvm_linker_add_dir(lld, *it)))
                        return EC_ERROR;

        int code;
        if (EC_FAILED(llvm_link(lld, &code)))
                return EC_ERROR;

        return cc_check_return_code(self, "lld", code);
}

extern errcode cc_generate_exec(cc_instance* self)
{
        cc_close_output_stream(self);
        if (EC_FAILED(cc_compile(self, LCOK_OBJ)))
                return EC_ERROR;

        llvm_linker lld;
        llvm_linker_init(&lld, self->input.lld_path);
        lld.entry = self->input.entry;
        lld.output = self->output.file_path;
        errcode result = cc_link(self, &lld);
        cc_cleanup_compilation(self, LCOK_OBJ);
        llvm_linker_dispose(&lld);
        return result;
}