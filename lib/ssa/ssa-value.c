#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-context.h"

extern void ssa_init_value_base(ssa_value* self, ssa_value_kind k, ssa_id id)
{
        ssa_set_value_kind(self, k);
        ssa_set_value_id(self, id);
}

extern void ssa_init_label(ssa_value* self, ssa_id id)
{
        ssa_init_value_base(self, SVK_LABEL, id);
}

extern void ssa_init_typed_value(ssa_value* self, ssa_value_kind k, ssa_id id, tree_type* t)
{
        ssa_init_value_base(self, k, id);
        ssa_set_value_type(self, t);
}

extern void ssa_init_variable(ssa_value* self, ssa_id id, tree_type* t)
{
        ssa_init_typed_value(self, SVK_VARIABLE, id, t);
}

extern void ssa_init_constant(ssa_value* self, tree_type* t, avalue val)
{
        ssa_init_typed_value(self, SVK_CONSTANT, 0, t);
        ssa_set_constant_value(self, val);
}

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* t, avalue val)
{
        ssa_value* c = ssa_allocate(context, sizeof(ssa_constant));
        if (!c)
                return NULL;

        ssa_init_constant(c, t, val);
        return c;
}
