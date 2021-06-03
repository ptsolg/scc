#include "scc/cc/cc.h"
#include "cc-impl.h"
#include "scc/core/common.h"
#include <string.h>

extern void cc_init(cc_instance* self, FILE* message)
{
        self->opts.name = "cc";

        self->input.llc_path = NULL;
        self->input.lld_path = NULL;
        self->input.entry = NULL;
        self->input.tm_decls = NULL;
        vec_init(&self->input.sources);
        vec_init(&self->input.builtin_sources);
        vec_init(&self->input.libs);
        vec_init(&self->input.implicit_libs);
        vec_init(&self->input.obj_files);
        flookup_init(&self->input.source_lookup);
        flookup_init(&self->input.lib_lookup);

        self->output.kind = COK_EXEC;
        self->output.message = message;
        self->output.file = NULL;
        self->output.file_path = NULL;

        self->opts.target = CTK_X86_32;
        self->opts.optimization.eliminate_dead_code = false;
        self->opts.optimization.fold_constants = false;
        self->opts.optimization.promote_allocas = false;
        self->opts.optimization.level = 0;
        self->opts.cprint.print_expr_value = false;
        self->opts.cprint.print_expr_type = false;
        self->opts.cprint.print_impl_casts = false;
        self->opts.cprint.print_eval_result = false;
        self->opts.cprint.force_brackets = false;
        self->opts.cprint.print_semantic_init = false;
        self->opts.linker.emit_debug_info = false;
        self->opts.ext.enable_tm = false;
}

extern void cc_dispose(cc_instance* self)
{
        if (self->output.file)
                fclose(self->output.file);

        flookup_dispose(&self->input.lib_lookup);
        flookup_dispose(&self->input.source_lookup);

        for (int i = 0; i < self->input.libs.size; i++)
                dealloc(self->input.libs.items[i]);
        for (int i = 0; i < self->input.implicit_libs.size; i++)
                dealloc(self->input.implicit_libs.items[i]);
        vec_drop(&self->input.libs);
        vec_drop(&self->input.implicit_libs);

        vec_drop(&self->input.sources);
        vec_drop(&self->input.builtin_sources);
        vec_drop(&self->input.obj_files);
}

extern void cc_set_output_stream(cc_instance* self, FILE* out)
{
        self->output.file_path = NULL;
        self->output.file = out;
}

extern errcode cc_set_output_file(cc_instance* self, const char* file)
{
        if (self->output.file)
                fclose(self->output.file);

        if (!(self->output.file = fopen(file, "w")))
        {
                cc_unable_to_open(self, file);
                return EC_ERROR;
        }

        self->output.file_path = file;
        return EC_NO_ERROR;
}

extern void cc_add_lib_dir(cc_instance* self, const char* dir)
{
        flookup_add(&self->input.lib_lookup, dir);
}

extern errcode cc_add_lib(cc_instance* self, const char* lib, bool is_implicit)
{
        const char* default_ext = ".lib";
        char* s = alloc(strlen(lib) + strlen(default_ext) + 1);
        strcpy(s, lib);
        if (!*pathext(lib))
                strcat(s, default_ext);

        vec_push(is_implicit ? &self->input.implicit_libs : &self->input.libs, s);
        return EC_NO_ERROR;
}

extern void cc_add_source_dir(cc_instance* self, const char* dir)
{
        flookup_add(&self->input.source_lookup, dir);
}

extern errcode cc_add_source_file(cc_instance* self, const char* file, bool builtin)
{
        file_entry* source = file_get(&self->input.source_lookup, file);
        if (!source)
        {
                cc_file_doesnt_exit(self, file);
                return EC_ERROR;
        }

        vec_push(builtin ? &self->input.builtin_sources : &self->input.sources, source);
        return EC_NO_ERROR;
}

extern errcode cc_add_obj_file(cc_instance* self, const char* file)
{
        file_entry* obj = file_get(&self->input.source_lookup, file);
        if (!obj)
        {
                cc_file_doesnt_exit(self, file);
                return EC_ERROR;
        }

        vec_push(&self->input.obj_files, obj);
        return EC_NO_ERROR;
}

extern errcode cc_emulate_source_file(
        cc_instance* self, const char* file, const char* content, bool builtin, bool add_to_input)
{
        file_entry* source = file_emulate(&self->input.source_lookup, file, content);
        if (!source)
                return EC_ERROR;

        if (add_to_input)
                vec_push(builtin ? &self->input.builtin_sources : &self->input.sources, source);

        return EC_NO_ERROR;
}

extern errcode cc_run(cc_instance* self)
{
        assert(self->opts.ext.enable_tm == (bool)self->input.tm_decls);

        if (!self->input.sources.size
                && !self->input.obj_files.size
                && !self->input.libs.size)
        {
                cc_error(self, "no input files");
                return EC_ERROR;
        }

        switch (self->output.kind)
        {
                case COK_NONE: return cc_perform_syntax_analysis(self);
                case COK_EXEC: return cc_generate_exec(self);
                case COK_OBJ: return cc_generate_obj(self);
                case COK_LEXEMES: return cc_dump_tokens(self);
                case COK_C: return cc_dump_tree(self);
                case COK_SSA: return cc_generate_ssa(self);
                case COK_ASM: return cc_generate_asm(self);
                case COK_LLVM_IR: return cc_generate_llvm_ir(self);
        }
        return EC_ERROR;
}
