#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-block.h"
#include "scc/tree/tree-decl.h"

static void ssaize_record_decl(ssaizer* self, tree_decl* decl)
{
        assert(tree_decl_is(decl, TDK_RECORD));
        if (ssaizer_record_is_emitted(self, decl))
                return;

        ssaizer_set_record_emitted(self, decl);

        tree_decl_scope* fields = tree_get_record_fields(decl);
        TREE_FOREACH_DECL_IN_SCOPE(fields, it)
        {
                if (!tree_decl_is(it, TDK_FIELD))
                        continue;

                ssaize_type(self, tree_get_decl_type(it));
        }

        ssa_add_module_type_decl(self->module, decl);
}

extern void ssaize_type(ssaizer* self, tree_type* type)
{
        if (!type)
                return;

        while (1)
        {
                type = tree_desugar_type(type);
                tree_type_kind k = tree_get_type_kind(type);
                if (k == TTK_POINTER || k == TTK_ARRAY)
                        type = tree_get_chain_type_next(type);
                else if (k == TTK_FUNCTION)
                {
                        TREE_FOREACH_FUNC_TYPE_PARAM(type, it)
                                ssaize_type(self, *it);
                        type = tree_get_func_type_result(type);
                }
                else if (k == TTK_DECL)
                {
                        ssaize_record_decl(self, tree_get_decl_type_entity(type));
                        return;
                }
                else
                        return;
        }
}

static ssa_value* ssaize_alloca(ssaizer* self, tree_type* t)
{
        ssa_value* v = ssa_build_alloca_after(&self->builder, t, self->alloca_insertion_pos);
        if (v)
                self->alloca_insertion_pos = ssa_get_var_instr(v);
        return v;
}

extern bool ssaize_var_decl(ssaizer* self, tree_decl* decl)
{
        if (tree_decl_is_anon(decl))
                return true;

        tree_type* t = tree_get_decl_type(decl);
        ssaize_type(self, t);

        ssa_value* val;
        tree_storage_duration sd = tree_get_decl_storage_duration(decl);
        if (sd == TSD_STATIC || sd == TSD_THREAD)
        {
                if (!(val = ssa_new_global_var(self->context, decl)))
                        return false;

                // todo: initializer
                ssa_add_module_global(self->module, val);
        }
        else if (sd == TSD_AUTOMATIC)
        {
                if (!(val = ssaize_alloca(self, t)))
                        return false;

                tree_expr* init = tree_get_var_init(decl);
                ssa_value* init_value = NULL;

                if (init && !(init_value = ssaize_expr(self, init)))
                        return false;
                if (init && !ssa_build_store(&self->builder, init_value, val))
                        return false;
        }
        else
                UNREACHABLE();

        if (tree_decl_is_global(decl))
                ssaizer_set_global_decl(self, decl, val);
        else
                ssaizer_set_def(self, decl, val);

        return true;
}

extern bool ssaize_decl_group(ssaizer* self, const tree_decl* decl)
{
        TREE_FOREACH_DECL_IN_GROUP(decl, it)
                if (!ssaize_decl(self, *it))
                        return false;
        return true;
}

static bool ssaizer_maybe_insert_return(ssaizer* self)
{
        if (!self->block || ssaizer_current_block_is_terminated(self))
                return true;

        tree_type* restype = ssa_get_function_result_type(self->function);
        ssa_value* val = NULL;
        if (!tree_type_is_void(restype))
        {
                ssa_value* alloca = ssaize_alloca(self, restype);
                val = ssa_build_load(&self->builder, alloca);
        }

        ssa_build_return(&self->builder, val);
        ssaizer_finish_current_block(self);
        return true;
}

static void ssaizer_function_cleanup(ssaizer* self)
{
        strmap_clear(&self->labels);
        ssaizer_pop_scope(self); // params
}

static bool ssaize_function_decl_body(ssaizer* self, ssa_value* func, tree_stmt* body)
{
        self->function = func;

        ssaizer_enter_block(self, ssaizer_new_block(self));
        ssa_builder_set_uid(&self->builder, 0);
        self->alloca_insertion_pos = ssa_get_block_instrs_end(self->block);

        ssaizer_push_scope(self); // params

        tree_decl_scope* params = tree_get_func_params(ssa_get_function_entity(func));
        TREE_FOREACH_DECL_IN_SCOPE(params, it)
                if (!ssaize_decl(self, it))
                        goto error;
        if (!ssaize_compound_stmt(self, body) || !ssaizer_maybe_insert_return(self))
                goto error;

        ssa_fix_function_content_uids(self->function);
        ssaizer_function_cleanup(self);
        return true;

error:
        ssaizer_function_cleanup(self);
        return false;
}

extern bool ssaize_function_decl(ssaizer* self, tree_decl* func)
{
        if (tree_get_func_builtin_kind(func) != TFBK_ORDINARY)
                return true;

        ssaize_type(self, tree_get_decl_type(func));

        ssa_value* ssa_func = ssa_new_function(self->context, func);
        tree_stmt* body = tree_get_func_body(func);
        if (body && !ssaize_function_decl_body(self, ssa_func, body))
                return false;

        ssa_add_module_global(self->module, ssa_func);
        ssaizer_set_global_decl(self, func, ssa_func);
        return true;
}

extern bool ssaize_param_decl(ssaizer* self, tree_decl* param)
{
        tree_type* param_type = tree_get_decl_type(param);
        ssa_value* param_value = ssa_build_function_param(&self->builder, param_type);
        if (!param_value)
                return false;

        ssa_add_function_param(self->function, self->context, param_value);
        ssa_value* loaded_param = ssaize_alloca(self, param_type);
        if (!loaded_param || !ssa_build_store(&self->builder, param_value, loaded_param))
                return false;

        ssaizer_set_def(self, param, loaded_param);
        return true;
}

extern bool ssaize_decl(ssaizer* self, tree_decl* decl)
{
        switch (tree_get_decl_kind(decl))
        {
                case TDK_VAR: return ssaize_var_decl(self, decl);
                case TDK_GROUP: return ssaize_decl_group(self, decl);
                case TDK_FUNCTION: return ssaize_function_decl(self, decl);
                case TDK_PARAM: return ssaize_param_decl(self, decl);

                case TDK_RECORD:
                        ssaize_record_decl(self, decl);
                        return true;

                case TDK_TYPEDEF:
                        ssaize_type(self, tree_get_decl_type(decl));
                        return true;

                default:
                        // just ignore unknown decl
                        return true;
        }
}