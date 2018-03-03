#include "scc/cc/llvm.h"
#include "scc/core/file.h"
#include "scc/core/args.h"
#include <stdio.h>
#include <string.h>
#include <scc/cc/llvm.h>

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

extern errcode llvm_compile(llvm_compiler* self, int* exit_code)
{
        arg_info args;
        arg_info_init(&args);
        arg_append(&args, self->file);

        char opt[8];
        snprintf(opt, ARRAY_SIZE(opt), "-O=%u", self->opt_level);
        arg_append(&args, opt);

        char filetype[64];
        snprintf(filetype, ARRAY_SIZE(filetype), "-filetype=%s",
                (self->output_kind == LCOK_OBJ ? "obj" : "asm"));
        arg_append(&args, filetype);

        char arch[64];
        snprintf(arch, ARRAY_SIZE(arch), "-march=%s",
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

extern errcode llvm_linker_add_dir(llvm_linker* self, const char* dir)
{
#if OS_WIN
        size_t len = strlen(dir) + sizeof("/LIBPATH:\"\"");
        char* copy = allocate(self->alloc, len + 1);
        if (!copy)
                return EC_ERROR;

        snprintf(copy, len, "/LIBPATH:\"%s\"", dir);
        if (EC_FAILED(dseq_append(&self->dirs, copy)))
        {
                deallocate(self->alloc, copy);
                return EC_ERROR;
        }
        return EC_NO_ERROR;
#elif OS_OSX
        char* copy = allocate(self->alloc, strlen(dir) + 1);
        if (!copy)
                return S_ERROR;
        if (S_FAILED(dseq_append(&self->dirs, copy)))
        {
                deallocate(self->alloc, copy);
                return S_ERROR;
        }
        strcpy(copy, dir);
        return S_NO_ERROR;
#else
#error
#endif
}

extern errcode llvm_linker_add_file(llvm_linker* self, const char* file)
{
        char* copy = allocate(self->alloc, strlen(file) + 1);
        if (!copy)
                return EC_ERROR;

        strcpy(copy, file);
        if (EC_FAILED(dseq_append(&self->files, copy)))
        {
                deallocate(self->alloc, copy);
                return EC_ERROR;
        }
        return EC_NO_ERROR;
}

extern void llvm_linker_init(llvm_linker* self, const char* path)
{
        self->path = path;
        self->output = NULL;
        self->entry = NULL;
        self->alloc = STDALLOC;
        dseq_init_alloc(&self->files, self->alloc);
        dseq_init_alloc(&self->dirs, self->alloc);
}

extern void llvm_linker_dispose(llvm_linker* self)
{
        for (void** it = dseq_begin(&self->files),
                **end = dseq_end(&self->files); it != end; it++)
        {
                deallocate(self->alloc, *it);
        }

        for (void** it = dseq_begin(&self->dirs),
                **end = dseq_end(&self->dirs); it != end; it++)
        {
                deallocate(self->alloc, *it);
        }

        dseq_dispose(&self->files);
        dseq_dispose(&self->dirs);
}

extern errcode llvm_link(llvm_linker* self, int* exit_code)
{
        arg_info args;
        arg_info_init(&args);

        for (void** it = dseq_begin(&self->files),
                **end = dseq_end(&self->files); it != end; it++)
        {
                arg_append(&args, *it);
        }

        for (void** it = dseq_begin(&self->dirs),
                **end = dseq_end(&self->dirs); it != end; it++)
        {
                arg_append(&args, *it);
        }

#if OS_WIN
        char output[MAX_PATH_LEN + sizeof("/OUT:\"\"")];
        if (self->output)
        {
                snprintf(output, ARRAY_SIZE(output), "/OUT:\"%s\"", self->output);
                arg_append(&args, output);
        }

        char entry[128];
        if (self->entry)
        {
                snprintf(entry, ARRAY_SIZE(entry), "/ENTRY:%s", self->entry);
                arg_append(&args, entry);
        }
#elif OS_OSX
        if (self->output)
        {
                arg_append(&args, "-o");
                arg_append(&args, self->output);
        }
        if (self->entry)
        {
                arg_append(&args, "-e");
                arg_append(&args, self->entry);
        }
        arg_append(&args, "-lSystem");
        arg_append(&args, "-macosx_version_min");
        arg_append(&args, "10.12");
#else
#error
#endif

        //printf("lld >> %s\n", self->path);
        //for (int i = 0; i < args.argc; i++)
        //        printf("lld >> %s\n", args.argv[i]);

        return execute(self->path, exit_code, args.argc, args.argv);
}