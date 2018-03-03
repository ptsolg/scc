#ifndef SSA_PRINTER_H
#define SSA_PRINTER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/read-write.h"

typedef struct _ssa_module ssa_module;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_instr ssa_instr;
typedef struct _tree_context tree_context;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_function ssa_function;

typedef struct _ssa_printer
{
        writebuf buf;
        const ssa_context* context;
} ssa_printer;

extern void ssa_init_printer(ssa_printer* self, write_cb* write, const ssa_context* context);

extern void ssa_dispose_printer(ssa_printer* self);

extern void ssa_print_instr(ssa_printer* self, const ssa_instr* instr);
extern void ssa_print_value(ssa_printer* self, const ssa_value* value);
extern void ssa_print_block(ssa_printer* self, const ssa_block* block);
extern void ssa_print_function(ssa_printer* self, const ssa_function* func);
extern void ssa_print_module(ssa_printer* self, const ssa_module* module);

#ifdef __cplusplus
}
#endif

#endif