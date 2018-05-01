#ifndef SSA_PASS_H
#define SSA_PASS_H

#include "scc/core/list.h"

typedef struct _ssa_pass ssa_pass;
typedef struct _ssa_module ssa_module;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_block ssa_block;
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
        union
        {
                void(*run_on_function)(ssa_pass*, ssa_value*);
                void(*run_on_module)(ssa_pass*, ssa_module*);
                void* run_fn;
        };
} ssa_pass;

extern void ssa_init_pass(ssa_pass* self, ssa_pass_kind kind, void* run_fn);

extern void ssa_run_pass_on_function(ssa_pass* self, ssa_value* function);
extern void ssa_run_pass_on_module(ssa_pass* self, ssa_module* module);

typedef struct _ssa_pass_manager
{
        list_head passes;
} ssa_pass_manager;

extern void ssa_init_pass_manager(ssa_pass_manager* self);
extern void ssa_pass_manager_add_pass(ssa_pass_manager* self, ssa_pass* pass);

extern void ssa_pass_manager_run(ssa_pass_manager* self, ssa_module* module);

#endif