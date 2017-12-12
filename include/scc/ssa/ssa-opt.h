#ifndef SSA_OPT_H
#define SSA_OPT_H

#include "ssa-instr.h"
#include "ssa-pass.h"

extern void ssa_fold_constants(ssa_context* context, ssa_function* function);
extern void ssa_eliminate_dead_code(ssa_context* context, ssa_function* function);

typedef struct
{
        ssa_pass pass;
        ssa_context* context;
} ssa_constant_fold_pass;

extern void ssa_init_constant_fold_pass(ssa_constant_fold_pass* self, ssa_context* context);

typedef struct
{
        ssa_pass pass;
        ssa_context* context;
} ssa_dead_code_elimination_pass;

extern void ssa_init_dead_code_elimination_pass(
        ssa_dead_code_elimination_pass* self, ssa_context* context);

typedef struct _ssa_optimizer_opts
{
        bool fold_constants;
        bool eliminate_dead_code;
} ssa_optimizer_opts;

extern void ssa_reset_optimizer_opts(ssa_optimizer_opts* self);

extern void ssa_optimize(ssa_context* context, 
        ssa_module* module, const ssa_optimizer_opts* opts);

#endif