#include "cc-impl.h"
#include "scc/cc/cc.h"
#include "scc/cc/llvm.h"
#include "scc/c-common/context.h"
#include "scc/core/file.h"
#include "scc/lex/lexer.h"
#include "scc/syntax/parser.h"
#include "scc/syntax/printer.h"
#include "scc/ssa/context.h"
#include "scc/ssa/pretty-print.h"
#include "scc/ssa-emit/emit.h"
#include "scc/ssa-optimize/optimize.h"
#include "scc/tree/context.h"
#include "scc/tree/target.h"
#include <stdarg.h>
#include <string.h>

#define LL_EXT "ll"
#define SSA_EXT "ssa"
#define OBJ_EXT "obj"
#define ASM_EXT "s"

static errcode cc_handle_pragma_link(void* cc, const char* lib)
{
        return cc_add_lib(cc, lib);
}

typedef struct
{
        tree_target_info target;
        tree_context tree;
        c_context c;
        c_error_handler eh;
        ssa_context ssa;
        cc_instance* instance;
} cc_context;

static void cc_handle_error(void* eh, c_error_severity severity, c_location loc, const char* err)
{
        static const char* c_error_severity_to_str[] =
        {
                "warning",
                "error",
                "fatal error",
        };

        cc_context* context = (cc_context*)((char*)eh - offsetof(cc_context, eh));
        fprintf(
                context->instance->output.message,
                "%s:%d:%d: %s: %s\n",
                pathfile(loc.file),
                loc.line,
                loc.column,
                c_error_severity_to_str[severity],
                err);
}

static void cc_context_init(cc_context* self, cc_instance* cc, jmp_buf on_fatal_error)
{
        self->instance = cc;

        tree_init_target_info(&self->target,
                cc->opts.target == CTK_X86_32 ? TTAK_X86_32 : TTAK_X86_64);

        tree_init(&self->tree, &self->target);

        self->eh.on_error = cc_handle_error;
        c_context_init(&self->c, &self->tree, &cc->input.source_lookup, &self->eh, on_fatal_error);
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
        return (file_entry**)vec_begin(&self->input.sources);
}

static file_entry** cc_sources_end(cc_instance* self)
{
        return (file_entry**)vec_end(&self->input.sources);
}

#define CC_FOREACH_SOURCE(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_sources_begin(PCC),\
                **ENDNAME = cc_sources_end(PCC); ITNAME != ENDNAME; ITNAME++)

static file_entry** cc_builtin_sources_begin(cc_instance* self)
{
        return (file_entry**)vec_begin(&self->input.builtin_sources);
}

static file_entry** cc_builtin_sources_end(cc_instance* self)
{
        return (file_entry**)vec_end(&self->input.builtin_sources);
}

#define CC_FOREACH_BUILTIN_SOURCE(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_builtin_sources_begin(PCC),\
                **ENDNAME = cc_builtin_sources_end(PCC); ITNAME != ENDNAME; ITNAME++)

static file_entry** cc_libs_begin(cc_instance* self)
{
        return (file_entry**)vec_begin(&self->input.libs);
}

static file_entry** cc_libs_end(cc_instance* self)
{
        return (file_entry**)vec_end(&self->input.libs);
}

#define CC_FOREACH_LIB(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_libs_begin(PCC),\
                **ENDNAME = cc_libs_end(PCC); ITNAME != ENDNAME; ITNAME++)

static file_entry** cc_obj_files_begin(cc_instance* self)
{
        return (file_entry**)vec_begin(&self->input.obj_files);
}

static file_entry** cc_obj_files_end(cc_instance* self)
{
        return (file_entry**)vec_end(&self->input.obj_files);
}

#define CC_FOREACH_OBJ_FILE(PCC, ITNAME, ENDNAME) \
        for(file_entry** ITNAME = cc_obj_files_begin(PCC),\
                **ENDNAME = cc_obj_files_end(PCC); ITNAME != ENDNAME; ITNAME++)

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
        if (self->input.sources.size > 1)
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
        printer->opts.print_semantic_init = self->opts.cprint.print_semantic_init;
}

static void cc_print_tokens(
        cc_instance* self,
        cc_context* context,
        FILE* output,
        const struct vec* tokens)
{
        c_printer printer;
        c_printer_init(&printer, &context->c, output);
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
        struct vec tokens;

        cc_context_init(&context, self, fatal);
        vec_init(&tokens);

        if (setjmp(fatal))
                goto cleanup;
        if (EC_FAILED(c_lex_source(&context.c, *cc_sources_begin(self), self->output.message, &tokens)))
                goto cleanup;

        result = EC_NO_ERROR;
        if (self->output.file)
                cc_print_tokens(self, &context, self->output.file, &tokens);

cleanup:
        vec_drop(&tokens);
        cc_context_dispose(&context);
        return result;
}

static void cc_print_tree_module(cc_instance* self,
        cc_context* context, FILE* output, const tree_module* module)
{
        c_printer printer;
        c_printer_init(&printer, &context->c, output);
        cc_set_cprinter_opts(self, &printer);
        c_print_module(&printer, module);
        c_printer_dispose(&printer);
}

static tree_module* cc_parse_file(cc_instance* self, cc_context* context, file_entry* file)
{
        c_pragma_handlers h = { .on_link = cc_handle_pragma_link, .data = self };
        return c_parse_source(&context->c, file, h, self->output.message);
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
        c_pragma_handlers h = { .on_link = cc_handle_pragma_link, .data = self };

        cc_context_init(&context, self, fatal);
        if (setjmp(fatal))
        {
                cc_context_dispose(&context);
                return EC_ERROR;
        }

        errcode result = EC_NO_ERROR;
        CC_FOREACH_SOURCE(self, it, end)
                if (!c_parse_source(&context.c, *it, h, self->output.message))
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
        opts->promote_allocas = self->opts.optimization.promote_allocas;
}

