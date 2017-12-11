#ifndef SSA_OPT_H
#define SSA_OPT_H

#include "ssa-instr.h"
#include "ssa-pass.h"

typedef struct _ssa_function ssa_function;

extern void ssa_opt_constant_fold(ssa_context* context, ssa_module* module);
extern void ssa_opt_init_constant_fold_pass(ssa_pass* pass);

extern void ssa_opt_eliminate_dead_code(ssa_context* context, ssa_module* module);
extern void ssa_opt_init_eliminate_dead_code_pass(ssa_pass* pass);

typedef enum
{
        SOK_CONSTANT_FOLDING,
        SOK_DEAD_CODE_ELIMINATION,
        SOK_SIZE,
} ssa_opt_kind;

typedef struct _ssa_optimizer
{
        ssa_pass passes[SOK_SIZE];
        bool enabled[SOK_SIZE];
} ssa_optimizer;

extern void ssa_init_optimizer(ssa_optimizer* self);
extern void ssa_optimizer_enable_pass(ssa_optimizer* self, ssa_opt_kind kind);
extern void ssa_optimizer_run(ssa_optimizer* self, ssa_context* context, ssa_module* module);

#endif