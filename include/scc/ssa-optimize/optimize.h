#ifndef SSA_OPT_H
#define SSA_OPT_H

#include "scc/ssa/pass.h"
#include <stdbool.h>

extern void ssa_fold_constants(const ssa_pass* pass);
extern void ssa_eliminate_dead_code(const ssa_pass* pass);
extern void ssa_promote_allocas(const ssa_pass* pass);
extern void ssa_inline_functions(const ssa_pass* pass);

typedef struct _ssa_optimizer_opts
{
        bool fold_constants;
        bool eliminate_dead_code;
        bool promote_allocas;
        bool inline_functions;
} ssa_optimizer_opts;

extern void ssa_reset_optimizer_opts(ssa_optimizer_opts* self);

extern void ssa_optimize(ssa_context* context, 
        ssa_module* module, const ssa_optimizer_opts* opts);

#endif
