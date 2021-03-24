#include "scc/ssa/pass.h"
#include "scc/ssa/module.h"
#include "scc/ssa/instr.h"

extern void ssa_init_pass(ssa_pass* self, ssa_pass_kind kind, void(*run)(const ssa_pass*))
{
        init_list(&self->node);
        self->kind = kind;
        self->module = NULL;
        self->function = NULL;
        self->entry = run;
        self->context = NULL;
}

extern void ssa_init_pass_manager(ssa_pass_manager* self)
{
        init_list(&self->passes);
}

extern void ssa_pass_manager_add_pass(ssa_pass_manager* self, ssa_pass* pass)
{
        list_push(&self->passes, &pass->node);
}

extern void ssa_pass_manager_run(ssa_pass_manager* self, ssa_context* context, ssa_module* module)
{
        LIST_FOREACH(&self->passes, ssa_pass*, pass)
        {
                if (!pass->entry)
                        continue;

                pass->context = context;
                pass->module = module;
                if (pass->kind == SPK_MODULE)
                        pass->entry(pass);
                else if (pass->kind == SPK_FUNCTION)
                        SSA_FOREACH_MODULE_GLOBAL(module, it, end)
                                if (ssa_get_value_kind(*it) == SVK_FUNCTION)
                                {
                                        pass->function = *it;
                                        pass->entry(pass);
                                }
        }
}
