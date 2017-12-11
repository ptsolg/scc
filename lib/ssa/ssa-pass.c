#include "scc/ssa/ssa-pass.h"

extern void ssa_pass_init(ssa_pass* self, void(*run)(ssa_context*, ssa_module*))
{
        self->run = run;
        list_node_init(&self->node);
}

extern void ssa_pass_run(ssa_pass* self, ssa_context* context, ssa_module* module)
{
        self->run(context, module);
}

extern void ssa_pass_manager_init(ssa_pass_manager* self)
{
        list_init(&self->passes);
}

extern void ssa_pass_manager_add_pass(ssa_pass_manager* self, ssa_pass* pass)
{
        list_push_back(&self->passes, &pass->node);
}

extern void ssa_pass_manager_run(
        ssa_pass_manager* self, ssa_context* context, ssa_module* module)
{
        LIST_FOREACH(&self->passes, ssa_pass*, pass)
                ssa_pass_run(pass, context, module);
}