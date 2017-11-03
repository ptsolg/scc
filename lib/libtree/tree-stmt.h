#ifndef TREE_STMT_H
#define TREE_STMT_H

#ifdef S_HAS_PRAGMA
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
} tree_scope_flags;

typedef struct _tree_scope
{
        tree_scope* _parent;
        tree_decl_scope _decls;
        tree_scope_flags _flags;
        list_head _stmts;
} tree_scope;

extern void tree_init_scope(
        tree_scope* self, tree_context* context, tree_scope* parent, tree_scope_flags flags);

extern void tree_init_scope_ex(
        tree_scope* self,
        tree_context* context,
        tree_scope* parent,
        tree_decl_scope* decl_parent,
        tree_scope_flags flags);

extern void tree_dispose_scope(tree_scope* self);
extern void tree_scope_add(tree_scope* self, tree_stmt* s);

static inline tree_scope_flags tree_get_scope_flags(const tree_scope* self);
static inline tree_decl_scope* tree_get_scope_decls(tree_scope* self);
static inline const tree_decl_scope* tree_get_scope_cdecls(const tree_scope* self);
static inline tree_stmt* tree_get_scope_begin(const tree_scope* self);
static inline const tree_stmt* tree_get_scope_end(const tree_scope* self);
static inline tree_scope* tree_get_scope_parent(const tree_scope* self);

static inline void tree_set_scope_flags(tree_scope* self, tree_scope_flags f);
static inline void tree_add_scope_flags(tree_scope* self, tree_scope_flags f);

#define TREE_FOREACH_STMT(PSCOPE, ITNAME) \
        for (tree_stmt* ITNAME = tree_get_scope_begin(PSCOPE); \
                ITNAME != tree_get_scope_end(PSCOPE); \
                ITNAME = tree_get_next_stmt(ITNAME))

typedef enum _tree_stmt_kind
{
        TSK_UNKNOWN,
        TSK_LABELED,
        TSK_CASE,
        TSK_DEFAULT,
        TSK_COMPOUND,
        TSK_expr,
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
} tree_stmt_kind;

struct _tree_stmt_base
{
        list_node _node;
        tree_stmt_kind _kind;
        tree_xlocation _loc;
};

extern tree_stmt* tree_new_stmt(
        tree_context* context, tree_stmt_kind kind, tree_xlocation loc, ssize size);

static inline struct _tree_stmt_base* _tree_get_stmt(tree_stmt* self);
static inline const struct _tree_stmt_base* _tree_get_cstmt(const tree_stmt* self);

static inline tree_stmt_kind tree_get_stmt_kind(const tree_stmt* self);
static inline tree_xlocation tree_get_stmt_loc(const tree_stmt* self);
static inline tree_stmt* tree_get_next_stmt(const tree_stmt* self);
static inline bool tree_stmt_is(const tree_stmt* self, tree_stmt_kind k);

static inline void tree_set_stmt_kind(tree_stmt* self, tree_stmt_kind k);
static inline void tree_set_stmt_loc(tree_stmt* self, tree_xlocation l);

struct _tree_labeled_stmt
{
        struct _tree_stmt_base _base;
        tree_decl* _label;
};

extern tree_stmt* tree_new_labeled_stmt(tree_context* context, tree_xlocation loc, tree_decl* label);


static inline struct _tree_labeled_stmt* _tree_get_label_stmt(tree_stmt* self);
static inline const struct _tree_labeled_stmt* _tree_get_label_cstmt(const tree_stmt* self);

static inline tree_decl* tree_get_label_stmt_decl(const tree_stmt* self);
static inline void tree_set_label_stmt_decl(tree_stmt* self, tree_decl* label);

struct _tree_case_stmt
{
        struct _tree_stmt_base _base;
        tree_expr* _value;
        tree_stmt* _body;
};

extern tree_stmt* tree_new_case_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* value, tree_stmt* body);

static inline struct _tree_case_stmt* _tree_get_case(tree_stmt* self);
static inline const struct _tree_case_stmt* _tree_get_ccase(const tree_stmt* self);

static inline tree_expr* tree_get_case_value(const tree_stmt* self);
static inline tree_stmt* tree_get_case_body(const tree_stmt* self);

static inline void tree_set_case_value(tree_stmt* self, tree_expr* value);
static inline void tree_set_case_body(tree_stmt* self, tree_stmt* body);

