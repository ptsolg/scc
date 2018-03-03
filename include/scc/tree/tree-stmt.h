#ifndef TREE_STMT_H
#define TREE_STMT_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-decl.h"

typedef struct _tree_stmt tree_stmt;
typedef struct _tree_decl tree_decl;
typedef struct _tree_expr tree_expr;
typedef struct _tree_scope tree_scope;

typedef enum
{
        TSF_NONE = 0,
        TSF_BREAK = 1,
        TSF_CONTINUE = 2,
        TSF_ITERATION = TSF_BREAK | TSF_CONTINUE,
        TSF_SWITCH = 4 | TSF_BREAK,
        TSF_DECL = 8,
} tree_scope_flags;

typedef struct _tree_scope
{
        tree_scope* parent;
        tree_decl_scope decls;
        tree_scope_flags flags;
        list_head stmts;
} tree_scope;

extern void tree_init_scope(
        tree_scope* self, tree_context* context, tree_scope* parent, tree_scope_flags flags);

extern void tree_init_scope_ex(
        tree_scope* self,
        tree_context* context,
        tree_scope* parent,
        tree_decl_scope* decl_parent,
        tree_scope_flags flags);

extern void tree_add_stmt_to_scope(tree_scope* self, tree_stmt* s);

static TREE_INLINE tree_scope_flags tree_get_scope_flags(const tree_scope* self)
{
        return self->flags;
}

static TREE_INLINE tree_decl_scope* tree_get_scope_decls(tree_scope* self)
{
        return &self->decls;
}

static TREE_INLINE const tree_decl_scope* tree_get_scope_cdecls(const tree_scope* self)
{
        return &self->decls;
}

static TREE_INLINE tree_stmt* tree_get_scope_stmts_begin(const tree_scope* self)
{
        return (tree_stmt*)list_begin(&self->stmts);
}

static TREE_INLINE const tree_stmt* tree_get_scope_stmts_end(const tree_scope* self)
{
        return (const tree_stmt*)list_cend(&self->stmts);
}

static TREE_INLINE tree_scope* tree_get_scope_parent(const tree_scope* self)
{
        return self->parent;
}

static TREE_INLINE void tree_set_scope_flags(tree_scope* self, tree_scope_flags f)
{
        self->flags = f;
}

static TREE_INLINE void tree_add_scope_flags(tree_scope* self, tree_scope_flags f)
{
        self->flags = (tree_scope_flags)(self->flags | f);
}

#define TREE_FOREACH_STMT(PSCOPE, ITNAME) \
        for (tree_stmt* ITNAME = tree_get_scope_stmts_begin(PSCOPE); \
                ITNAME != tree_get_scope_stmts_end(PSCOPE); \
                ITNAME = tree_get_next_stmt(ITNAME))

typedef enum _tree_stmt_kind
{
        TSK_UNKNOWN,
        TSK_LABELED,
        TSK_CASE,
        TSK_DEFAULT,
        TSK_COMPOUND,
        TSK_EXPR,
        TSK_IF,
        TSK_SWITCH,
        TSK_WHILE,
        TSK_DO_WHILE,
        TSK_FOR,
        TSK_GOTO,
        TSK_CONTINUE,
        TSK_BREAK,
        TSK_DECL,
        TSK_RETURN,
        TSK_SIZE,
} tree_stmt_kind;

struct _tree_stmt_base
{
        list_node node;
        tree_stmt_kind kind;
        tree_xlocation loc;
};

struct _tree_labeled_stmt
{
        struct _tree_stmt_base base;
        tree_decl* label;
};

struct _tree_case_stmt
{
        struct _tree_stmt_base base;
        tree_expr* expr;
        tree_stmt* body;
        int_value value;
};

struct _tree_default_stmt
{
        struct _tree_stmt_base base;
        tree_stmt* body;
};

struct _tree_compound_stmt
{
        struct _tree_stmt_base base;
        tree_scope scope;
};

