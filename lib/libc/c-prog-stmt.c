#include "c-prog-stmt.h"
#include "c-prog-conversions.h"
#include "c-prog-decl.h"

extern tree_stmt* cprog_finish_stmt(cprog* self, tree_stmt* s)
{
        if (!s)
                return NULL;
    
        bool is_top_level = self->scope == NULL;
        tree_stmt_kind sk = tree_get_stmt_kind(s);
        tree_scope_flags flags = tree_get_scope_flags(self->scope);
        tree_location kw_loc = tree_get_xloc_begin(tree_get_stmt_loc(s));

        if (sk == TSK_BREAK && !(flags & TSF_BREAK))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                        "a break statement may only be used within a loop or switch");
                return NULL;
        }
        else if (sk == TSK_CONTINUE && !(flags & TSF_CONTINUE))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                        "a continue statement may only be used within a loop");
                return NULL;
        }
        else if (sk == TSK_CASE && !(flags & TSF_SWITCH))
        {
                cerror(self->error_manager, CES_ERROR, kw_loc,
                        "a case statement may only be used within a switch");
                return NULL;
        }
        else if (sk == TSK_COMPOUND && is_top_level)
        {
                // check for unknown labels
                return s;
        }

        tree_scope_add(self->scope, s);
        return s;
}

extern tree_stmt* cprog_build_block_stmt(
        cprog* self, tree_location lbrace_loc, cstmt_context context)
{
        tree_scope_flags flags = TSF_NONE;
        if (context == CSC_ITERATION)
                flags = TSF_ITERATION;
        else if (context == CSC_SWITCH)
                flags = TSF_SWITCH;
        return tree_new_compound_stmt_ex(self->context,
                tree_init_xloc(lbrace_loc, 0), self->scope, self->locals, flags);
}

extern tree_stmt* cprog_build_case_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_exp* value,
        tree_stmt* body)
{
        return tree_new_case_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), value, body);
}

extern tree_stmt* cprog_build_default_stmt(
        cprog* self, tree_location kw_loc, tree_location colon_loc, tree_stmt* body)
{
        return tree_new_default_stmt(self->context,
                tree_init_xloc(kw_loc, colon_loc), body);
}

static tree_decl* cprog_finish_label(cprog* self, tree_decl* label)
{
        if (!cprog_export_decl(self, self->labels, label))
                return NULL;

        tree_decl_scope_insert(self->labels, label);
        return label;
}

extern tree_stmt* cprog_build_labeled_stmt(
        cprog* self,
        tree_location id_loc,
        tree_location colon_loc,
        tree_id name,
        tree_stmt* target)
{
        tree_decl* label = cprog_get_label_decl(self, name);
        if (label && tree_get_label_decl_stmt(label))
                return NULL; // label is already defined
        else if (!label)
        {
                label = cprog_finish_label(self,
                        tree_new_label_decl(self->context, self->labels, 0, name, target));
                if (!label)
                        return NULL;
        }

        tree_set_label_decl_stmt(label, target);
        return tree_new_labeled_stmt(self->context,
                tree_init_xloc(id_loc, colon_loc), label);
}

extern tree_stmt* cprog_build_exp_stmt(
        cprog* self, tree_location begin_loc, tree_location semicolon_loc, tree_exp* exp)
{
        return tree_new_exp_stmt(self->context,
                tree_init_xloc(begin_loc, semicolon_loc), exp);
}

extern tree_stmt* cprog_build_if_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_exp* condition,
        tree_stmt* body,
        tree_stmt* else_)
{
        return tree_new_if_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), condition, body, else_);
}

extern tree_stmt* cprog_build_decl_stmt(
        cprog* self, tree_location begin_loc, tree_location semicolon_loc, tree_decl* d)
{
        return tree_new_decl_stmt(self->context,
                tree_init_xloc(begin_loc, semicolon_loc), d);
}

extern tree_stmt* cprog_build_switch_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_exp* value,
        tree_stmt* body)
{
        return tree_new_switch_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), body, value);
}

extern tree_stmt* cprog_build_while_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_exp* condition,
        tree_stmt* body)
{
        return tree_new_while_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), condition, body);
}

extern tree_stmt* cprog_build_do_while_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_exp* condition,
        tree_stmt* body)
{
        return tree_new_do_while_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc), condition, body);
}

extern tree_stmt* cprog_build_for_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_exp* condition,
        tree_exp* step,
        tree_stmt* body)
{
        return tree_new_for_stmt(self->context,
                tree_init_xloc(kw_loc, rbracket_loc), init, condition, step, body);
}

extern tree_stmt* cprog_build_goto_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc, tree_id label)
{
        tree_decl* ld = cprog_get_label_decl(self, label);
        if (!ld)
        {
                ld = cprog_finish_label(self,
                        tree_new_label_decl(self->context, self->labels, 0, label, NULL));
                if (!ld)
                        return NULL;
        }
        return tree_new_goto_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc), ld);
}

extern tree_stmt* cprog_build_continue_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_continue_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* cprog_build_break_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc)
{
        return tree_new_break_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc));
}

extern tree_stmt* cprog_build_return_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc, tree_exp* value)
{
        if (value)
                value = cprog_build_impl_cast(self, value,
                        tree_get_function_restype(tree_get_decl_type(self->function)));

        return tree_new_return_stmt(self->context,
                tree_init_xloc(kw_loc, semicolon_loc), value);
}