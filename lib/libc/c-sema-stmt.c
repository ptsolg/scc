#include "c-sema-stmt.h"
#include "c-sema-conv.h"
#include "c-sema-decl.h"

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
        return tree_new_case_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), value, body);
}

extern tree_stmt* csema_new_default_stmt(
        csema* self, tree_location kw_loc, tree_location colon_loc, tree_stmt* body)
{
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
        return tree_new_switch_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), body, value);
}

extern tree_stmt* csema_new_while_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body)
{
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
        return tree_new_do_while_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc), condition, body);
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
        return tree_new_for_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), init, condition, step, body);
}

extern tree_stmt* csema_new_goto_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc, tree_id label)
{
        tree_decl* l = csema_forward_label_decl(self, label);
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
        if (value)
                value = csema_new_impl_cast(self, value,
                        tree_get_function_restype(tree_get_decl_type(self->function)));

        return tree_new_return_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc), value);
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
        else if ((sk == TSK_CASE || sk == TSK_DEFAULT) && (flags & TSF_SWITCH) != TSF_SWITCH)
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                       "a %s statement may only be used within a switch",
                       (sk == TSK_CASE ? "case" : "default"));
                return false;
        }
        else if (sk == TSK_COMPOUND && is_top_level)
        {
                // check for unknown labels
        }
        return true;
}