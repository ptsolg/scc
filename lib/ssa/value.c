#include "scc/ssa/value.h"
#include "scc/core/list.h"
#include "scc/ssa/context.h"
#include "scc/ssa/instr.h"
#include "scc/ssa/block.h"

extern void ssa_init_value(ssa_value* self, ssa_value_kind kind, tree_type* type)
{
        ssa_set_value_kind(self, kind);
        _ssa_value_base(self)->id = 0;
        ssa_set_value_type(self, type);
        ssa_set_value_metadata(self, NULL);
        init_list(&_ssa_value_base(self)->use_list);
}

extern ssa_value* ssa_new_value(ssa_context* context,
        ssa_value_kind kind, tree_type* type, size_t size)
{
        ssa_value* v = ssa_allocate_node(context, size);
        if (!v)
                return NULL;

        ssa_init_value(v, kind, type);
        return v;
}

extern void ssa_replace_value_with(ssa_value* what, ssa_value* with)
{
        SSA_FOREACH_VALUE_USE(what, it, end)
                it->value = with;

        struct list* uses = &_ssa_value_base(what)->use_list;
        list_append(&_ssa_value_base(with)->use_list, uses);
}

extern void _ssa_remove_value_use(ssa_value_use* self)
{
        self->value = NULL;
        list_remove(&self->node);
}

extern void _ssa_add_value_use(ssa_value* val, ssa_value_use* use)
{
        assert(use->value == NULL);
        use->value = val;
        list_push(&_ssa_value_base(val)->use_list, &use->node);
}

extern ssa_block* ssa_get_value_use_block(const ssa_value_use* self)
{
        return ssa_get_instr_block(ssa_get_value_use_instr(self));
}

extern ssa_value* ssa_get_value_use_label(const ssa_value_use* self)
{
        return ssa_get_block_label(ssa_get_value_use_block(self));
}

extern void ssa_set_value_use_value(ssa_value_use* self, ssa_value* val)
{
        _ssa_remove_value_use(self);
        _ssa_add_value_use(val, self);
}

extern ssa_value* ssa_new_undef(ssa_context* context, tree_type* type)
{
        return ssa_new_value(context, SVK_UNDEF, type, sizeof(struct _ssa_undef));
}

extern void ssa_init_local_var(ssa_value* self, tree_type* type)
{
        ssa_init_value(self, SVK_LOCAL_VAR, type);
}

extern ssa_value* ssa_new_global_var(
        ssa_context* context, tree_decl* var, ssa_const* init)
{
        assert(var);
        tree_type* t = tree_desugar_type(tree_get_decl_type(var));
        t = tree_new_pointer_type(context->tree, t);
        ssa_value* v = ssa_new_value(context, SVK_GLOBAL_VAR, t, sizeof(struct _ssa_global_var));
        if (!v)
                return NULL;

        _ssa_global_var(v)->entity = var;
        ssa_set_global_var_init(v, init);
        return v;
}

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* type, const struct num* val)
{
        ssa_value* c = ssa_new_value(context,
                SVK_CONSTANT, type, sizeof(struct _ssa_constant));
        if (!c)
                return NULL;

        ssa_set_constant_value(c, val);
        return c;
}

extern void ssa_init_label(ssa_value* self, tree_type* type)
{
        ssa_init_value(self, SVK_LABEL, type);
}

extern ssa_value* ssa_new_string(
        ssa_context* context, tree_type* type, tree_id ref)
{
        ssa_value* s = ssa_new_value(context, SVK_STRING, type, sizeof(struct _ssa_string));
        if (!s)
                return NULL;

        ssa_set_string_value(s, ref);
        return s;
}

extern ssa_value* ssa_new_param(ssa_context* context, tree_type* type)
{
        return ssa_new_value(context, SVK_PARAM, type, sizeof(struct _ssa_param));
}

extern ssa_value* ssa_new_function(
        ssa_context* context, tree_decl* func, ssa_intrin_kind intrin_kind)
{
        assert(func);
        tree_type* t = tree_desugar_type(tree_get_decl_type(func));
        if (tree_type_is(t, TTK_FUNCTION))
                t = tree_new_pointer_type(context->tree, t);

        ssa_value* val = ssa_new_value(context, SVK_FUNCTION, t, sizeof(struct _ssa_function));
        if (!val)
                return NULL;

        struct _ssa_function* f = _ssa_function(val);
        f->entity = func;
        f->intrin_kind = intrin_kind;
        init_list(&f->blocks);
        ssa_init_array(&f->params);

        return val;
}

extern void ssa_add_function_block(ssa_value* self, ssa_block* block)
{
        assert(block && ssa_get_function_intrin_kind(self) == SSA_INTRIN_NONE);
        list_push(&_ssa_function(self)->blocks, &block->node);
}

extern void ssa_add_function_param(ssa_value* self, ssa_context* context, ssa_value* param)
{
        assert(param);
        ssa_value** obj = ssa_reserve_object(context, &_ssa_function(self)->params, sizeof(ssa_value*));
        *obj = param;
}

extern bool ssa_function_returns_void(const ssa_value* self)
{
        return tree_type_is_void(ssa_get_function_result_type(self));
}

extern void ssa_number_function_values(ssa_value* self)
{
        ssa_id id = 0;
        tree_type* ft = tree_get_pointer_target(ssa_get_value_type(self));

        if (tree_get_semantic_func_type_params_size(ft))
                SSA_FOREACH_FUNCTION_PARAM(self, it)
                        _ssa_value_base(*it)->id = id++;

        SSA_FOREACH_FUNCTION_BLOCK(self, block)
        {
                _ssa_value_base(ssa_get_block_label(block))->id = id++;
                SSA_FOREACH_BLOCK_INSTR(block, instr)
                        if (ssa_instr_has_var(instr))
                                _ssa_value_base(ssa_get_instr_var(instr))->id = id++;
        }
}
