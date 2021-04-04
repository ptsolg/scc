#ifndef CC_LLVM_H
#define CC_LLVM_H

#include "scc/core/common.h"
#include "scc/core/vec.h"

typedef enum
{
        LCOL_O0,
        LCOL_O1,
        LCOL_O2,
        LCOL_O3,
} llvm_compiler_opt_level;

typedef enum
{
        LCOK_OBJ,
        LCOK_ASM,
} llvm_compiler_output_kind;

typedef enum
{
        LCAK_X86,
        LCAK_X86_64,
} llvm_compiler_arch_kind;

typedef struct
{
        const char* path;
        llvm_compiler_opt_level opt_level;
        llvm_compiler_output_kind output_kind;
        llvm_compiler_arch_kind arch;
        const char* file;
        const char* output;
} llvm_compiler;

#define LLC_NAME "llc.exe"

extern void llvm_compiler_init(llvm_compiler* self, const char* path);
extern errcode llvm_compile(llvm_compiler* self);

typedef struct
{
        const char* path;
        const char* output;
        const char* entry;
        struct vec files;
        struct vec dirs;
} llvm_linker;

#define LLD_NAME "lld-link.exe"

extern void llvm_linker_init(llvm_linker* self, const char* path);
extern void llvm_linker_dispose(llvm_linker* self);
extern void llvm_linker_add_dir(llvm_linker* self, const char* dir);
extern void llvm_linker_add_file(llvm_linker* self, const char* file);
extern int llvm_link(llvm_linker* self);

#endif