struct _tree_default_stmt
{
        struct _tree_stmt_base _base;
        tree_stmt* _body;
};

extern tree_stmt* tree_new_default_stmt(tree_context* context, tree_xlocation loc, tree_stmt* body);

static inline struct _tree_default_stmt* _tree_get_default(tree_stmt* self);
static inline const struct _tree_default_stmt* _tree_get_cdefault(const tree_stmt* self);

static inline tree_stmt* tree_get_default_body(const tree_stmt* self);
static inline void tree_set_default_body(tree_stmt* self, tree_stmt* body);

struct _tree_compound_stmt
{
        struct _tree_stmt_base _base;
        tree_scope _scope;
};

extern tree_stmt* tree_new_compound_stmt(
        tree_context* context, tree_xlocation loc, tree_scope* parent, tree_scope_flags flags);

extern tree_stmt* tree_new_compound_stmt_ex(
        tree_context* context,
        tree_xlocation loc,
        tree_scope* parent,
        tree_decl_scope* decl_parent,
        tree_scope_flags flags);

static inline struct _tree_compound_stmt* _tree_get_compound(tree_stmt* self);
static inline const struct _tree_compound_stmt* _tree_get_ccompound(const tree_stmt* self);

static inline tree_scope* tree_get_compound_scope(tree_stmt* self);
static inline const tree_scope* tree_get_compound_cscope(const tree_stmt* self);

struct _tree_decl_stmt
{
        struct _tree_stmt_base _base;
        tree_decl* _entity;
};

extern tree_stmt* tree_new_decl_stmt(tree_context* context, tree_xlocation loc, tree_decl* d);

static inline struct _tree_decl_stmt* _tree_get_decl_stmt(tree_stmt* self);
static inline const struct _tree_decl_stmt* _tree_get_decl_cstmt(const tree_stmt* self);

static inline tree_decl* tree_get_decl_stmt_entity(const tree_stmt* self);
static inline void tree_set_decl_stmt_entity(tree_stmt* self, tree_decl* d);

struct _tree_expr_stmt
{
        struct _tree_stmt_base _base;
        tree_expr* _root;
};

extern tree_stmt* tree_new_expr_stmt(tree_context* context, tree_xlocation loc, tree_expr* root);

static inline struct _tree_expr_stmt* _tree_get_expr_stmt(tree_stmt* self);
static inline const struct _tree_expr_stmt* _tree_get_expr_cstmt(const tree_stmt* self);

static inline tree_expr* tree_get_expr_stmt_root(const tree_stmt* self);
static inline void tree_set_expr_stmt_root(tree_stmt* self, tree_expr* expr);

struct _tree_if_stmt
{
        struct _tree_stmt_base _base;
        tree_expr* _condition;
        tree_stmt* _body;
        tree_stmt* _else;
};

extern tree_stmt* tree_new_if_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_);

static inline struct _tree_if_stmt* _tree_get_if_stmt(tree_stmt* self);
static inline const struct _tree_if_stmt* _tree_get_if_cstmt(const tree_stmt* self);

static inline tree_expr* tree_get_if_condition(const tree_stmt* self);
static inline tree_stmt* tree_get_if_body(const tree_stmt* self);
static inline tree_stmt* tree_get_if_else(const tree_stmt* self);

static inline void tree_set_if_condition(tree_stmt* self, tree_expr* expr);
static inline void tree_set_if_body(tree_stmt* self, tree_stmt* body);
static inline void tree_set_if_else(tree_stmt* self, tree_stmt* else_);

struct _tree_switch_stmt
{
        struct _tree_stmt_base _base;
        tree_stmt* _body;
        tree_expr* _expr;
};

extern tree_stmt* tree_new_switch_stmt(tree_context* context,
        tree_xlocation loc,
        tree_stmt* body,
        tree_expr* expr);

static inline struct _tree_switch_stmt* _tree_get_switch_stmt(tree_stmt* self);
static inline const struct _tree_switch_stmt* _tree_get_switch_cstmt(const tree_stmt* self);

static inline tree_expr* tree_get_switch_expr(const tree_stmt* self);
static inline tree_stmt* tree_get_switch_body(const tree_stmt* self);

static inline void tree_set_switch_expr(tree_stmt* self, tree_expr* expr);
static inline void tree_set_switch_body(tree_stmt* self, tree_stmt* body);

