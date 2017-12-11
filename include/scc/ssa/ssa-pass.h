#ifndef SSA_PASS_H
#define SSA_PASS_H

#include "scc/scl/list.h"

typedef struct _ssa_pass ssa_pass;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_module ssa_module;

typedef struct _ssa_pass
{
        list_node node;
        void(*run)(ssa_context*, ssa_module*);
} ssa_pass;

extern void ssa_pass_init(ssa_pass* self, void(*run)(ssa_context*, ssa_module*));
extern void ssa_pass_run(ssa_pass* self, ssa_context* context, ssa_module* module);

typedef struct _ssa_pass_manager
{
        list_head passes;
} ssa_pass_manager;

extern void ssa_pass_manager_init(ssa_pass_manager* self);
extern void ssa_pass_manager_add_pass(ssa_pass_manager* self, ssa_pass* pass);

extern void ssa_pass_manager_run(
        ssa_pass_manager* self, ssa_context* context, ssa_module* module);

#endif