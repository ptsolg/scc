#include "scc/cc/llvm.h"
#include "scc/core/file.h"
#include "scc/core/cmd.h"
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

        // for (int i = 0; i < args.argc; i++)
        //        printf("llc >> %s\n", args.argv[i]);

        return execute(self->path, exit_code, args.argc, args.argv);
}

extern void llvm_linker_add_dir(llvm_linker* self, const char* dir)
{
        size_t len = strlen(dir) + sizeof("/LIBPATH:\"\"");
        char* copy = alloc(len + 1);
        snprintf(copy, len, "/LIBPATH:\"%s\"", dir);
        vec_push(&self->dirs, copy);
}

extern void llvm_linker_add_file(llvm_linker* self, const char* file)
{
        char* copy = alloc(strlen(file) + 1);
        strcpy(copy, file);
        vec_push(&self->files, copy);
}

extern void llvm_linker_init(llvm_linker* self, const char* path)
{
        self->path = path;
        self->output = NULL;
        self->entry = NULL;
        vec_init(&self->files);
        vec_init(&self->dirs);
}

extern void llvm_linker_dispose(llvm_linker* self)
{
        for (void** it = vec_begin(&self->files),
                **end = vec_end(&self->files); it != end; it++)
        {
                dealloc(*it);
        }

        for (void** it = vec_begin(&self->dirs),
                **end = vec_end(&self->dirs); it != end; it++)
        {
                dealloc(*it);
        }

        vec_drop(&self->files);
        vec_drop(&self->dirs);
}

extern errcode llvm_link(llvm_linker* self, int* exit_code)
{
        arg_info args;
        arg_info_init(&args);

        for (void** it = vec_begin(&self->files),
                **end = vec_end(&self->files); it != end; it++)
        {
                arg_append(&args, *it);
        }

        for (void** it = vec_begin(&self->dirs),
                **end = vec_end(&self->dirs); it != end; it++)
        {
                arg_append(&args, *it);
        }

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

        // printf("lld >> %s\n", self->path);
        // for (int i = 0; i < args.argc; i++)
        //        printf("lld >> %s\n", args.argv[i]);

        return execute(self->path, exit_code, args.argc, args.argv);
}