struct _tree_decl_stmt
{
        struct _tree_stmt_base base;
        tree_decl* entity;
};

struct _tree_expr_stmt
{
        struct _tree_stmt_base base;
        tree_expr* root;
};

struct _tree_if_stmt
{
        struct _tree_stmt_base base;
        tree_expr* condition;
        tree_stmt* body;
        tree_stmt* else_stmt;
};

struct _tree_switch_stmt
{
        struct _tree_stmt_base base;
        tree_stmt* body;
        tree_expr* expr;
};

struct _tree_while_stmt
{
        struct _tree_stmt_base base;
        tree_expr* condition;
        tree_stmt* body;
};

struct _tree_do_while_stmt
{
        struct _tree_stmt_base base;
        tree_expr* condition;
        tree_stmt* body;
};

struct _tree_for_stmt
{
        struct _tree_stmt_base base;
        tree_stmt* init;
        tree_expr* condition;
        tree_expr* step;
        tree_stmt* body;
};

struct _tree_goto_stmt
{
        struct _tree_stmt_base base;
        tree_decl* label;
};

struct _tree_continue_stmt
{
        struct _tree_stmt_base base;
};

struct _tree_break_stmt
{
        struct _tree_stmt_base base;
};

struct _tree_return_stmt
{
        struct _tree_stmt_base base;
        tree_expr* value;
};

typedef struct _tree_stmt
{
        union
        {
                struct _tree_stmt_base base;
                struct _tree_labeled_stmt labeled;
                struct _tree_case_stmt case_stmt;
                struct _tree_default_stmt default_stmt;
                struct _tree_compound_stmt compound;
                struct _tree_expr_stmt expr;
                struct _tree_if_stmt if_stmt;
                struct _tree_switch_stmt switch_stmt;
                struct _tree_while_stmt while_stmt;
                struct _tree_do_while_stmt do_while;
                struct _tree_for_stmt for_stmt;
                struct _tree_goto_stmt goto_stmt;
                struct _tree_continue_stmt continue_stmt;
                struct _tree_break_stmt break_stmt;
                struct _tree_return_stmt return_stmt;
                struct _tree_decl_stmt decl;
        };
} tree_stmt;

extern tree_stmt* tree_new_stmt(
        tree_context* context, tree_stmt_kind kind, tree_xlocation loc, size_t size);

static TREE_INLINE tree_stmt_kind tree_get_stmt_kind(const tree_stmt* self)
{
        return self->base.kind;
}

static TREE_INLINE tree_xlocation tree_get_stmt_loc(const tree_stmt* self)
{
        return self->base.loc;
}

static TREE_INLINE tree_stmt* tree_get_next_stmt(const tree_stmt* self)
{
        return (tree_stmt*)list_node_next(&self->base.node);
}

static TREE_INLINE bool tree_stmt_is(const tree_stmt* self, tree_stmt_kind k)
{
        return tree_get_stmt_kind(self) == k;
}

static TREE_INLINE void tree_set_stmt_kind(tree_stmt* self, tree_stmt_kind k)
{
        self->base.kind = k;
}

static TREE_INLINE void tree_set_stmt_loc(tree_stmt* self, tree_xlocation l)
{
        self->base.loc = l;
}

extern tree_stmt* tree_new_labeled_stmt(tree_context* context, tree_xlocation loc, tree_decl* label);

static TREE_INLINE tree_decl* tree_get_label_stmt_decl(const tree_stmt* self)
{
        return self->labeled.label;
}

static TREE_INLINE void tree_set_label_stmt_decl(tree_stmt* self, tree_decl* label)
{
        self->labeled.label = label;
}

extern tree_stmt* tree_new_case_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_expr* expr,
        const int_value* value,
        tree_stmt* body);

static TREE_INLINE tree_expr* tree_get_case_expr(const tree_stmt* self)
{
        return self->case_stmt.expr;
}

