#include "scc/c/c-sema-stmt.h"
#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-expr.h"
#include "c-misc.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-eval.h"
#include "scc/tree/tree-stmt.h"

extern tree_stmt* c_sema_add_stmt(c_sema* self, tree_stmt* s)
{
        assert(s);
        tree_add_stmt_to_scope(self->scope, s);
        return s;
}

extern tree_stmt* c_sema_new_block_stmt(
        c_sema* self, tree_location lbrace_loc, int scope_flags)
{
        return tree_new_compound_stmt_ex(self->context,
                tree_create_xloc(lbrace_loc, 0), self->scope, self->locals, scope_flags);
}

extern tree_stmt* c_sema_new_case_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_expr* expr)
{
        if (!c_sema_in_switch_stmt(self))
        {
                c_error_case_stmt_outside_switch(self->logger, kw_loc);
                return false;
        }

        if (!c_sema_require_integer_expr(self, expr))
                return NULL;

        tree_eval_result r;
        if (!tree_eval_expr_as_integer(self->context, expr, &r))
        {
                c_error_case_stmt_isnt_constant(self->logger, kw_loc);
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
                c_error_case_stmt_duplication(self->logger, kw_loc);
                return NULL;
        }
        return case_stmt;
}

extern void c_sema_set_case_stmt_body(c_sema* self, tree_stmt* stmt, tree_stmt* body)
{
        tree_set_case_body(stmt, body);
}

extern tree_stmt* c_sema_new_default_stmt(
        c_sema* self, tree_location kw_loc, tree_location colon_loc)
{
        if (!c_sema_in_switch_stmt(self))
        {
                c_error_default_stmt_outside_switch(self->logger, kw_loc);
                return false;
        }
        if (c_sema_switch_stmt_has_default_label(self))
        {
                c_error_default_stmt_duplication(self->logger, kw_loc);
                return false;
        }

        tree_stmt* default_stmt = tree_new_default_stmt(self->context,
                tree_create_xloc(kw_loc, colon_loc), NULL);
        c_sema_set_switch_stmt_has_default_label(self);
        return default_stmt;
}

extern void c_sema_set_default_stmt_body(c_sema* self, tree_stmt* stmt, tree_stmt* body)
{
        tree_set_default_body(stmt, body);
}

extern tree_stmt* c_sema_new_labeled_stmt(c_sema* self, tree_decl* label, tree_stmt* stmt)
{
        tree_set_label_decl_stmt(label, stmt);
        return tree_new_labeled_stmt(self->context, tree_get_decl_loc(label), label);
}

extern tree_stmt* c_sema_new_expr_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_expr* expr)
{
        return tree_new_expr_stmt(self->context,
                tree_create_xloc(begin_loc, semicolon_loc), expr);
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
        tree_expr* value)
{
        tree_stmt* s = c_sema_new_switch_stmt(self, kw_loc, rbracket_loc, value, NULL);
        if (!s)
                return NULL;

        c_sema_push_switch_stmt_info(self, s);
        return s;
}

extern tree_stmt* c_sema_finish_switch_stmt(c_sema* self, tree_stmt* switch_, tree_stmt* body)
{
        if (!body)
                return NULL;

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
                c_error_non_variable_decl_in_for_loop(self->logger, d);
                return false;
        }

        tree_decl_storage_class sc = tree_get_decl_storage_class(d);
        if (sc != TDSC_NONE && sc != TDSC_AUTO && sc != TDSC_REGISTER)
        {
                c_error_invalid_storage_class_for_loop_decl(self->logger, d);
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
        tree_location semicolon_loc)
{
        tree_decl* l = c_sema_declare_label_decl(self, id_loc, id);
        if (!l)
                return NULL;

        return tree_new_goto_stmt(self->context, tree_create_xloc(kw_loc, semicolon_loc), l);
}

extern tree_stmt* c_sema_new_continue_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_continue_stmt(self->context,
                tree_create_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* c_sema_new_break_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_break_stmt(self->context,
                tree_create_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* c_sema_new_return_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc, tree_expr* value)
{
        tree_type* restype = tree_get_function_type_result(tree_get_decl_type(self->function));
        if (tree_type_is_void(restype) && value)
        {
                c_error_return_non_void(self->logger, value);
                return NULL;
        }

        c_assignment_conversion_result r;
        if (!value || c_sema_assignment_conversion(self, restype, &value, &r))
                return tree_new_return_stmt(self->context,
                        tree_create_xloc(kw_loc, semicolon_loc), value);

        tree_type* vt = tree_get_expr_type(value);
        tree_location vloc = tree_get_expr_loc(value);

        if (r.kind == CACRK_INCOMPATIBLE)
                c_error_return_type_doesnt_match(self->logger, value);
        else if (r.kind == CACRK_RHS_NOT_AN_ARITHMETIC)
                c_sema_require_arithmetic_expr_type(self, vt, vloc);
        else if (r.kind == CACRK_RHS_NOT_A_RECORD)
                c_sema_require_record_expr_type(self, vt, vloc);
        else if (r.kind == CACRK_INCOMPATIBLE_RECORDS)
                c_sema_require_compatible_expr_types(self, restype, vt, vloc);
        else if (r.kind == CACRK_QUAL_DISCARTION)
                c_error_return_discards_quals(self->logger, vloc, r.discarded_quals);
        else if (r.kind == CACRK_INCOMPATIBLE_POINTERS)
                c_error_return_from_incompatible_pointer_type(self->logger, vloc);
        return NULL;
}

extern bool c_sema_check_stmt(const c_sema* self, const tree_stmt* s, int scope_flags)
{
        if (!s)
                return false;

        bool is_top_level = self->scope == NULL;
        tree_stmt_kind sk = tree_get_stmt_kind(s);

        if (sk == TSK_BREAK && !(scope_flags & TSF_BREAK))
        {
                c_error_break_stmt_outside_loop_or_switch(self->logger, s);
                return false;
        }
        else if (sk == TSK_CONTINUE && !(scope_flags & TSF_CONTINUE))
        {
                c_error_continue_stmt_outside_loop(self->logger, s);
                return false;
        }
        else if (sk == TSK_DECL && !(scope_flags & TSF_DECL))
        {
                c_error_decl_stmt_outside_block(self->logger, s);
                return false;
        }
        else if (sk == TSK_COMPOUND && is_top_level)
        {
                TREE_FOREACH_DECL_IN_SCOPE(self->labels, it)
                        if (!tree_get_label_decl_stmt(it))
                        {
                                c_error_undefined_label(self->logger, it);
                                return false;
                        }
        }
        return true;
}