#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/tree/tree-decl.h"

extern bool ssaize_var_decl(ssaizer* self, const tree_decl* decl)
{
        ssa_value* alloca = ssa_build_alloca(&self->builder, tree_get_decl_type(decl));
        if (!alloca)
                return false;

        ssaizer_set_def(self, decl, alloca);
        tree_expr* init = tree_get_var_init(decl);
        ssa_value* init_value = NULL;
        if (init && !(init_value = ssaize_expr(self, init)))
                return false;

        if (init && !ssa_build_store(&self->builder, init_value, alloca))
                return false;

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
        {
                return true;
        }

        tree_type* restype = ssa_get_function_result_type(self->function);
        ssa_value* val = NULL;
        if (!tree_type_is_void(restype))
        {
                ssa_value* alloca = ssa_build_alloca(&self->builder, restype);
                val = ssa_build_load(&self->builder, alloca);
        }

        ssa_build_return(&self->builder, val);
        ssaizer_finish_current_block(self);
        return true;
}

static void ssaizer_function_cleanup(ssaizer* self)
{
      //  htab_clear(&self->labels);
        ssaizer_pop_scope(self); // params
}

static bool ssaize_function_args(ssaizer* self, ssa_function* func, tree_decl* decl)
{
        ssaizer_push_scope(self);

        tree_decl_scope* params = tree_get_func_params(decl);
        TREE_FOREACH_DECL_IN_SCOPE(params, it)
        {
                tree_type* param_type = tree_get_decl_type(it);
                ssa_value* param_value = ssa_build_function_param(&self->builder, param_type);
                if (!param_value)
                        return false;

                ssa_add_function_param(func, self->context, param_value);
                ssa_value* param = ssa_build_alloca(&self->builder, param_type);
                if (!param || !ssa_build_store(&self->builder, param_value, param))
                        return false;

                ssaizer_set_def(self, it, param);
        }
        return true;
}

extern bool ssaize_function_decl(ssaizer* self, tree_decl* func)
{
        tree_stmt* body = tree_get_func_body(func);
        if (!body)
                return true;

        self->function = ssa_new_function(self->context, func);
        ssaizer_enter_block(self, ssaizer_new_block(self));
        ssa_builder_set_uid(&self->builder, 0);

        if (!ssaize_function_args(self, self->function, func) 
                || !ssaize_compound_stmt(self, body)
                || !ssaizer_maybe_insert_return(self))
        {
                ssaizer_function_cleanup(self);
                return false;
        }

        ssaizer_function_cleanup(self);
        ssa_fix_function_content_uids(self->function);
        ssa_module_add_func_def(self->module, self->function);
        return true;
}

extern bool ssaize_decl(ssaizer* self, tree_decl* decl)
{
        tree_decl_kind k = tree_get_decl_kind(decl);
        if (k == TDK_VAR)
                return ssaize_var_decl(self, decl);
        else if (k == TDK_GROUP)
                return ssaize_decl_group(self, decl);
        else if (k == TDK_FUNCTION)
                return ssaize_function_decl(self, decl);

        // just ignore unknown decl
        return true;
}