static TREE_INLINE const int_value* tree_get_case_cvalue(const tree_stmt* self)
{
        return &self->case_stmt.value;
}

static TREE_INLINE tree_stmt* tree_get_case_body(const tree_stmt* self)
{
        return self->case_stmt.body;
}

static TREE_INLINE void tree_set_case_expr(tree_stmt* self, tree_expr* value)
{
        self->case_stmt.expr = value;
}

static TREE_INLINE void tree_set_case_body(tree_stmt* self, tree_stmt* body)
{
        self->case_stmt.body = body;
}

static TREE_INLINE void tree_set_case_value(tree_stmt* self, const int_value* value)
{
        self->case_stmt.value = *value;
}

extern tree_stmt* tree_new_default_stmt(tree_context* context, tree_xlocation loc, tree_stmt* body);

static TREE_INLINE tree_stmt* tree_get_default_body(const tree_stmt* self)
{
        return self->default_stmt.body;
}

static TREE_INLINE void tree_set_default_body(tree_stmt* self, tree_stmt* body)
{
        self->default_stmt.body = body;
}

extern tree_stmt* tree_new_compound_stmt(
        tree_context* context, tree_xlocation loc, tree_scope* parent, tree_scope_flags flags);

extern tree_stmt* tree_new_compound_stmt_ex(
        tree_context* context,
        tree_xlocation loc,
        tree_scope* parent,
        tree_decl_scope* decl_parent,
        tree_scope_flags flags);

static TREE_INLINE tree_scope* tree_get_compound_scope(tree_stmt* self)
{
        return &self->compound.scope;
}

static TREE_INLINE const tree_scope* tree_get_compound_cscope(const tree_stmt* self)
{
        return &self->compound.scope;
}

extern tree_stmt* tree_new_decl_stmt(tree_context* context, tree_xlocation loc, tree_decl* d);

static TREE_INLINE tree_decl* tree_get_decl_stmt_entity(const tree_stmt* self)
{
        return self->decl.entity;
}

static TREE_INLINE void tree_set_decl_stmt_entity(tree_stmt* self, tree_decl* d)
{
        self->decl.entity = d;
}

extern tree_stmt* tree_new_expr_stmt(tree_context* context, tree_xlocation loc, tree_expr* root);

static TREE_INLINE tree_expr* tree_get_expr_stmt_root(const tree_stmt* self)
{
        return self->expr.root;
}

static TREE_INLINE void tree_set_expr_stmt_root(tree_stmt* self, tree_expr* expr)
{
        self->expr.root = expr;
}

extern tree_stmt* tree_new_if_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_);

static TREE_INLINE tree_expr* tree_get_if_condition(const tree_stmt* self)
{
        return self->if_stmt.condition;
}

static TREE_INLINE tree_stmt* tree_get_if_body(const tree_stmt* self)
{
        return self->if_stmt.body;
}

static TREE_INLINE tree_stmt* tree_get_if_else(const tree_stmt* self)
{
        return self->if_stmt.else_stmt;
}

static TREE_INLINE void tree_set_if_condition(tree_stmt* self, tree_expr* expr)
{
        self->if_stmt.condition = expr;
}

static TREE_INLINE void tree_set_if_body(tree_stmt* self, tree_stmt* body)
{
        self->if_stmt.body = body;
}

static TREE_INLINE void tree_set_if_else(tree_stmt* self, tree_stmt* else_stmt)
{
        self->if_stmt.else_stmt = else_stmt;
}

extern tree_stmt* tree_new_switch_stmt(tree_context* context,
        tree_xlocation loc,
        tree_stmt* body,
        tree_expr* expr);

static TREE_INLINE tree_expr* tree_get_switch_expr(const tree_stmt* self)
{
        return self->switch_stmt.expr;
}

static TREE_INLINE tree_stmt* tree_get_switch_body(const tree_stmt* self)
{
        return self->switch_stmt.body;
}