struct _tree_while_stmt
{
        struct _tree_stmt_base _base;
        tree_expr* _condition;
        tree_stmt* _body;
};

extern tree_stmt* tree_new_while_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* condition, tree_stmt* body);

static inline struct _tree_while_stmt* _tree_get_while_stmt(tree_stmt* self);
static inline const struct _tree_while_stmt* _tree_get_while_cstmt(const tree_stmt* self);

static inline tree_expr* tree_get_while_condition(const tree_stmt* self);
static inline tree_stmt* tree_get_while_body(const tree_stmt* self);

static inline void tree_set_while_condition(tree_stmt* self, tree_expr* expr);
static inline void tree_set_while_body(tree_stmt* self, tree_stmt* body);

struct _tree_do_while_stmt
{
        struct _tree_stmt_base _base;
        tree_expr* _condition;
        tree_stmt* _body;
};

extern tree_stmt* tree_new_do_while_stmt(
        tree_context* context, tree_xlocation loc, tree_expr* condition, tree_stmt* body);

static inline struct _tree_do_while_stmt* _tree_get_do_while_stmt(tree_stmt* self);
static inline const struct _tree_do_while_stmt* _tree_get_do_while_cstmt(const tree_stmt* self);

static inline tree_expr* tree_get_do_while_condition(const tree_stmt* self);
static inline tree_stmt* tree_get_do_while_body(const tree_stmt* self);

static inline void tree_set_do_while_condition(tree_stmt* self, tree_expr* expr);
static inline void tree_set_do_while_body(tree_stmt* self, tree_stmt* body);

struct _tree_for_stmt
{
        struct _tree_stmt_base _base;
        tree_stmt* _init;
        tree_expr* _condition;
        tree_expr* _step;
        tree_stmt* _body;
};

extern tree_stmt* tree_new_for_stmt(
        tree_context* context,
        tree_xlocation loc,
        tree_stmt* init,
        tree_expr* condition,
        tree_expr* step,
        tree_stmt* body);

static inline struct _tree_for_stmt* _tree_get_for_stmt(tree_stmt* self);
static inline const struct _tree_for_stmt* _tree_get_for_cstmt(const tree_stmt* self);

static inline tree_stmt* tree_get_for_init(const tree_stmt* self);
static inline tree_expr* tree_get_for_condition(const tree_stmt* self);
static inline tree_expr* tree_get_for_step(const tree_stmt* self);
static inline tree_stmt* tree_get_for_body(const tree_stmt* self);

static inline void tree_set_for_init(tree_stmt* self, tree_stmt* init);
static inline void tree_set_for_condition(tree_stmt* self, tree_expr* condition);
static inline void tree_set_for_step(tree_stmt* self, tree_expr* step);
static inline void tree_set_for_body(tree_stmt* self, tree_stmt* body);


struct _tree_goto_stmt
{
        struct _tree_stmt_base _base;
        tree_decl* _label;
};

extern tree_stmt* tree_new_goto_stmt(tree_context* context, tree_xlocation loc, tree_decl* label);

static inline struct _tree_goto_stmt* _tree_get_goto_stmt(tree_stmt* self);
static inline const struct _tree_goto_stmt* _tree_get_goto_cstmt(const tree_stmt* self);

static inline tree_decl* tree_get_goto_label(const tree_stmt* self);
static inline void tree_set_goto_label(tree_stmt* self, tree_decl* label);


struct _tree_continue_stmt
{
        struct _tree_stmt_base _base;
        // todo
};

extern tree_stmt* tree_new_continue_stmt(tree_context* context, tree_xlocation loc);

struct _tree_break_stmt
{
        struct _tree_stmt_base _base;
        // todo
};

extern tree_stmt* tree_new_break_stmt(tree_context* context, tree_xlocation loc);

struct _tree_return_stmt
{
        struct _tree_stmt_base _base;
        tree_expr* _value;
        // todo
};

extern tree_stmt* tree_new_return_stmt(tree_context* context, tree_xlocation loc, tree_expr* value);

static inline struct _tree_return_stmt* _tree_get_return_stmt(tree_stmt* self);
static inline const struct _tree_return_stmt* _tree_get_return_cstmt(const tree_stmt* self);

static inline tree_expr* tree_get_return_value(const tree_stmt* self);
static inline void tree_set_return_value(tree_stmt* self, tree_expr* expr);

