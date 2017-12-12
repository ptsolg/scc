#include "scc/cc/llvm.h"
#include <stdio.h>
#include <string.h>

extern void llvm_compiler_init(llvm_compiler* self, const char* path)
{
        scc_tool_init(&self->tool, "llc", path);
        self->opt_level = LCOL_O0;
        self->file = NULL;
}

extern void llvm_compiler_set_file(llvm_compiler* self, const char* file)
{
        self->file = file;
}

extern void llvm_compiler_set_opt_level(llvm_compiler* self, llvm_compiler_opt_level level)
{
        self->opt_level = level;
}

extern serrcode llvm_compiler_run(llvm_compiler* self, int* code)
{
        char opt[8];
        snprintf(opt, S_ARRAY_SIZE(opt), "-O=%u", self->opt_level);

        const char* argv[] = 
        {
                self->file,
                opt,
                "-filetype=obj",
                NULL
        };

        return scc_tool_exec(&self->tool, code, argv);
}

static char* llvm_get_tool_path(llvm_tools* self, const char* tool_name)
{
        char path[S_MAX_PATH_LEN + 1];
        strncpy(path, self->dir, S_MAX_PATH_LEN);
        if (S_FAILED(path_join(path, tool_name)))
                return NULL;

        char* result = allocate(STDALLOC, strlen(path) + 1);
        if (!result)
                return NULL;

        strcpy(result, path);
        return result;
}

extern serrcode llvm_tools_init(llvm_tools* self, const char* dir)
{
        self->llc_path = NULL;
        self->dir = dir;

#if S_WIN
        const char* llc = "llc.exe";
#elif S_OSX
        const char* llc = "llc";
#else
#error todo
#endif

        if (!(self->llc_path = llvm_get_tool_path(self, llc)))
                goto error;

        llvm_compiler_init(&self->llc, self->llc_path);
        return S_NO_ERROR;

error:
        llvm_tools_dispose(self);
        return S_ERROR;
}

extern void llvm_tools_dispose(llvm_tools* self)
{
        deallocate(STDALLOC, self->llc_path);
}