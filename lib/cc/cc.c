#include "scc/cc/cc.h"
#include "cc-impl.h"

extern void cc_init(cc_instance* self, FILE* message)
{
        cc_init_ex(self, message, STDALLOC);
}

extern void cc_init_ex(cc_instance* self, FILE* message, allocator* alloc)
{
        self->alloc = alloc;
        self->opts.name = "cc";

        self->input.llc_path = NULL;
        self->input.lld_path = NULL;
        self->input.entry = NULL;
        dseq_init_alloc(&self->input.sources, alloc);
        dseq_init_alloc(&self->input.libs, alloc);
        flookup_init_ex(&self->input.source_lookup, alloc);
        flookup_init_ex(&self->input.lib_lookup, alloc);

        self->output.kind = COK_EXEC;
        self->output.message = message;
        self->output.file = NULL;
        self->output.file_path = NULL;

        self->opts.target = CTK_X86_32;
        self->opts.optimization.eliminate_dead_code = false;
        self->opts.optimization.fold_constants = false;
        self->opts.cprint.print_expr_value = false;
        self->opts.cprint.print_expr_type = false;
        self->opts.cprint.print_impl_casts = false;
        self->opts.cprint.print_eval_result = false;
        self->opts.cprint.force_brackets = false;
}

extern void cc_dispose(cc_instance* self)
{
        if (self->output.file)
                fclose(self->output.file);

        flookup_dispose(&self->input.lib_lookup);
        flookup_dispose(&self->input.source_lookup);
        dseq_dispose(&self->input.libs);
        dseq_dispose(&self->input.sources);
}

extern void cc_set_output_stream(cc_instance* self, FILE* out)
{
        self->output.file_path = NULL;
        self->output.file = out;
}

extern serrcode cc_set_output_file(cc_instance* self, const char* file)
{
        if (self->output.file)
                fclose(self->output.file);

        if (!(self->output.file = fopen(file, "w")))
        {
                cc_unable_to_open(self, file);
                return S_ERROR;
        }

        self->output.file_path = file;
        return S_NO_ERROR;
}

extern serrcode cc_add_lib_dir(cc_instance* self, const char* dir)
{
        return flookup_add(&self->input.lib_lookup, dir);
}

extern serrcode cc_add_lib(cc_instance* self, const char* lib)
{
        file_entry* file = file_get(&self->input.lib_lookup, lib);
        if (!file)
        {
                cc_file_doesnt_exit(self, lib);
                return S_ERROR;
        }

        dseq_append(&self->input.libs, file);
        return S_NO_ERROR;
}

extern serrcode cc_add_source_dir(cc_instance* self, const char* dir)
{
        return flookup_add(&self->input.source_lookup, dir);
}

extern serrcode cc_add_source_file(cc_instance* self, const char* file)
{
        file_entry* source = file_get(&self->input.source_lookup, file);
        if (!source)
        {
                cc_file_doesnt_exit(self, file);
                return S_ERROR;
        }

        dseq_append(&self->input.sources, source);
        return S_NO_ERROR;
}

extern serrcode cc_emulate_source_file(
        cc_instance* self, const char* file, const char* content)
{
        file_entry* source = file_emulate(&self->input.source_lookup, file, content);
        if (!source)
                return S_ERROR;

         dseq_append(&self->input.sources, source);
        return S_NO_ERROR;
}

extern serrcode cc_run(cc_instance* self)
{
        if (!dseq_size(&self->input.sources))
        {
                cc_error(self, "no input files");
                return S_ERROR;
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
        return S_ERROR;
}