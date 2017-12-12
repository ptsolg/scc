#include "scc/ssa/ssa-pass.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-function.h"
#include "scc/ssa/ssa-instr.h"

extern void ssa_init_pass(ssa_pass* self, ssa_pass_kind kind, void* run_fn)
{
        self->kind = kind;
        self->run_fn = run_fn;
        list_node_init(&self->node);
}

extern void ssa_run_pass_on_function(ssa_pass* self, ssa_function* function)
{
        self->run_on_function(self, function);
}

extern void ssa_run_pass_on_module(ssa_pass* self, ssa_module* module)
{
        self->run_on_module(self, module);
}

extern void ssa_init_pass_manager(ssa_pass_manager* self)
{
        list_init(&self->passes);
}

extern void ssa_pass_manager_add_pass(ssa_pass_manager* self, ssa_pass* pass)
{
        list_push_back(&self->passes, &pass->node);
}

extern void ssa_pass_manager_run(ssa_pass_manager* self, ssa_module* module)
{
        LIST_FOREACH(&self->passes, ssa_pass*, pass)
        {
                if (pass->kind == SPK_MODULE)
                        ssa_run_pass_on_module(pass, module);
                else if (pass->kind == SPK_FUNCTION)
                        SSA_FOREACH_MODULE_DEF(module, function)
                                ssa_run_pass_on_function(pass, function);
        }
}