typedef struct _tree_stmt
{
        union
        {
                struct _tree_stmt_base _base;
                struct _tree_labeled_stmt _label;
                struct _tree_case_stmt _case;
                struct _tree_default_stmt _default;
                struct _tree_compound_stmt _compound;
                struct _tree_expr_stmt _expr;
                struct _tree_if_stmt _if;
                struct _tree_switch_stmt _switch;
                struct _tree_while_stmt _while;
                struct _tree_do_while_stmt _do_while;
                struct _tree_for_stmt _for;
                struct _tree_goto_stmt _goto;
                struct _tree_continue_stmt _continue;
                struct _tree_break_stmt _break;
                struct _tree_return_stmt _return;
                struct _tree_decl_stmt _decl;
        };
} tree_stmt;

static inline tree_scope_flags tree_get_scope_flags(const tree_scope* self)
{
        return self->_flags;
}

static inline tree_decl_scope* tree_get_scope_decls(tree_scope* self)
{
        return &self->_decls;
}

static inline const tree_decl_scope* tree_get_scope_cdecls(const tree_scope* self)
{
        return &self->_decls;
}

static inline tree_stmt* tree_get_scope_begin(const tree_scope* self)
{
        return (tree_stmt*)list_begin(&self->_stmts);
}

static inline const tree_stmt* tree_get_scope_end(const tree_scope* self)
{
        return (const tree_stmt*)list_cend(&self->_stmts);
}

static inline tree_scope* tree_get_scope_parent(const tree_scope* self)
{
        return self->_parent;
}

static inline void tree_set_scope_flags(tree_scope* self, tree_scope_flags f)
{
        self->_flags = f;
}

static inline void tree_add_scope_flags(tree_scope* self, tree_scope_flags f)
{
        self->_flags |= f;
}

#define TREE_ASSERT_STMT(S) S_ASSERT(S)

static inline struct _tree_stmt_base* _tree_get_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self);
        return (struct _tree_stmt_base*)self;
}

static inline const struct _tree_stmt_base* _tree_get_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self);
        return (const struct _tree_stmt_base*)self;
}

static inline tree_stmt_kind tree_get_stmt_kind(const tree_stmt* self)
{
        return _tree_get_cstmt(self)->_kind;
}

static inline tree_xlocation tree_get_stmt_loc(const tree_stmt* self)
{
        return _tree_get_cstmt(self)->_loc;
}

static inline tree_stmt* tree_get_next_stmt(const tree_stmt* self)
{
        return (tree_stmt*)list_node_next(&_tree_get_cstmt(self)->_node);
}

static inline bool tree_stmt_is(const tree_stmt* self, tree_stmt_kind k)
{
        return tree_get_stmt_kind(self) == k;
}

static inline void tree_set_stmt_kind(tree_stmt* self, tree_stmt_kind k)
{
        _tree_get_stmt(self)->_kind = k;
}

static inline void tree_set_stmt_loc(tree_stmt* self, tree_xlocation l)
{
        _tree_get_stmt(self)->_loc = l;
}

#undef TREE_ASSERT_STMT
#define TREE_ASSERT_STMT(S, K) S_ASSERT((S) && tree_get_stmt_kind(S) == (K))

static inline struct _tree_labeled_stmt* _tree_get_label_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_LABELED);
        return (struct _tree_labeled_stmt*)self;
}

static inline const struct _tree_labeled_stmt* _tree_get_label_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_LABELED);
        return (const struct _tree_labeled_stmt*)self;
}

static inline tree_decl* tree_get_label_stmt_decl(const tree_stmt* self)
{
        return _tree_get_label_cstmt(self)->_label;
}

static inline void tree_set_label_stmt_decl(tree_stmt* self, tree_decl* label)
{
        _tree_get_label_stmt(self)->_label = label;
}

static inline struct _tree_case_stmt* _tree_get_case(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_CASE);
        return (struct _tree_case_stmt*)self;
}

static inline const struct _tree_case_stmt* _tree_get_ccase(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_CASE);
        return (const struct _tree_case_stmt*)self;
}

static inline tree_expr* tree_get_case_value(const tree_stmt* self)
{
        return _tree_get_ccase(self)->_value;
}

static inline tree_stmt* tree_get_case_body(const tree_stmt* self)
{
        return _tree_get_ccase(self)->_body;
}

