#include "scc/cc/llvm.h"
#include "scc/scl/file.h"
#include "scc/scl/args.h"
#include <stdio.h>
#include <string.h>

#define MAX_ARGC 1024

typedef struct
{
        int argc;
        const char* argv[MAX_ARGC];
} arg_info;

static void arg_info_init(arg_info* self)
{
        *self->argv = NULL;
        self->argc = 0;
}

static void arg_append(arg_info* self, const char* arg)
{
        if (self->argc + 1 >= MAX_ARGC)
                return;

        self->argv[self->argc++] = arg;
        self->argv[self->argc] = NULL;
}

extern void llvm_compiler_init(llvm_compiler* self, const char* path)
{
        self->path = path;
        self->opt_level = LCOL_O0;
        self->output_kind = LCOK_ASM;
        self->arch = LCAK_X86;
        self->file = NULL;
        self->output = NULL;
}

extern serrcode llvm_compile(llvm_compiler* self, int* exit_code)
{
        arg_info args;
        arg_info_init(&args);
        arg_append(&args, self->file);

        char opt[8];
        snprintf(opt, S_ARRAY_SIZE(opt), "-O=%u", self->opt_level);
        arg_append(&args, opt);

        char filetype[64];
        snprintf(filetype, S_ARRAY_SIZE(filetype), "-filetype=%s",
                (self->output_kind == LCOK_OBJ ? "obj" : "asm"));
        arg_append(&args, filetype);

        char arch[64];
        snprintf(arch, S_ARRAY_SIZE(arch), "-march=%s",
                (self->arch == LCAK_X86 ? "x86" : "x86-64"));
        arg_append(&args, arch);

        if (self->output)
        {
                arg_append(&args, "-o");
                arg_append(&args, self->output);
        }

        //for (int i = 0; i < args.argc; i++)
        //        printf("llc >> %s\n", args.argv[i]);

        return execute(self->path, exit_code, args.argc, args.argv);
}

extern serrcode llvm_linker_add_dir(llvm_linker* self, const char* dir)
{
        ssize len = strlen(dir) + sizeof("/LIBPATH:\"\"");
        char* copy = allocate(self->alloc, len + 1);
        if (!copy)
                return S_ERROR;

        snprintf(copy, len, "/LIBPATH:\"%s\"", dir);
        if (S_FAILED(dseq_append_ptr(&self->dirs, copy)))
        {
                deallocate(self->alloc, copy);
                return S_ERROR;
        }
        return S_NO_ERROR;
}

extern serrcode llvm_linker_add_file(llvm_linker* self, const char* file)
{
        char* copy = allocate(self->alloc, strlen(file) + 1);
        if (!copy)
                return S_ERROR;

        strcpy(copy, file);
        if (S_FAILED(dseq_append_ptr(&self->files, copy)))
        {
                deallocate(self->alloc, copy);
                return S_ERROR;
        }
        return S_NO_ERROR;
}

extern void llvm_linker_init(llvm_linker* self, const char* path)
{
        self->path = path;
        self->output = NULL;
        self->entry = NULL;
        self->alloc = STDALLOC;
        dseq_init_ex_ptr(&self->files, self->alloc);
        dseq_init_ex_ptr(&self->dirs, self->alloc);
}

extern void llvm_linker_dispose(llvm_linker* self)
{
        for (void** it = dseq_begin_ptr(&self->files),
                **end = dseq_end_ptr(&self->files); it != end; it++)
        {
                deallocate(self->alloc, *it);
        }

        for (void** it = dseq_begin_ptr(&self->dirs),
                **end = dseq_end_ptr(&self->dirs); it != end; it++)
        {
                deallocate(self->alloc, *it);
        }

        dseq_dispose(&self->files);
        dseq_dispose(&self->dirs);
}

extern serrcode llvm_link(llvm_linker* self, int* exit_code)
{
        arg_info args;
        arg_info_init(&args);

        for (void** it = dseq_begin_ptr(&self->files),
                **end = dseq_end_ptr(&self->files); it != end; it++)
        {
                arg_append(&args, *it);
        }

        for (void** it = dseq_begin_ptr(&self->dirs),
                **end = dseq_end_ptr(&self->dirs); it != end; it++)
        {
                arg_append(&args, *it);
        }

        char output[S_MAX_PATH_LEN + sizeof("/OUT:\"\"")];
        if (self->output)
        {
                snprintf(output, S_ARRAY_SIZE(output), "/OUT:\"%s\"", self->output);
                arg_append(&args, output);
        }

        char entry[128];
        if (self->entry)
        {
                snprintf(entry, S_ARRAY_SIZE(entry), "/ENTRY:%s", self->entry);
                arg_append(&args, entry);
        }

        //printf("lld >> %s\n", self->path);
        //for (int i = 0; i < args.argc; i++)
        //        printf("lld >> %s\n", args.argv[i]);

        return execute(self->path, exit_code, args.argc, args.argv);
}