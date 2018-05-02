#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-block.h"

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

extern void ssa_init_local_var(ssa_value* self, ssa_id id, tree_type* type)
{
        ssa_init_value(self, SVK_LOCAL_VAR, id, type);
}

extern ssa_value* ssa_new_global_var(ssa_context* context, tree_decl* var)
{
        assert(var);
        tree_type* t = tree_desugar_type(tree_get_decl_type(var));
        t = tree_new_pointer_type(context->tree, t);
        ssa_value* v = ssa_new_value(context, SVK_GLOBAL_VAR, 0, t, sizeof(struct _ssa_global_var));
        if (!v)
                return NULL;

        _ssa_global_var(v)->_entity = var;
        return v;
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

extern ssa_value* ssa_new_function(ssa_context* context, tree_decl* func)
{
        assert(func);
        tree_type* t = tree_desugar_type(tree_get_decl_type(func));
        if (tree_type_is(t, TTK_FUNCTION))
                t = tree_new_pointer_type(context->tree, t);

        ssa_value* val = ssa_new_value(context, SVK_FUNCTION, 0, t, sizeof(struct _ssa_function));
        if (!val)
                return NULL;

        struct _ssa_function* f = _ssa_function(val);
        f->_entity = func;
        list_init(&f->_blocks);
        ssa_init_array(&f->_params);

        return val;
}

extern void ssa_add_function_block(ssa_value* self, ssa_block* block)
{
        assert(block);
        list_push_back(&_ssa_function(self)->_blocks, &block->_node);
}

extern void ssa_add_function_param(ssa_value* self, ssa_context* context, ssa_value* param)
{
        assert(param);
        ssa_value** obj = ssa_reserve_object(context, &_ssa_function(self)->_params, sizeof(ssa_value*));
        *obj = param;
}

extern bool ssa_function_returns_void(const ssa_value* self)
{
        return tree_type_is_void(ssa_get_function_result_type(self));
}

extern void ssa_fix_function_content_uids(ssa_value* self)
{
        ssa_id uid = 0;
        SSA_FOREACH_FUNCTION_PARAM(self, it)
                ssa_set_value_id(*it, uid++);

        SSA_FOREACH_FUNCTION_BLOCK(self, block)
        {
                ssa_set_value_id(ssa_get_block_label(block), uid++);
                SSA_FOREACH_BLOCK_INSTR(block, instr)
                        if (ssa_instr_has_var(instr))
                                ssa_set_value_id(ssa_get_instr_var(instr), uid++);
        }
}