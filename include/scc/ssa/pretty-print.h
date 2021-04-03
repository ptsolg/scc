#ifndef SSA_PRETTY_PRINT_H
#define SSA_PRETTY_PRINT_H

#include <stdio.h>

typedef struct _write_cb write_cb;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_module ssa_module;

extern void ssa_pretty_print_module(
        FILE* fout, ssa_context* context, const ssa_module* module);

extern void ssa_pretty_print_module_llvm(
        FILE* fout, ssa_context* context, const ssa_module* module);

#endif
