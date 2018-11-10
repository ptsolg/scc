#include "scc/c/sema-stmt.h"
#include "scc/c/sema-conv.h"
#include "scc/c/sema-decl.h"
#include "scc/c/sema-expr.h"
#include "misc.h"
#include "errors.h"
#include "scc/tree/eval.h"
#include "scc/tree/stmt.h"

extern void c_sema_add_compound_stmt_item(c_sema* self, tree_stmt* compound, tree_stmt* item)
{
        assert(item);
        tree_add_stmt_to_scope(tree_get_compound_scope(compound), item);
}

extern tree_stmt* c_sema_new_compound_stmt(c_sema* self, tree_location lbrace_loc)
{
        return tree_new_compound_stmt(self->context,
                tree_create_xloc(lbrace_loc, TREE_INVALID_LOC), self->scope, self->locals);
}

extern tree_stmt* c_sema_start_atomic_stmt(c_sema* self, tree_location kw_loc)
{
        self->tm_info.atomic_stmt_nesting++;
        return tree_new_atomic_stmt(self->context, tree_create_xloc(kw_loc, kw_loc), NULL);
}

extern tree_stmt* c_sema_finish_atomic_stmt(c_sema* self, tree_stmt* atomic, tree_stmt* body)
{
        self->tm_info.atomic_stmt_nesting--;
        tree_set_atomic_body(atomic, body);
        return atomic;
}

extern tree_stmt* c_sema_start_case_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_expr* expr)
{
        if (!c_sema_in_switch_stmt(self))
        {
                c_error_case_stmt_outside_switch(self->ccontext, kw_loc);
                return NULL;
        }

        if (c_sema_in_transaction_safe_block(self) && !c_sema_switch_stmt_in_atomic_block(self))
        {
                c_error_jumping_into_the_atomic_block_is_prohibited(self->ccontext, kw_loc, CTK_CASE);
                return NULL;
        }

        if (!c_sema_require_integer_expr(self, expr))
                return NULL;

        tree_eval_result r;
        if (!tree_eval_expr_as_integer(self->context, expr, &r))
        {
                c_error_case_stmt_isnt_constant(self->ccontext, kw_loc);
                return NULL;
        }
        
        int_value value = avalue_get_int(&r.value);
        tree_stmt* switch_ = c_sema_get_switch_stmt_info(self)->switch_stmt;
        expr = c_sema_new_impl_cast(self, expr,
                tree_get_expr_type(tree_get_switch_expr(switch_)));

        tree_stmt* case_stmt = tree_new_case_stmt(self->context,
                tree_create_xloc(kw_loc, colon_loc), expr, &value, NULL);
        if (!c_sema_switch_stmt_register_case_label(self, case_stmt))
        {
                c_error_case_stmt_duplication(self->ccontext, kw_loc);
                return NULL;
        }
        return case_stmt;
}

extern tree_stmt* c_sema_finish_case_stmt(c_sema* self, tree_stmt* stmt, tree_stmt* body)
{
        tree_set_case_body(stmt, body);
        return stmt;
}

extern tree_stmt* c_sema_start_default_stmt(
        c_sema* self, tree_location kw_loc, tree_location colon_loc)
{
        if (!c_sema_in_switch_stmt(self))
        {
                c_error_default_stmt_outside_switch(self->ccontext, kw_loc);
                return false;
        }
        if (c_sema_switch_stmt_has_default_label(self))
        {
                c_error_default_stmt_duplication(self->ccontext, kw_loc);
                return false;
        }
        if (c_sema_in_transaction_safe_block(self) && !c_sema_switch_stmt_in_atomic_block(self))
        {
                c_error_jumping_into_the_atomic_block_is_prohibited(self->ccontext, kw_loc, CTK_DEFAULT);
                return NULL;
        }

        tree_stmt* default_stmt = tree_new_default_stmt(self->context,
                tree_create_xloc(kw_loc, colon_loc), NULL);
        c_sema_set_switch_stmt_has_default_label(self);
        return default_stmt;
}

extern tree_stmt* c_sema_finish_default_stmt(c_sema* self, tree_stmt* stmt, tree_stmt* body)
{
        tree_set_default_body(stmt, body);
        return stmt;
}

extern tree_stmt* c_sema_new_labeled_stmt(c_sema* self, tree_decl* label, tree_stmt* stmt)
{
        if (c_sema_in_transaction_safe_block(self))
                strmap_insert(&self->tm_info.atomic_labels, tree_get_decl_name(label), label);

        tree_set_label_decl_stmt(label, stmt);
        return tree_new_labeled_stmt(self->context, tree_get_decl_loc(label), label);
}

extern tree_stmt* c_sema_new_expr_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_expr* expr)
{
        return tree_new_expr_stmt(self->context, tree_create_xloc(begin_loc, semicolon_loc), expr);
}

extern tree_stmt* c_sema_new_if_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_)
{
        if (!c_sema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_if_stmt(self->context, 
                tree_create_xloc(kw_loc, rbracket_loc), condition, body, else_);
}

extern tree_stmt* c_sema_new_decl_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_decl* d)
{
        return tree_new_decl_stmt(self->context,
                tree_create_xloc(begin_loc, semicolon_loc), d);
}

extern tree_stmt* c_sema_new_switch_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        tree_stmt* body)
{
        c_sema_integer_promotion(self, &value);
        if (!c_sema_require_integer_expr(self, value))
                return NULL;

        return tree_new_switch_stmt(self->context,
                tree_create_xloc(kw_loc, rbracket_loc), body, value);
}

