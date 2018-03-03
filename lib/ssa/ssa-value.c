#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-instr.h"

extern void ssa_init_value(
        ssa_value* self,
        ssa_value_kind kind,
        ssa_id id,
        tree_type* type)
{
        ssa_set_value_kind(self, kind);
        ssa_set_value_id(self, id);
        ssa_set_value_type(self, type);
        list_init(&_ssa_value_base(self)->_use_list);
}

extern ssa_value* ssa_new_value(ssa_context* context,
        ssa_value_kind kind, ssa_id id, tree_type* type, size_t size)
{
        ssa_value* v = ssa_allocate(context, size);
        if (!v)
                return NULL;

        ssa_init_value(v, kind, id, type);
        return v;
}

extern void ssa_replace_value_with(ssa_value* what, ssa_value* with)
{
        SSA_FOREACH_VALUE_USE(what, it, end)
                it->_value = with;

        list_head* uses = &_ssa_value_base(what)->_use_list;
        list_push_back_list(&_ssa_value_base(with)->_use_list, uses);
        list_init(uses);
}

extern void ssa_init_variable(ssa_value* self, ssa_id id, tree_type* type)
{
        ssa_init_value(self, SVK_VARIABLE, id, type);
}

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* type, const avalue* val)
{
        ssa_value* c = ssa_new_value(context,
                SVK_CONSTANT, 0, type, sizeof(struct _ssa_constant));
        if (!c)
                return NULL;

        ssa_set_constant_value(c, val);
        return c;
}

extern void ssa_init_label(ssa_value* self, ssa_id id, tree_type* type)
{
        ssa_init_value(self, SVK_LABEL, id, type);
}

extern ssa_value* ssa_new_decl(
        ssa_context* context, tree_type* type, tree_decl* decl)
{
        ssa_value* d = ssa_new_value(context, SVK_DECL, 0, type, sizeof(struct _ssa_decl));
        if (!d)
                return NULL;

        ssa_set_decl_entity(d, decl);
        return d;
}

extern ssa_value* ssa_new_string(
        ssa_context* context, ssa_id id, tree_type* type, tree_id ref)
{
        ssa_value* s = ssa_new_value(context, SVK_STRING, id, type, sizeof(struct _ssa_string));
        if (!s)
                return NULL;

        ssa_set_string_value(s, ref);
        return s;
}

extern ssa_value* ssa_new_param(ssa_context* context, ssa_id id, tree_type* type)
{
        return ssa_new_value(context, SVK_PARAM, id, type, sizeof(struct _ssa_param));
}
