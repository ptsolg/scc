#ifndef CC_LLVM_H
#define CC_LLVM_H

#include "tool.h"
#include "scc/scl/file.h"

typedef enum
{
        LCOL_O0,
        LCOL_O1,
        LCOL_O2,
        LCOL_O3,
} llvm_compiler_opt_level;

typedef struct
{
        scc_tool tool;
        const char* file;
        llvm_compiler_opt_level opt_level;
} llvm_compiler;

extern void llvm_compiler_init(llvm_compiler* self, const char* path);
extern void llvm_compiler_set_file(llvm_compiler* self, const char* file);
extern void llvm_compiler_set_opt_level(llvm_compiler* self, llvm_compiler_opt_level level);
extern serrcode llvm_compiler_run(llvm_compiler* self, int* code);

typedef struct
{
        const char* dir;
        char* llc_path;

        llvm_compiler llc;
} llvm_tools;

extern serrcode llvm_tools_init(llvm_tools* self, const char* dir);
extern void llvm_tools_dispose(llvm_tools* self);

#endif