extern tree_stmt* c_sema_start_switch_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        int scope_flags)
{
        tree_stmt* s = c_sema_new_switch_stmt(self, kw_loc, rbracket_loc, value, NULL);
        if (!s)
                return NULL;

        c_sema_push_switch_stmt_info(self, s);
        if (scope_flags & CSF_ATOMIC)
                c_sema_set_switch_stmt_in_atomic_block(self);

        return s;
}

extern tree_stmt* c_sema_finish_switch_stmt(c_sema* self, tree_stmt* switch_, tree_stmt* body)
{
        tree_set_switch_body(switch_, body);
        c_sema_pop_switch_stmt_info(self);
        return switch_;
}

extern tree_stmt* c_sema_new_while_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body)
{
        if (!c_sema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_while_stmt(self->context,
                tree_create_xloc(kw_loc, rbracket_loc), condition, body);
}

extern tree_stmt* c_sema_new_do_while_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_expr* condition,
        tree_stmt* body)
{
        if (!c_sema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_do_while_stmt(self->context,
                tree_create_xloc(kw_loc, semicolon_loc), condition, body);
}

static bool _c_sema_check_iteration_stmt_decl(const c_sema* self, const tree_decl* d)
{
        if (!tree_decl_is(d, TDK_VAR))
        {
                c_error_non_variable_decl_in_for_loop(self->ccontext, d);
                return false;
        }

        tree_storage_class sc = tree_get_decl_storage_class(d);
        if (sc != TSC_NONE && sc != TSC_AUTO && sc != TSC_REGISTER)
        {
                c_error_invalid_storage_class_for_loop_decl(self->ccontext, d);
                return false;
        }
        return true;
}

static bool c_sema_check_iteration_stmt_decl(const c_sema* self, const tree_decl* d)
{
        if (!tree_decl_is(d, TDK_GROUP))
                return _c_sema_check_iteration_stmt_decl(self, d);

        TREE_FOREACH_DECL_IN_GROUP(d, it)
                if (!_c_sema_check_iteration_stmt_decl(self, *it))
                        return false;
        return true;
}

extern tree_stmt* c_sema_new_for_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_expr* condition,
        tree_expr* step,
        tree_stmt* body)
{
        if (tree_stmt_is(init, TSK_DECL))
                if (!c_sema_check_iteration_stmt_decl(self, tree_get_decl_stmt_entity(init)))
                        return NULL;

        if (condition && !c_sema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_for_stmt(self->context,
                tree_create_xloc(kw_loc, rbracket_loc), init, condition, step, body);
}

extern tree_stmt* c_sema_new_goto_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location id_loc,
        tree_id id,
        tree_location semicolon_loc,
        int scope_flags)
{
        tree_decl* l = c_sema_declare_label_decl(self, id_loc, id);
        if (!l)
                return NULL;

        tree_stmt* goto_stmt = tree_new_goto_stmt(
                self->context, tree_create_xloc(kw_loc, semicolon_loc), l);
        if (!(scope_flags & CSF_ATOMIC))
                ptrvec_push(&self->tm_info.non_atomic_gotos, goto_stmt);

        return goto_stmt;
}

extern tree_stmt* c_sema_new_continue_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_continue_stmt(self->context, tree_create_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* c_sema_new_break_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_break_stmt(self->context, tree_create_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* c_sema_new_return_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc, tree_expr* value)
{
        tree_type* restype = tree_get_func_type_result(tree_get_decl_type(self->function));
        if (tree_type_is_void(restype) && value)
        {
                c_error_return_non_void(self->ccontext, value);
                return NULL;
        }

        c_assignment_conversion_result r;
        if (value && !c_sema_assignment_conversion(self, restype, &value, &r))
        {
                c_error_invalid_return_type(self->ccontext, value, &r);
                return NULL;
        }

        return tree_new_return_stmt(self->context, tree_create_xloc(kw_loc, semicolon_loc), value);
}

static bool c_sema_check_top_level_compound_stmt(const c_sema* self, const tree_stmt* s)
{
        TREE_FOREACH_DECL_IN_SCOPE(self->labels, it)
                if (!tree_get_label_decl_stmt(it))
                {
                        c_error_undefined_label(self->ccontext, it);
                        return false;
                }

        for (void** it = ptrvec_begin(&self->tm_info.non_atomic_gotos),
                **end = ptrvec_end(&self->tm_info.non_atomic_gotos); it != end; it++)
        {
                tree_decl* goto_dest = tree_get_goto_label(*it);
                if (!strmap_has(&self->tm_info.atomic_labels, tree_get_decl_name(goto_dest)))
                        continue;

                c_error_jumping_into_the_atomic_block_is_prohibited(
                        self->ccontext, tree_get_stmt_loc(*it).begin, CTK_GOTO);
                return false;
        }
        return true;
}

extern bool c_sema_check_stmt(const c_sema* self, const tree_stmt* s, int scope_flags)
{
        if (!s)
                return false;

        bool is_top_level = self->scope == NULL;
        tree_stmt_kind sk = tree_get_stmt_kind(s);

        if (sk == TSK_BREAK && !(scope_flags & CSF_BREAK))
        {
                c_error_break_stmt_outside_loop_or_switch(self->ccontext, s);
                return false;
        }
        else if (sk == TSK_CONTINUE && !(scope_flags & CSF_CONTINUE))
        {
                c_error_continue_stmt_outside_loop(self->ccontext, s);
                return false;
        }
        else if (sk == TSK_DECL && !(scope_flags & CSF_DECL))
        {
                c_error_decl_stmt_outside_block(self->ccontext, s);
                return false;
        }
        else if (sk == TSK_COMPOUND && is_top_level)
                return c_sema_check_top_level_compound_stmt(self, s);

        return true;
}