static errcode cc_codegen_file_ex(cc_instance* self, file_entry* file, bool emit_llvm_ir, FILE* output)
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

        ssa_implicitl_modules am;
        am.tm = NULL;
        if (self->opts.ext.enable_tm)
                if (!(am.tm = cc_parse_file(self, &context, self->input.tm_decls)))
                        goto cleanup;

        ssa_optimizer_opts opts;
        cc_set_ssa_optimizer_opts(self, &opts);
      
        ssa_module* sm = ssa_emit_module(&context.ssa, module, &opts, &am);
        if (!sm)
                goto cleanup;

        if (emit_llvm_ir)
                ssa_pretty_print_module_llvm(output, &context.ssa, sm);
        else
                ssa_pretty_print_module(output, &context.ssa, sm);
        
        result = EC_NO_ERROR;
cleanup:
        cc_context_dispose(&context);
        return result;
}

static void get_file_as(struct pathbuf* path, const file_entry* file, const char* ext)
{
        *path = pathbuf_from_str(file->path);
        char* ext_pos = (char*)pathext(path->buf);
        if (*ext_pos)
                strcpy(ext_pos, ext);
}

static errcode cc_codegen_file(cc_instance* self, file_entry* file, bool emit_llvm_ir)
{
        const char* ext = emit_llvm_ir ? LL_EXT : SSA_EXT;
        struct pathbuf path;
        get_file_as(&path, file, ext);

        FILE* fout = fopen(path.buf, "w");
        if (!fout)
                return EC_ERROR;

        errcode result = cc_codegen_file_ex(self, file, emit_llvm_ir, fout);
        fclose(fout);
        return result;
}

static void cc_cleanup_codegen(cc_instance* self, bool emit_llvm_ir)
{
        const char* ext = emit_llvm_ir ? LL_EXT : SSA_EXT;
        struct pathbuf file;
        CC_FOREACH_SOURCE(self, it, end)
        {
                get_file_as(&file, *it, ext);
                fs_delfile(file.buf);
        }
        CC_FOREACH_BUILTIN_SOURCE(self, it, end)
        {
                get_file_as(&file, *it, ext);
                fs_delfile(file.buf);
        }
}

static errcode cc_codegen(cc_instance* self, bool emit_llvm_ir)
{
        CC_FOREACH_SOURCE(self, it, end)
                if (EC_FAILED(cc_codegen_file(self, *it, emit_llvm_ir)))
                        goto cleanup;
        CC_FOREACH_BUILTIN_SOURCE(self, it, end)
                if (EC_FAILED(cc_codegen_file(self, *it, emit_llvm_ir)))
                        goto cleanup;

        return EC_NO_ERROR;

cleanup:
        cc_cleanup_codegen(self, emit_llvm_ir);
        return EC_ERROR;
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
        if (EC_FAILED(cc_codegen_file(self, file, true)))
                return EC_ERROR;

        struct pathbuf ll_file;
        get_file_as(&ll_file, file, LL_EXT);

        
        llvm_compiler llc;
        llvm_compiler_init(&llc, self->input.llc_path);
        llc.opt_level = self->opts.optimization.level > LCOL_O3 
                ? LCOL_O3 : self->opts.optimization.level;
        llc.file = ll_file.buf;
        llc.output_kind = output_kind;
        llc.arch = self->opts.target == CTK_X86_32 ? LCAK_X86 : LCAK_X86_64;
        llc.output = output;

        int exit_code = llvm_compile(&llc);
        fs_delfile(ll_file.buf);
        return cc_check_return_code(self, "llc", exit_code);
}

static void cc_cleanup_compilation(cc_instance* self, llvm_compiler_output_kind output_kind)
{
        const char* ext = output_kind == LCOK_ASM ? ASM_EXT : OBJ_EXT;
        struct pathbuf file;
        CC_FOREACH_SOURCE(self, it, end)
        {
                get_file_as(&file, *it, ext);
                fs_delfile(file.buf);
        }
        CC_FOREACH_BUILTIN_SOURCE(self, it, end)
        {
                get_file_as(&file, *it, ext);
                fs_delfile(file.buf);
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
        CC_FOREACH_BUILTIN_SOURCE(self, it, end)
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
                        *cc_sources_begin(self), false, self->output.file);
        }

        return cc_codegen(self, false);
}

extern errcode cc_generate_llvm_ir(cc_instance* self)
{
        if (self->output.file)
        {
                if (!cc_check_single_input(self))
                        return EC_ERROR;

                return cc_codegen_file_ex(self,
                        *cc_sources_begin(self), true, self->output.file);
        }

        return cc_codegen(self, true);
}

static errcode cc_link(cc_instance* self, llvm_linker* lld)
{
        struct pathbuf obj_file;
        CC_FOREACH_SOURCE(self, it, end)
        {
                get_file_as(&obj_file, *it, OBJ_EXT);
                llvm_linker_add_file(lld, obj_file.buf);
        }
        CC_FOREACH_BUILTIN_SOURCE(self, it, end)
        {
                get_file_as(&obj_file, *it, OBJ_EXT);
                llvm_linker_add_file(lld, obj_file.buf);
        }

        CC_FOREACH_OBJ_FILE(self, it, end)
                llvm_linker_add_file(lld, (*it)->path);

        CC_FOREACH_LIB(self, it, end)
                llvm_linker_add_file(lld, (*it)->path);

        FLOOKUP_FOREACH_DIR(&self->input.lib_lookup, it, end)
                llvm_linker_add_dir(lld, *it);

        int exit_code = llvm_link(lld);
        return cc_check_return_code(self, "lld", exit_code);
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
