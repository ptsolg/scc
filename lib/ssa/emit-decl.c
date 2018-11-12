#include "emitter.h"
#include "scc/ssa/module.h"
#include "scc/ssa/context.h"

static void ssa_emit_record_decl(ssa_module_emitter* self, tree_decl* decl)
{
        assert(tree_decl_is(decl, TDK_RECORD));

        if (ssa_record_is_emmited(self, decl))
                return;

        ssa_set_record_emmited(self, decl);

        tree_decl_scope* fields = tree_get_record_fields(decl);
        TREE_FOREACH_DECL_IN_SCOPE(fields, it)
        {
                if (!tree_decl_is(it, TDK_FIELD))
                        continue;

                ssa_emit_type(self, tree_get_decl_type(it));
        }

        ssa_add_module_type_decl(self->module, decl);
}

extern void ssa_emit_type(ssa_module_emitter* self, const tree_type* type)
{
        if (!type)
                return;

        while (1)
        {
                type = tree_desugar_type_c(type);
                tree_type_kind k = tree_get_type_kind(type);
                if (k == TTK_POINTER || k == TTK_ARRAY)
                        type = tree_get_chain_type_next(type);
                else if (k == TTK_FUNCTION)
                {
                        TREE_FOREACH_FUNC_TYPE_PARAM(type, it)
                                ssa_emit_type(self, *it);
                        type = tree_get_func_type_result(type);
                }
                else if (k == TTK_DECL)
                {
                        ssa_emit_record_decl(self, tree_get_decl_type_entity(type));
                        return;
                }
                else
                        return;
        }
}

static bool ssa_emit_return_opt(ssa_function_emitter* self)
{
        if (!self->block || ssa_current_block_is_terminated(self))
                return true;

        tree_type* restype = ssa_get_function_result_type(self->function);
        ssa_value* val = NULL;
        if (!tree_type_is_void(restype))
        {
                ssa_value* alloca = ssa_emit_alloca(self, restype);
                val = ssa_emit_load(self, alloca);
        }

        ssa_build_return(&self->builder, val);
        ssa_emit_current_block(self);
        return true;
}

static bool ssa_emit_function_decl_body(
        ssa_module_emitter* self, ssa_value* func, const tree_stmt* body)
{
        bool result = false;
        ssa_function_emitter fe;
        ssa_init_function_emitter(&fe, self, func);

        ssa_push_scope(&fe); // params

        tree_decl_scope* params = tree_get_func_params(ssa_get_function_entity(func));
        TREE_FOREACH_DECL_IN_SCOPE(params, it)
                if (!ssa_emit_local_decl(&fe, it))
                        goto cleanup;

        tree_type* func_type = tree_get_pointer_target(ssa_get_value_type(func));
        if (tree_func_type_is_transaction_safe(func_type))
        {
                ssa_value* isactive = ssa_build_call_0(
                        &fe.builder, fe.module_emitter->tm_info.active);
                if (!isactive || !(isactive = ssa_build_neq_zero(&fe.builder, isactive)))
                        goto cleanup;

                ssa_block* onactive = ssa_new_function_block(&fe);
                ssa_block* otherwise = ssa_new_function_block(&fe);
                if (!ssa_emit_cond_jmp(&fe, isactive, onactive, otherwise))
                        goto cleanup;

                ssa_enter_block(&fe, onactive);
                if (!ssa_emit_stmt_as_atomic(&fe, body) || !ssa_emit_return_opt(&fe))
                        goto cleanup;

                ssa_enter_block(&fe, otherwise);
        }

        if (!ssa_emit_stmt(&fe, body) || !ssa_emit_return_opt(&fe))
                goto cleanup;

        result = true;
cleanup:
        ssa_pop_scope(&fe);
        ssa_dispose_function_emitter(&fe);
        return result;
}

static bool ssa_emit_function_decl(ssa_module_emitter* self, tree_decl* func)
{
        if (tree_get_func_builtin_kind(func) != TFBK_ORDINARY)
                return true;

        ssa_value* val = ssa_get_global_decl(self, func);
        if (!val)
        {
                val = ssa_new_function(self->context, func);
                ssa_add_module_global(self->module, val);
                ssa_set_global_decl(self, func, val);
        }
        else if (ssa_function_has_body(val))
                return true;

        ssa_set_function_entity(val, func);
        tree_stmt* body = tree_get_func_body(func);
        return body ? ssa_emit_function_decl_body(self, val, body) : true;
}

