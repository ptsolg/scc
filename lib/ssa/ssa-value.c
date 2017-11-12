#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-context.h"

extern ssa_value* ssa_new_value_base(
        ssa_context* context, ssa_value_kind kind, tree_type* type, ssize size)
{
        ssa_value* v = ssa_allocate(context, size);
        if (!v)
                return NULL;

        ssa_set_value_kind(v, kind);
        ssa_set_value_type(v, type);
        return v;
}

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* type, avalue val)
{
        ssa_value* v = ssa_new_value_base(context,
                SVK_CONSTANT, type, sizeof(struct _ssa_constant));
        if (!v)
                return NULL;

        ssa_set_constant_value(v, val);
        return v;
}

extern ssa_value* ssa_new_var(
        ssa_context* context, tree_type* type, ssa_id id, ssa_instr* init)
{
        ssa_value* v = ssa_new_value_base(context,
                SVK_VARIABLE, type, sizeof(struct _ssa_var));
        if (!v)
                return NULL;

        ssa_set_var_id(v, id);
        ssa_set_var_init(v, init);
        return v;
}