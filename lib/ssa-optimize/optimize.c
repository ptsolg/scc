#include "scc/ssa-optimize/optimize.h"
#include "scc/ssa/module.h"

extern void ssa_reset_optimizer_opts(ssa_optimizer_opts* self)
{
        self->fold_constants = false;
        self->eliminate_dead_code = false;
        self->promote_allocas = false;
}

extern void ssa_optimize(ssa_context* context,
        ssa_module* module, const ssa_optimizer_opts* opts)
{
        ssa_pass cf;
        ssa_init_pass(&cf, SPK_FUNCTION, &ssa_fold_constants);
        ssa_pass dce;
        ssa_init_pass(&dce, SPK_FUNCTION, &ssa_eliminate_dead_code);
        ssa_pass pa;
        ssa_init_pass(&pa, SPK_FUNCTION, &ssa_promote_allocas);

        ssa_pass_manager pm;
        ssa_init_pass_manager(&pm);

        if (opts->fold_constants)
                ssa_pass_manager_add_pass(&pm, &cf);
        if (opts->eliminate_dead_code || opts->promote_allocas)
                ssa_pass_manager_add_pass(&pm, &dce);
        if (opts->promote_allocas)
                ssa_pass_manager_add_pass(&pm, &pa);

        ssa_pass_manager_run(&pm, context, module);
        ssa_number_module_values(module);
}