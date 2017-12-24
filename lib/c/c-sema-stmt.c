#include "scc/c/c-sema-stmt.h"
#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-expr.h"
#include "scc/c/c-info.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-eval.h"

extern tree_stmt* csema_add_stmt(csema* self, tree_stmt* s)
{
        S_ASSERT(s);
        tree_scope_add(self->scope, s);
        return s;
}

extern tree_stmt* csema_new_block_stmt(
        csema* self, tree_location lbrace_loc, cstmt_context context)
{
        tree_scope_flags flags = cstmt_context_to_scope_flags(context);
        return tree_new_compound_stmt_ex(self->context,
                tree_init_xloc(lbrace_loc, 0), self->scope, self->locals, flags);
}

extern tree_stmt* csema_new_case_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_expr* expr)
{
        if (!csema_in_switch_stmt(self))
        {
                cerror_case_stmt_outside_switch(self->logger, kw_loc);
                return false;
        }

        if (!csema_require_integer_expr(self, expr))
                return NULL;

        int_value value;
        tree_eval_info info;
        tree_init_eval_info(&info, self->target);
        if (!tree_eval_as_integer(&info, expr, &value))
        {
                cerror_case_stmt_isnt_constant(self->logger, kw_loc);
                return NULL;
        }

        tree_stmt* switch_ = csema_get_switch_stmt_info(self)->switch_stmt;
        expr = csema_new_impl_cast(self, expr,
                tree_get_expr_type(tree_get_switch_expr(switch_)));

        tree_stmt* case_stmt = tree_new_case_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), expr, &value, NULL);
        if (!csema_switch_stmt_register_case_label(self, case_stmt))
        {
                cerror_case_stmt_duplication(self->logger, kw_loc);
                return NULL;
        }
        return case_stmt;
}

extern void csema_set_case_stmt_body(csema* self, tree_stmt* stmt, tree_stmt* body)
{
        tree_set_case_body(stmt, body);
}

extern tree_stmt* csema_new_default_stmt(
        csema* self, tree_location kw_loc, tree_location colon_loc)
{
        if (!csema_in_switch_stmt(self))
        {
                cerror_default_stmt_outside_switch(self->logger, kw_loc);
                return false;
        }
        if (csema_switch_stmt_has_default(self))
        {
                cerror_default_stmt_duplication(self->logger, kw_loc);
                return false;
        }

        tree_stmt* default_stmt = tree_new_default_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), NULL);
        csema_set_switch_stmt_has_default(self);
        return default_stmt;
}

extern void csema_set_default_stmt_body(csema* self, tree_stmt* stmt, tree_stmt* body)
{
        tree_set_default_body(stmt, body);
}

extern tree_stmt* csema_new_labeled_stmt(csema* self, tree_decl* label, tree_stmt* stmt)
{
        tree_set_label_decl_stmt(label, stmt);
        return tree_new_labeled_stmt(self->context, tree_get_decl_loc(label), label);
}

extern tree_stmt* csema_new_expr_stmt(
        csema* self, tree_location begin_loc, tree_location semicolon_loc, tree_expr* expr)
{
        return tree_new_expr_stmt(self->context,
                tree_init_xloc(begin_loc, semicolon_loc), expr);
}

extern tree_stmt* csema_new_if_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_)
{
        if (!csema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_if_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), condition, body, else_);
}

extern tree_stmt* csema_new_decl_stmt(
        csema* self, tree_location begin_loc, tree_location semicolon_loc, tree_decl* d)
{
        return tree_new_decl_stmt(self->context,
                tree_init_xloc(begin_loc, semicolon_loc), d);
}

extern tree_stmt* csema_new_switch_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        tree_stmt* body)
{
        csema_integer_promotion(self, &value);
        if (!csema_require_integer_expr(self, value))
                return NULL;

        return tree_new_switch_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), body, value);
}

extern tree_stmt* csema_start_switch_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value)
{
        tree_stmt* s = csema_new_switch_stmt(self, kw_loc, rbracket_loc, value, NULL);
        if (!s)
                return NULL;

        csema_push_switch_stmt_info(self, s);
        return s;
}

extern tree_stmt* csema_finish_switch_stmt(csema* self, tree_stmt* switch_, tree_stmt* body)
{
        if (!body)
                return NULL;

        tree_set_switch_body(switch_, body);
        csema_pop_switch_stmt_info(self);
        return switch_;
}

extern tree_stmt* csema_new_while_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body)
{
        if (!csema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_while_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), condition, body);
}

extern tree_stmt* csema_new_do_while_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_expr* condition,
        tree_stmt* body)
{
        if (!csema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_do_while_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc), condition, body);
}