static TREE_INLINE void tree_set_switch_expr(tree_stmt* self, tree_expr* expr)
{
        self->switch_stmt.expr = expr;
}

static TREE_INLINE void tree_set_switch_body(tree_stmt* self, tree_stmt* body)
{
        self->switch_stmt.body = body;
}

extern tree_stmt* tree_new_while_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* condition, tree_stmt* body);

static TREE_INLINE tree_expr* tree_get_while_condition(const tree_stmt* self)
{
        return self->while_stmt.condition;
}

static TREE_INLINE tree_stmt* tree_get_while_body(const tree_stmt* self)
{
        return self->while_stmt.body;
}

static TREE_INLINE void tree_set_while_condition(tree_stmt* self, tree_expr* expr)
{
        self->while_stmt.condition = expr;
}

static TREE_INLINE void tree_set_while_body(tree_stmt* self, tree_stmt* body)
{
        self->while_stmt.body = body;
}

extern tree_stmt* tree_new_do_while_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* condition, tree_stmt* body);

static TREE_INLINE tree_expr* tree_get_do_while_condition(const tree_stmt* self)
{
        return self->do_while.condition;
}

static TREE_INLINE tree_stmt* tree_get_do_while_body(const tree_stmt* self)
{
        return self->do_while.body;
}

static TREE_INLINE void tree_set_do_while_condition(tree_stmt* self, tree_expr* expr)
{
        self->do_while.condition = expr;
}

static TREE_INLINE void tree_set_do_while_body(tree_stmt* self, tree_stmt* body)
{
        self->do_while.body = body;
}

extern tree_stmt* tree_new_for_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_stmt* init,
        tree_expr* condition,
        tree_expr* step,
        tree_stmt* body);

static TREE_INLINE tree_stmt* tree_get_for_init(const tree_stmt* self)
{
        return self->for_stmt.init;
}

static TREE_INLINE tree_expr* tree_get_for_condition(const tree_stmt* self)
{
        return self->for_stmt.condition;
}

static TREE_INLINE tree_expr* tree_get_for_step(const tree_stmt* self)
{
        return self->for_stmt.step;
}

static TREE_INLINE tree_stmt* tree_get_for_body(const tree_stmt* self)
{
        return self->for_stmt.body;
}

static TREE_INLINE void tree_set_for_init(tree_stmt* self, tree_stmt* init)
{
        self->for_stmt.init = init;
}

static TREE_INLINE void tree_set_for_condition(tree_stmt* self, tree_expr* condition)
{
        self->for_stmt.condition = condition;
}

static TREE_INLINE void tree_set_for_step(tree_stmt* self, tree_expr* step)
{
        self->for_stmt.step = step;
}

static TREE_INLINE void tree_set_for_body(tree_stmt* self, tree_stmt* body)
{
        self->for_stmt.body = body;
}

extern tree_stmt* tree_new_goto_stmt(tree_context* context, tree_xlocation loc, tree_decl* label);

static TREE_INLINE tree_decl* tree_get_goto_label(const tree_stmt* self)
{
        return self->goto_stmt.label;
}

static TREE_INLINE void tree_set_goto_label(tree_stmt* self, tree_decl* label)
{
        self->goto_stmt.label = label;
}

extern tree_stmt* tree_new_continue_stmt(tree_context* context, tree_xlocation loc);
extern tree_stmt* tree_new_break_stmt(tree_context* context, tree_xlocation loc);
extern tree_stmt* tree_new_return_stmt(tree_context* context, tree_xlocation loc, tree_expr* value);

static TREE_INLINE tree_expr* tree_get_return_value(const tree_stmt* self)
{
        return self->return_stmt.value;
}

static TREE_INLINE void tree_set_return_value(tree_stmt* self, tree_expr* expr)
{
        self->return_stmt.value = expr;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_STMT_H
