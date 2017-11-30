#include "scc/c/c-sema-stmt.h"
#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-expr.h"
#include "scc/c/c-info.h"

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
        tree_expr* value,
        tree_stmt* body)
{
        if (!dseq_size(&self->switch_stack))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                        "a case statement may only be used within a switch");
                return false;
        }

        if (!csema_require_integer_expr(self, value))
                return NULL;

        tree_stmt* switch_ = dseq_last_ptr(&self->switch_stack);
        value = csema_new_impl_cast(self, value,
                tree_get_expr_type(tree_get_switch_expr(switch_)));

        return tree_new_case_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), value, body);
}

extern tree_stmt* csema_new_default_stmt(
        csema* self, tree_location kw_loc, tree_location colon_loc, tree_stmt* body)
{
        if (!dseq_size(&self->switch_stack))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                        "a default statement may only be used within a switch");
                return false;
        }

        return tree_new_default_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), body);
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

        dseq_append_ptr(&self->switch_stack, s);
        return s;
}

extern tree_stmt* csema_finish_switch_stmt(csema* self, tree_stmt* switch_, tree_stmt* body)
{
        if (!body)
                return NULL;

        tree_set_switch_body(switch_, body);
        dseq_pop_ptr(&self->switch_stack);
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
        tree_location loc = tree_get_decl_loc_begin(d);
        const char* name = csema_get_id_cstr(self, tree_get_decl_name(d));

        if (!tree_decl_is(d, TDK_VAR))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "declaration of non-variable '%s' in 'for' loop", name);
                return false;
        }

        tree_decl_storage_class sc = tree_get_decl_storage_class(d);
        if (sc != TDSC_NONE && sc != TDSC_AUTO && sc != TDSC_REGISTER)
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "declaration of '%s' variable '%s' in 'for' loop initial declaration",
                        cget_decl_storage_class_string(sc), name);
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
                cerror(self->error_manager, CES_ERROR, tree_get_expr_loc(value),
                        "return with a value, in function returning void");
                return NULL;
        }

        cassign_conv_result r;
        if (!value || csema_assignment_conversion(self, restype, &value, &r))
                return tree_new_return_stmt(self->context,
                        tree_init_xloc(kw_loc, semicolon_loc), value);

        tree_type* vt = tree_get_expr_type(value);
        tree_location vloc = tree_get_expr_loc(value);

        if (r.kind == CACRK_INCOMPATIBLE)
                cerror(self->error_manager, CES_ERROR, vloc,
                        "return type does not match the function type");
        else if (r.kind == CACRK_RHS_NOT_AN_ARITHMETIC)
                csema_require_arithmetic_expr_type(self, vt, vloc);
        else if (r.kind == CACRK_RHS_NOT_A_RECORD)
                csema_require_record_expr_type(self, vt, vloc);
        else if (r.kind == CACRK_INCOMPATIBLE_RECORDS)
                csema_require_compatible_expr_types(self, restype, vt, vloc);
        else if (r.kind == CACRK_QUAL_DISCARTION)
        {
                char quals[64];
                cqet_qual_string(r.discarded_quals, quals);
                cerror(self->error_manager, CES_ERROR, vloc,
                        "return discards '%s' qualifier", quals);
        }
        else if (r.kind == CACRK_INCOMPATIBLE_POINTERS)
                cerror(self->error_manager, CES_ERROR, vloc,
                        "return from incompatible pointer type");
        return NULL;
}

extern bool csema_check_stmt(const csema* self, const tree_stmt* s, cstmt_context c)
{
        if (!s)
                return false;

        tree_scope_flags flags = cstmt_context_to_scope_flags(c);
        bool is_top_level = self->scope == NULL;
        tree_stmt_kind sk = tree_get_stmt_kind(s);
        tree_location kw_loc = tree_get_xloc_begin(tree_get_stmt_loc(s));

        if (sk == TSK_BREAK && !(flags & TSF_BREAK))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                       "a break statement may only be used within a loop or switch");
                return false;
        }
        else if (sk == TSK_CONTINUE && !(flags & TSF_CONTINUE))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                       "a continue statement may only be used within a loop");
                return false;
        }
        else if (sk == TSK_DECL && !(c & CSC_DECL))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                        "a declaration may only be used within a block");
                return false;
        }
        else if (sk == TSK_COMPOUND && is_top_level)
        {
                TREE_FOREACH_DECL_IN_LOOKUP(self->labels, it)
                {
                        tree_decl* label = hiter_get_ptr(&it);
                        if (!tree_get_label_decl_stmt(label))
                        {
                                tree_location l = tree_get_decl_loc_begin(label);
                                const char* n = csema_get_id_cstr(self, tree_get_decl_name(label));
                                cerror(self->error_manager, CES_ERROR, l,
                                        "label '%s' used but not defined", n);
                                return false;
                        }
                }
        }
        return true;
}