#include "scc/tree/tree-stmt.h"
#include "scc/tree/tree-context.h"

extern void tree_init_scope(
        tree_scope* self, tree_context* context, tree_scope* parent)
{
        tree_init_scope_ex(self, context, parent, tree_get_scope_decls(parent));
}

extern void tree_init_scope_ex(
        tree_scope* self,
        tree_context* context,
        tree_scope* parent,
        tree_decl_scope* decl_parent)
{
        tree_init_decl_scope(tree_get_scope_decls(self), context, decl_parent);
        list_init(&self->stmts);
        self->parent = parent;
}

extern void tree_add_stmt_to_scope(tree_scope* self, tree_stmt* s)
{
        list_push_back(&self->stmts, &s->base.node);
}

extern tree_stmt* tree_new_stmt(
        tree_context* context, tree_stmt_kind kind, tree_xlocation loc, size_t size)
{
        tree_stmt* s = tree_allocate_node(context, size);
        if (!s)
                return NULL;

        tree_set_stmt_kind(s, kind);
        tree_set_stmt_loc(s, loc);
        list_node_init(&s->base.node);
        return s;
}

extern tree_stmt* tree_new_labeled_stmt(tree_context* context, tree_xlocation loc, tree_decl* label)
{
        tree_stmt* s = tree_new_stmt(context, TSK_LABELED, loc, sizeof(struct _tree_labeled_stmt));
        if (!s)
                return NULL;

        tree_set_label_stmt_decl(s, label);
        return s;
}

extern tree_stmt* tree_new_case_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_expr* expr,
        const int_value* value,
        tree_stmt* body)
{
        tree_stmt* s = tree_new_stmt(context, TSK_CASE, loc, sizeof(struct _tree_case_stmt));
        if (!s)
                return NULL;

        tree_set_case_body(s, body);
        tree_set_case_expr(s, expr);
        tree_set_case_value(s, value);
        return s;
}

extern tree_stmt* tree_new_default_stmt(tree_context* context, tree_xlocation loc, tree_stmt* body)
{
        tree_stmt* s = tree_new_stmt(context, TSK_DEFAULT, loc, sizeof(struct _tree_default_stmt));
        if (!s)
                return NULL;

        tree_set_default_body(s, body);
        return s;
}

extern tree_stmt* tree_new_compound_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_scope* parent,
        tree_decl_scope* decl_parent)
{
        tree_stmt* s = tree_new_stmt(context, TSK_COMPOUND, loc, sizeof(struct _tree_compound_stmt));
        if (!s)
                return NULL;

        tree_init_scope_ex(tree_get_compound_scope(s), context, parent, decl_parent);
        return s;
}

extern tree_stmt* tree_new_decl_stmt(tree_context* context, tree_xlocation loc, tree_decl* d)
{
        tree_stmt* s = tree_new_stmt(context, TSK_DECL, loc, sizeof(struct _tree_decl_stmt));
        if (!s)
                return NULL;

        tree_set_decl_stmt_entity(s, d);
        return s;
}

extern tree_stmt* tree_new_expr_stmt(tree_context* context, tree_xlocation loc, tree_expr* root)
{
        tree_stmt* s = tree_new_stmt(context, TSK_EXPR, loc, sizeof(struct _tree_expr_stmt));
        if (!s)
                return NULL;

        tree_set_expr_stmt_root(s, root);
        return s;
}

extern tree_stmt* tree_new_if_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_)
{
        tree_stmt* s = tree_new_stmt(context, TSK_IF, loc, sizeof(struct _tree_if_stmt));
        if (!s)
                return NULL;

        tree_set_if_condition(s, condition);
        tree_set_if_body(s, body);
        tree_set_if_else(s, else_);
        return s;
}

extern tree_stmt* tree_new_switch_stmt(tree_context* context,
        tree_xlocation loc,
        tree_stmt* body,
        tree_expr* expr)
{
        tree_stmt* s = tree_new_stmt(context, TSK_SWITCH, loc, sizeof(struct _tree_switch_stmt));
        if (!s)
                return NULL;

        tree_set_switch_body(s, body);
        tree_set_switch_expr(s, expr);
        return s;
}

extern tree_stmt* tree_new_while_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* condition, tree_stmt* body)
{
        tree_stmt* s = tree_new_stmt(context, TSK_WHILE, loc, sizeof(struct _tree_while_stmt));
        if (!s)
                return NULL;

        tree_set_while_condition(s, condition);
        tree_set_while_body(s, body);
        return s;
}

extern tree_stmt* tree_new_do_while_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* condition, tree_stmt* body)
{
        tree_stmt* s = tree_new_stmt(context, TSK_DO_WHILE, loc, sizeof(struct _tree_do_while_stmt));
        if (!s)
                return NULL;

        tree_set_do_while_condition(s, condition);
        tree_set_do_while_body(s, body);
        return s;
}

extern tree_stmt* tree_new_for_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_stmt* init,
        tree_expr* condition,
        tree_expr* step,
        tree_stmt* body)
{
        tree_stmt* s = tree_new_stmt(context, TSK_FOR, loc, sizeof(struct _tree_for_stmt));
        if (!s)
                return NULL;

        tree_set_for_init(s, init);
        tree_set_for_condition(s, condition);
        tree_set_for_step(s, step);
        tree_set_for_body(s, body);
        return s;
}

extern tree_stmt* tree_new_goto_stmt(tree_context* context, tree_xlocation loc, tree_decl* label)
{
        tree_stmt* s = tree_new_stmt(context, TSK_GOTO, loc, sizeof(struct _tree_goto_stmt));
        if (!s)
                return NULL;

        tree_set_goto_label(s, label);
        return s;
}

extern tree_stmt* tree_new_break_stmt(tree_context* context, tree_xlocation loc)
{
        return tree_new_stmt(context, TSK_BREAK, loc, sizeof(struct _tree_break_stmt));
}

extern tree_stmt* tree_new_continue_stmt(tree_context* context, tree_xlocation loc)
{
        return tree_new_stmt(context, TSK_CONTINUE, loc, sizeof(struct _tree_continue_stmt));
}

extern tree_stmt* tree_new_return_stmt(tree_context* context, tree_xlocation loc, tree_expr* value)
{
        tree_stmt* s = tree_new_stmt(context, TSK_RETURN, loc, sizeof(struct _tree_return_stmt));
        if (!s)
                return NULL;

        tree_set_return_value(s, value);
        return s;
}

extern tree_stmt* tree_new_atomic_stmt(tree_context* context, tree_xlocation loc, tree_stmt* body)
{
        tree_stmt* s = tree_new_stmt(context, TSK_ATOMIC, loc, sizeof(struct _tree_atomic_stmt));
        if (!s)
                return NULL;

        tree_set_atomic_body(s, body);
        return s;
}