static inline void tree_set_case_value(tree_stmt* self, tree_expr* value)
{
        _tree_get_case(self)->_value = value;
}

static inline void tree_set_case_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_case(self)->_body = body;
}

static inline struct _tree_default_stmt* _tree_get_default(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_DEFAULT);
        return (struct _tree_default_stmt*)self;
}

static inline const struct _tree_default_stmt* _tree_get_cdefault(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_DEFAULT);
        return (const struct _tree_default_stmt*)self;
}

static inline tree_stmt* tree_get_default_body(const tree_stmt* self)
{
        return _tree_get_cdefault(self)->_body;
}

static inline void tree_set_default_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_default(self)->_body = body;
}

static inline struct _tree_compound_stmt* _tree_get_compound(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_COMPOUND);
        return (struct _tree_compound_stmt*)self;
}

static inline const struct _tree_compound_stmt* _tree_get_ccompound(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_COMPOUND);
        return (const struct _tree_compound_stmt*)self;
}

static inline tree_scope* tree_get_compound_scope(tree_stmt* self)
{
        return &_tree_get_compound(self)->_scope;
}

static inline const tree_scope* tree_get_compound_cscope(const tree_stmt* self)
{
        return &_tree_get_ccompound(self)->_scope;
}

static inline struct _tree_decl_stmt* _tree_get_decl_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_DECL);
        return (struct _tree_decl_stmt*)self;
}

static inline const struct _tree_decl_stmt* _tree_get_decl_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_DECL);
        return (const struct _tree_decl_stmt*)self;
}

static inline tree_decl* tree_get_decl_stmt_entity(const tree_stmt* self)
{
        return _tree_get_decl_cstmt(self)->_entity;
}

static inline void tree_set_decl_stmt_entity(tree_stmt* self, tree_decl* d)
{
        _tree_get_decl_stmt(self)->_entity = d;
}

static inline struct _tree_expr_stmt* _tree_get_expr_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_expr);
        return (struct _tree_expr_stmt*)self;
}

static inline const struct _tree_expr_stmt* _tree_get_expr_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_expr);
        return (const struct _tree_expr_stmt*)self;
}

static inline tree_expr* tree_get_expr_stmt_root(const tree_stmt* self)
{
        return _tree_get_expr_cstmt(self)->_root;
}

static inline void tree_set_expr_stmt_root(tree_stmt* self, tree_expr* expr)
{
        _tree_get_expr_stmt(self)->_root = expr;
}

static inline struct _tree_if_stmt* _tree_get_if_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_IF);
        return (struct _tree_if_stmt*)self;
}

static inline const struct _tree_if_stmt* _tree_get_if_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_IF);
        return (const struct _tree_if_stmt*)self;
}

static inline tree_expr* tree_get_if_condition(const tree_stmt* self)
{
        return _tree_get_if_cstmt(self)->_condition;
}

static inline tree_stmt* tree_get_if_body(const tree_stmt* self)
{
        return _tree_get_if_cstmt(self)->_body;
}

static inline tree_stmt* tree_get_if_else(const tree_stmt* self)
{
        return _tree_get_if_cstmt(self)->_else;
}

static inline void tree_set_if_condition(tree_stmt* self, tree_expr* expr)
{
        _tree_get_if_stmt(self)->_condition = expr;
}

static inline void tree_set_if_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_if_stmt(self)->_body = body;
}

static inline void tree_set_if_else(tree_stmt* self, tree_stmt* else_)
{
        _tree_get_if_stmt(self)->_else = else_;
}

static inline struct _tree_switch_stmt* _tree_get_switch_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_SWITCH);
        return (struct _tree_switch_stmt*)self;
}

static inline const struct _tree_switch_stmt* _tree_get_switch_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_SWITCH);
        return (const struct _tree_switch_stmt*)self;
}

static inline tree_expr* tree_get_switch_expr(const tree_stmt* self)
{
        return _tree_get_switch_cstmt(self)->_expr;
}

static inline tree_stmt* tree_get_switch_body(const tree_stmt* self)
{
        return _tree_get_switch_cstmt(self)->_body;
}

static inline void tree_set_switch_expr(tree_stmt* self, tree_expr* expr)
{
        _tree_get_switch_stmt(self)->_expr = expr;
}

static inline void tree_set_switch_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_switch_stmt(self)->_body = body;
}