static bool _csema_check_iteration_stmt_decl(const csema* self, const tree_decl* d)
{
        if (!tree_decl_is(d, TDK_VAR))
        {
                cerror_non_variable_decl_in_for_loop(self->logger, d);
                return false;
        }

        tree_decl_storage_class sc = tree_get_decl_storage_class(d);
        if (sc != TDSC_NONE && sc != TDSC_AUTO && sc != TDSC_REGISTER)
        {
                cerror_invalid_storage_class_for_loop_decl(self->logger, d);
                return false;
        }
        return true;
}

static bool csema_check_iteration_stmt_decl(const csema* self, const tree_decl* d)
{
        if (!tree_decl_is(d, TDK_GROUP))
                return _csema_check_iteration_stmt_decl(self, d);

        TREE_FOREACH_DECL_IN_GROUP(d, it)
                if (!_csema_check_iteration_stmt_decl(self, *it))
                        return false;
        return true;
}

extern tree_stmt* csema_new_for_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_expr* condition,
        tree_expr* step,
        tree_stmt* body)
{
        if (tree_stmt_is(init, TSK_DECL))
                if (!csema_check_iteration_stmt_decl(self, tree_get_decl_stmt_entity(init)))
                        return NULL;

        if (condition && !csema_require_scalar_expr(self, condition))
                return NULL;

        return tree_new_for_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), init, condition, step, body);
}

extern tree_stmt* csema_new_goto_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location id_loc,
        tree_id id,
        tree_location semicolon_loc)
{
        tree_decl* l = csema_declare_label_decl(self, id, id_loc);
        if (!l)
                return NULL;

        return tree_new_goto_stmt(self->context, tree_init_xloc(kw_loc, semicolon_loc), l);
}

extern tree_stmt* csema_new_continue_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_continue_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* csema_new_break_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_break_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* csema_new_return_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc, tree_expr* value)
{
        tree_type* restype = tree_get_function_type_result(tree_get_decl_type(self->function));
        if (tree_type_is_void(restype) && value)
        {
                cerror_return_non_void(self->logger, value);
                return NULL;
        }

        cassign_conv_result r;
        if (!value || csema_assignment_conversion(self, restype, &value, &r))
                return tree_new_return_stmt(self->context,
                        tree_init_xloc(kw_loc, semicolon_loc), value);

        tree_type* vt = tree_get_expr_type(value);
        tree_location vloc = tree_get_expr_loc(value);

        if (r.kind == CACRK_INCOMPATIBLE)
                cerror_return_type_doesnt_match(self->logger, value);
        else if (r.kind == CACRK_RHS_NOT_AN_ARITHMETIC)
                csema_require_arithmetic_expr_type(self, vt, vloc);
        else if (r.kind == CACRK_RHS_NOT_A_RECORD)
                csema_require_record_expr_type(self, vt, vloc);
        else if (r.kind == CACRK_INCOMPATIBLE_RECORDS)
                csema_require_compatible_expr_types(self, restype, vt, vloc);
        else if (r.kind == CACRK_QUAL_DISCARTION)
                cerror_return_discards_quals(self->logger, vloc, r.discarded_quals);
        else if (r.kind == CACRK_INCOMPATIBLE_POINTERS)
                cerror_return_from_incompatible_pointer_type(self->logger, vloc);
        return NULL;
}

extern bool csema_check_stmt(const csema* self, const tree_stmt* s, cstmt_context c)
{
        if (!s)
                return false;

        tree_scope_flags flags = cstmt_context_to_scope_flags(c);
        bool is_top_level = self->scope == NULL;
        tree_stmt_kind sk = tree_get_stmt_kind(s);

        if (sk == TSK_BREAK && !(flags & TSF_BREAK))
        {
                cerror_break_stmt_outside_loop_or_switch(self->logger, s);
                return false;
        }
        else if (sk == TSK_CONTINUE && !(flags & TSF_CONTINUE))
        {
                cerror_continue_stmt_outside_loop(self->logger, s);
                return false;
        }
        else if (sk == TSK_DECL && !(c & CSC_DECL))
        {
                cerror_decl_stmt_outside_block(self->logger, s);
                return false;
        }
        else if (sk == TSK_COMPOUND && is_top_level)
        {
                TREE_FOREACH_DECL_IN_LOOKUP(self->labels, it)
                {
                        tree_decl* label = hiter_get_ptr(&it);
                        if (!tree_get_label_decl_stmt(label))
                        {
                                cerror_undefined_label(self->logger, label);
                                return false;
                        }
                }
        }
        return true;
}