static bool ssa_emit_global_var_decl(ssa_module_emitter* self, tree_decl* var)
{
        if (tree_decl_is_anon(var))
                return true;

        ssa_emit_type(self, tree_get_decl_type(var));

        ssa_value* val = ssa_get_global_decl(self, var);
        if (!val)
        {
                val = ssa_new_global_var(self->context, var);
                ssa_add_module_global(self->module, val);
                ssa_set_global_decl(self, var, val);
        }
        else
        {
                tree_decl* e = ssa_get_global_var_entity(val);
                if (tree_get_var_init(e) || tree_get_decl_storage_class(e) == TSC_IMPL_EXTERN)
                        return true;
        }

        // todo: initializer

        ssa_set_global_var_entity(val, var);
        return true;
}

extern bool ssa_emit_global_decl(ssa_module_emitter* self, tree_decl* decl)
{
        switch (tree_get_decl_kind(decl))
        {
                case TDK_FUNCTION:
                        return ssa_emit_function_decl(self, decl);

                case TDK_VAR:
                        return ssa_emit_global_var_decl(self, decl);

                case TDK_TYPEDEF:
                        ssa_emit_type(self, tree_get_decl_type(decl));
                        return true;

                case TDK_GROUP:
                        TREE_FOREACH_DECL_IN_GROUP(decl, it)
                                if (!ssa_emit_global_decl(self, *it))
                                        return false;
                        return true;

                case TDK_RECORD:
                        ssa_emit_record_decl(self, decl);
                        return true;

                default:
                        // just skip other decls
                        return true;
        }
}

static bool ssa_emit_local_var_decl(ssa_function_emitter* self, tree_decl* var)
{
        ssa_emit_type(self->module_emitter, tree_get_decl_type(var));

        ssa_value* val;
        tree_storage_duration sd = tree_get_decl_storage_duration(var);
        if (sd == TSD_STATIC || sd == TSD_THREAD)
        {
                if (!(val = ssa_new_global_var(self->context, var)))
                        return false;

                ssa_set_def(self, var, val);
                return true;
        }

        if (!(val = ssa_emit_alloca(self, tree_get_decl_type(var))))
                return false;

        tree_expr* init = tree_get_var_init(var);
        ssa_value* init_value = NULL;

        if (init && !(init_value = ssa_emit_expr(self, init)))
                return false;
        if (init && !ssa_emit_store(self, init_value, val))
                return false;

        ssa_set_def(self, var, val);
        return true;
}

extern bool ssa_emit_param_decl(ssa_function_emitter* self, tree_decl* param)
{
        tree_type* param_type = tree_get_decl_type(param);
        ssa_value* param_value = ssa_build_function_param(&self->builder, param_type);
        if (!param_value)
                return false;

        ssa_add_function_param(self->function, self->context, param_value);
        ssa_value* loaded_param = ssa_emit_alloca(self, param_type);
        if (!loaded_param || !ssa_emit_store(self, param_value, loaded_param))
                return false;

        ssa_set_def(self, param, loaded_param);
        return true;
}

extern bool ssa_emit_local_decl(ssa_function_emitter* self, tree_decl* decl)
{
        switch (tree_get_decl_kind(decl))
        {
                case TDK_FUNCTION:
                        return ssa_emit_function_decl(self->module_emitter, decl);

                case TDK_VAR:
                        return ssa_emit_local_var_decl(self, decl);

                case TDK_TYPEDEF:
                        ssa_emit_type(self->module_emitter, tree_get_decl_type(decl));
                        return true;

                case TDK_GROUP:
                        TREE_FOREACH_DECL_IN_GROUP(decl, it)
                                if (!ssa_emit_local_decl(self, *it))
                                        return false;
                        return true;

                case TDK_RECORD:
                        ssa_emit_record_decl(self->module_emitter, decl);
                        return true;

                case TDK_PARAM:
                        return ssa_emit_param_decl(self, decl);

                default:
                        // just skip other decls
                        return true;
        }
}