static inline struct _tree_while_stmt* _tree_get_while_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_WHILE);
        return (struct _tree_while_stmt*)self;
}

static inline const struct _tree_while_stmt* _tree_get_while_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_WHILE);
        return (const struct _tree_while_stmt*)self;
}

static inline tree_expr* tree_get_while_condition(const tree_stmt* self)
{
        return _tree_get_while_cstmt(self)->_condition;
}

static inline tree_stmt* tree_get_while_body(const tree_stmt* self)
{
        return _tree_get_while_cstmt(self)->_body;
}

static inline void tree_set_while_condition(tree_stmt* self, tree_expr* expr)
{
        _tree_get_while_stmt(self)->_condition = expr;
}

static inline void tree_set_while_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_while_stmt(self)->_body = body;
}

static inline struct _tree_do_while_stmt* _tree_get_do_while_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_DO_WHILE);
        return (struct _tree_do_while_stmt*)self;
}

static inline const struct _tree_do_while_stmt* _tree_get_do_while_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_DO_WHILE);
        return (const struct _tree_do_while_stmt*)self;
}

static inline tree_expr* tree_get_do_while_condition(const tree_stmt* self)
{
        return _tree_get_do_while_cstmt(self)->_condition;
}

static inline tree_stmt* tree_get_do_while_body(const tree_stmt* self)
{
        return _tree_get_do_while_cstmt(self)->_body;
}

static inline void tree_set_do_while_condition(tree_stmt* self, tree_expr* expr)
{
        _tree_get_do_while_stmt(self)->_condition = expr;
}

static inline void tree_set_do_while_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_do_while_stmt(self)->_body = body;
}

static inline struct _tree_for_stmt* _tree_get_for_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_FOR);
        return (struct _tree_for_stmt*)self;
}

static inline const struct _tree_for_stmt* _tree_get_for_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_FOR);
        return (const struct _tree_for_stmt*)self;
}

static inline tree_stmt* tree_get_for_init(const tree_stmt* self)
{
        return _tree_get_for_cstmt(self)->_init;
}

static inline tree_expr* tree_get_for_condition(const tree_stmt* self)
{
        return _tree_get_for_cstmt(self)->_condition;
}

static inline tree_expr* tree_get_for_step(const tree_stmt* self)
{
        return _tree_get_for_cstmt(self)->_step;
}

static inline tree_stmt* tree_get_for_body(const tree_stmt* self)
{
        return _tree_get_for_cstmt(self)->_body;
}

static inline void tree_set_for_init(tree_stmt* self, tree_stmt* init)
{
        _tree_get_for_stmt(self)->_init = init;
}

static inline void tree_set_for_condition(tree_stmt* self, tree_expr* condition)
{
        _tree_get_for_stmt(self)->_condition = condition;
}

static inline void tree_set_for_step(tree_stmt* self, tree_expr* step)
{
        _tree_get_for_stmt(self)->_step = step;
}

static inline void tree_set_for_body(tree_stmt* self, tree_stmt* body)
{
        _tree_get_for_stmt(self)->_body = body;
}

static inline struct _tree_goto_stmt* _tree_get_goto_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_GOTO);
        return (struct _tree_goto_stmt*)self;
}

static inline const struct _tree_goto_stmt* _tree_get_goto_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_GOTO);
        return (const struct _tree_goto_stmt*)self;
}

static inline tree_decl* tree_get_goto_label(const tree_stmt* self)
{
        return _tree_get_goto_cstmt(self)->_label;
}

static inline void tree_set_goto_label(tree_stmt* self, tree_decl* label)
{
        _tree_get_goto_stmt(self)->_label = label;
}

static inline struct _tree_return_stmt* _tree_get_return_stmt(tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_RETURN);
        return (struct _tree_return_stmt*)self;
}

static inline const struct _tree_return_stmt* _tree_get_return_cstmt(const tree_stmt* self)
{
        TREE_ASSERT_STMT(self, TSK_RETURN);
        return (const struct _tree_return_stmt*)self;
}

static inline tree_expr* tree_get_return_value(const tree_stmt* self)
{
        return _tree_get_return_cstmt(self)->_value;
}

static inline void tree_set_return_value(tree_stmt* self, tree_expr* expr)
{
        _tree_get_return_stmt(self)->_value = expr;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_STMT_H
