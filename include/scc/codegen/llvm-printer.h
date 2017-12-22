#ifndef CG_LLVM_PRINTER_H
#define CG_LLVM_PRINTER_H

#include "scc/scl/read-write.h"
#include "scc/ssa/ssa-common.h"

typedef struct _ssa_module ssa_module;
typedef struct _tree_module tree_module;
typedef struct _tree_decl tree_decl;
typedef struct _ssa_function ssa_function;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_instr ssa_instr;
typedef struct _tree_type tree_type;
typedef struct _ssa_context ssa_context;
typedef struct _tree_context tree_context;

typedef struct _llvm_printer
{
        writebuf buf;
        const ssa_context* ssa;
        const tree_context* tree;
        ssa_id tmp_uid;
} llvm_printer;

extern void llvm_printer_init(llvm_printer* self, write_cb* write, const ssa_context* ssa);
extern void llvm_printer_dispose(llvm_printer* self);

extern void llvm_printer_emit_type(llvm_printer* self, const tree_type* type);
extern void llvm_printer_emit_value(
        llvm_printer* self, const ssa_value* value, bool emit_type);
extern void llvm_printer_emit_instr(llvm_printer* self, const ssa_instr* instr);
extern void llvm_printer_emit_block(llvm_printer* self, const ssa_block* block);
extern void llvm_printer_emit_decl(llvm_printer* self, const tree_decl* decl);
extern void llvm_printer_emit_function_decl(llvm_printer* self, const ssa_function* func);

extern void llvm_printer_emit_module(
        llvm_printer* self, const ssa_module* sm, const tree_module* tm);

#endif