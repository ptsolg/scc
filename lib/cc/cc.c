#include "scc/cc/cc.h"
#include "scc/cc/tool.h"
#include "scc/cc/cc-run.h"
#include <stdarg.h>

static void* scc_cc_on_out_of_memory(base_allocator* alloc, ssize bytes, ssize align)
{
        scc_cc* self = (scc_cc*)((char*)alloc - offsetof(scc_cc, alloc));
        scc_cc_error(self, "unable to allocate memory");
        return NULL;
}

extern allocator* scc_cc_alloc(scc_cc* self)
{
        return base_allocator_base(&self->alloc);
}

static void* scc_cc_allocate(scc_cc* self, ssize bytes)
{
        return allocate(scc_cc_alloc(self), bytes);
}

static void scc_cc_deallocate(scc_cc* self, void* block)
{
        deallocate(scc_cc_alloc(self), block);
}

extern void scc_cc_init(scc_cc* self, FILE* log, jmp_buf on_fatal_error)
{
        base_allocator_init(&self->alloc, (void*)&scc_cc_on_out_of_memory, on_fatal_error);
        allocator* alloc = scc_cc_alloc(self);
        scc_cc_opts* o = &self->opts;
        o->target = SCTK_32;
        o->mode = SCRM_DEFAULT;
        o->output = SCOK_C;
        o->print.flags = SCPF_NONE;
        o->print.double_precision = 4;
        o->print.float_precision = 4;
        o->optimization.fold_constants = false;
        o->optimization.eliminate_dead_code = false;
        self->out = NULL;
        self->log = log;
        dseq_init_ex_ptr(&self->sources, alloc);
        dseq_init_ex_ptr(&self->libs, alloc);
        flookup_init_ex(&self->source_lookup, alloc);
        flookup_init_ex(&self->lib_lookup, alloc);
        tree_init_target_info(&self->target, TTARGET_X32);
        tree_init_ex(&self->tree, &self->target, alloc);
        cinit_ex(&self->c, &self->tree, on_fatal_error, alloc);
        ssa_init_ex(&self->ssa, &self->tree, on_fatal_error, alloc);
}

static char* scc_cc_copy_string(scc_cc* self, const char* string)
{
        char* copy = scc_cc_allocate(self, strlen(string) + 1);
        strcpy(copy, string);
        return copy;
}

extern void scc_cc_dispose(scc_cc* self)
{
        ssa_dispose(&self->ssa);
        cdispose(&self->c);
        tree_dispose(&self->tree);
        flookup_dispose(&self->lib_lookup);
        flookup_dispose(&self->source_lookup);
        dseq_dispose(&self->libs);
        dseq_dispose(&self->sources);
        base_allocator_dispose(&self->alloc);
}

extern void scc_cc_error(scc_cc* self, const char* format, ...)
{
        fprintf(self->log, "scc: error: ");
        va_list args;
        va_start(args, format);
        vfprintf(self->log, format, args);
        fprintf(self->log, "\n");
}

extern void scc_cc_file_doesnt_exist(scc_cc* self, const char* file)
{
        scc_cc_error(self, "no such file or directory '%s'", file);
}

extern void scc_cc_unable_to_open(scc_cc* self, const char* file)
{
        scc_cc_error(self, "unable to open '%s'", file);
}

extern FILE* scc_cc_open_existing_file(scc_cc* self, const char* file, const char* mode)
{
        FILE* f = scc_cc_open_file(self, file, mode);
        if (!f)
        {
                scc_cc_file_doesnt_exist(self, file);
                return NULL;
        }
        return f;
}

extern FILE* scc_cc_open_file(scc_cc* self, const char* file, const char* mode)
{
        char path[S_MAX_PATH_LEN];
        if (S_FAILED(path_get_cd(path) || S_FAILED(path_join(path, file))))
                return NULL;

        return fopen(path, mode);
}

extern void scc_cc_set_mode(scc_cc* self, scc_cc_run_mode mode)
{
        self->opts.mode = mode;
}

extern void scc_cc_set_output(scc_cc* self, FILE* out)
{
        self->out = out;
}

extern void scc_cc_set_log(scc_cc* self, FILE* log)
{
        self->log = log;
}

extern serrcode scc_cc_add_source_file(scc_cc* self, const char* file)
{
        S_ASSERT(file);
        file_entry* f = file_get(&self->source_lookup, file);
        if (!f)
        {
                scc_cc_file_doesnt_exist(self, file);
                return S_ERROR;
        }
        dseq_append_ptr(&self->sources, f);
        return S_NO_ERROR;
}

extern serrcode scc_cc_emulate_source_file(
        scc_cc* self, const char* filename, const char* content)
{
        file_entry* f = file_emulate(&self->source_lookup, filename, content);
        if (!f)
                return S_ERROR;

        dseq_append_ptr(&self->sources, f);
        return S_NO_ERROR;
}

extern void scc_cc_add_source_dir(scc_cc* self, const char* dir)
{
        flookup_add(&self->source_lookup, dir);
}

extern serrcode scc_cc_run(scc_cc* self)
{
        tree_init_target_info(&self->target,
                self->opts.target == SCTK_32 ? TTARGET_X32 : TTARGET_X64);

        if (!dseq_size(&self->sources))
        {
                scc_cc_error(self, "no input files");
                return S_ERROR;
        }

        scc_cc_run_mode m = self->opts.mode;
        if (m == SCRM_DEFAULT)
                return scc_cc_link(self);
        else if (m == SCRM_LEX_ONLY)
                return scc_cc_lex(self);
        else if (m == SCRM_SYNTAX_ONLY)
                return scc_cc_parse(self);
        else if (m == SCRM_ASM_ONLY)
                return scc_cc_compile(self);

        return S_ERROR;
}
