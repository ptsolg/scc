#ifndef SSA_PASS_H
#define SSA_PASS_H

#include "scc/core/list.h"

typedef struct _ssa_pass ssa_pass;
typedef struct _ssa_module ssa_module;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_value ssa_value;

typedef enum _ssa_pass_kind
{
        SPK_FUNCTION,
        SPK_MODULE,
        SPK_SIZE,
} ssa_pass_kind;

typedef struct _ssa_pass
{
        list_node node;
        ssa_pass_kind kind;
        ssa_module* module;
        ssa_value* function;
        ssa_context* context;
        void(*entry)(const ssa_pass*);
} ssa_pass;

extern void ssa_init_pass(ssa_pass* self, ssa_pass_kind kind, void(*entry)(const ssa_pass*));

typedef struct _ssa_pass_manager
{
        list_head passes;
} ssa_pass_manager;

extern void ssa_init_pass_manager(ssa_pass_manager* self);
extern void ssa_pass_manager_add_pass(ssa_pass_manager* self, ssa_pass* pass);
extern void ssa_pass_manager_run(ssa_pass_manager* self, ssa_context* context, ssa_module* module);

#endif