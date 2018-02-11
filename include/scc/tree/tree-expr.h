#ifndef TREE_EXPR_H
#define TREE_EXPR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-common.h"
#include "tree-type.h" // TTQ_CONST

typedef struct _tree_expr tree_expr;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;
typedef struct _tree_designator tree_designator;

typedef enum
{
        TEK_UNKNOWN,
        TEK_BINARY,
        TEK_UNARY,
        TEK_CALL,
        TEK_SUBSCRIPT,
        TEK_CONDITIONAL,
        TEK_INTEGER_LITERAL,
        TEK_CHARACTER_LITERAL,
        TEK_FLOATING_LITERAL,
        TEK_STRING_LITERAL,
        TEK_DECL, 
        TEK_MEMBER,
        TEK_CAST,
        TEK_SIZEOF,
        TEK_PAREN,
        TEK_DESIGNATION,
        TEK_INIT_LIST,
        TEK_IMPL_INIT,
        TEK_SIZE,
} tree_expr_kind;

#define TREE_ASSERT_EXPR_KIND(K) S_ASSERT((K) > TEK_UNKNOWN && (K) < TEK_SIZE)

typedef enum
{
        TVK_LVALUE,
        TVK_RVALUE,
} tree_value_kind;

struct _tree_expr_base
{
        tree_expr_kind kind;
        tree_value_kind value_kind;
        tree_type* type;
        tree_location loc;
};

typedef enum
{
        TBK_UNKNOWN,
        TBK_MUL,
        TBK_DIV,
        TBK_MOD,
        TBK_ADD,
        TBK_SUB,
        TBK_SHL,
        TBK_SHR,
        TBK_LE,
        TBK_GR,
        TBK_LEQ,
        TBK_GEQ,
        TBK_EQ,
        TBK_NEQ,
        TBK_AND,
        TBK_XOR,
        TBK_OR,
        TBK_LOG_AND,
        TBK_LOG_OR,
        TBK_ASSIGN,
        TBK_ADD_ASSIGN,
        TBK_SUB_ASSIGN,
        TBK_MUL_ASSIGN,
        TBK_DIV_ASSIGN,
        TBK_MOD_ASSIGN,
        TBK_SHL_ASSIGN,
        TBK_SHR_ASSIGN,
        TBK_AND_ASSIGN,
        TBK_XOR_ASSIGN,
        TBK_OR_ASSIGN,
        TBK_COMMA,
        TBK_SIZE,
} tree_binop_kind;

#define TREE_ASSERT_BINOP_KIND(K) S_ASSERT((K) > TBK_UNKNOWN && (K) < TBK_SIZE)

struct _tree_binop
{
        struct _tree_expr_base base;
        tree_binop_kind kind;
        tree_expr* lhs;
        tree_expr* rhs;
};

typedef enum
{
        TUK_UNKNOWN,
        TUK_POST_INC,
        TUK_POST_DEC,
        TUK_PRE_INC,
        TUK_PRE_DEC,
        TUK_PLUS,
        TUK_MINUS,
        TUK_NOT,
        TUK_LOG_NOT,
        TUK_DEREFERENCE,
        TUK_ADDRESS,
        TUK_SIZE,
} tree_unop_kind;

#define TREE_ASSERT_UNOP_KIND(K) S_ASSERT((K) > TUK_UNKNOWN && (K) < TUK_SIZE)

struct _tree_unop
{
        struct _tree_expr_base base;
        tree_unop_kind kind;
        tree_expr* operand;
};

struct _tree_cast_expr
{
        struct _tree_expr_base base;
        tree_expr* operand;
        bool is_implicit;
};

struct _tree_call_expr
{
        struct _tree_expr_base base;
        tree_expr* lhs;
        tree_array args;
};

struct _tree_subscript_expr
{
        struct _tree_expr_base base;
        tree_expr* lhs;
        tree_expr* rhs;
};

struct _tree_conditional_expr
{
        struct _tree_expr_base base;
        tree_expr* condition;
        tree_expr* lhs;
        tree_expr* rhs;
};
struct _tree_integer_literal_expr
{
        struct _tree_expr_base base;
        suint64 value;
};

struct _tree_paren_expr
{
        struct _tree_expr_base base;
        tree_expr* expr;
};

typedef struct _tree_designator
{
        bool is_field;
        tree_location loc;

        union
        {
                tree_id field;
                tree_expr* index;
        };
} tree_designator;

struct _tree_designation
{
        struct _tree_expr_base base;
        tree_expr* init;
        tree_array designators;
};

struct _tree_init_list_expr
{
        struct _tree_expr_base base;
        tree_array exprs;
        bool has_trailing_comma;
};

struct _tree_impl_init_expr
{
        struct _tree_expr_base base;
        tree_expr* expr;
};

struct _tree_character_literal_expr
{
        struct _tree_expr_base base;
        int value;
};

struct _tree_floating_literal_expr
{
        struct _tree_expr_base base;
        float_value value;
};

struct _tree_string_literal_expr
{
        struct _tree_expr_base base;
        tree_id id;
};

struct _tree_decl_expr
{
        struct _tree_expr_base base;
        tree_decl* entity;
};

struct _tree_member_expr
{
        struct _tree_expr_base base;
        tree_expr* lhs;
        tree_decl* decl;
        bool is_arrow;
};

struct _tree_sizeof_expr
{
        struct _tree_expr_base base;
        bool contains_type;

        union
        {
                void* operand;
                tree_expr* expr;
                tree_type* type;
        };
};

typedef struct _tree_expr
{
        union
        {
                struct _tree_expr_base base;
                struct _tree_binop binop;
                struct _tree_unop unop;
                struct _tree_cast_expr cast;
                struct _tree_call_expr call;
                struct _tree_subscript_expr subscript;
                struct _tree_conditional_expr conditional;
                struct _tree_integer_literal_expr int_literal;
                struct _tree_character_literal_expr char_literal;
                struct _tree_floating_literal_expr floating_literal;
                struct _tree_string_literal_expr string_literal;
                struct _tree_decl_expr decl;
                struct _tree_member_expr member;
                struct _tree_sizeof_expr sizeof_expr;
                struct _tree_paren_expr paren;
                struct _tree_designation designation;
                struct _tree_init_list_expr init_list;
                struct _tree_impl_init_expr impl_init;
        };
} tree_expr;

extern bool tree_expr_is_literal(const tree_expr* self);
extern const tree_expr* tree_ignore_ccasts(const tree_expr* self);
extern tree_expr* tree_ignore_impl_casts(tree_expr* self);
extern const tree_expr* tree_ignore_impl_ccasts(const tree_expr* self);
extern tree_expr* tree_ignore_paren_exprs(tree_expr* self);
extern const tree_expr* tree_ignore_paren_cexps(const tree_expr* self);
// ignores implicit casts and parenthesises, if any
extern tree_expr* tree_desugar_expr(tree_expr* self);
extern const tree_expr* tree_desugar_cexpr(const tree_expr* self);

extern bool tree_expr_is_null_pointer_constant(tree_context* context, const tree_expr* expr);
extern bool tree_expr_designates_bitfield(const tree_expr* self);

extern tree_expr* tree_new_expr(
        tree_context* context,
        tree_expr_kind kind,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        ssize size);

static TREE_INLINE tree_expr_kind tree_get_expr_kind(const tree_expr* self)
{
        return self->base.kind;
}

static TREE_INLINE bool tree_expr_is(const tree_expr* self, tree_expr_kind kind)
{
        return tree_get_expr_kind(self) == kind;
}

static TREE_INLINE tree_value_kind tree_get_expr_value_kind(const tree_expr* self)
{
        return self->base.value_kind;
}

static TREE_INLINE bool tree_expr_is_lvalue(const tree_expr* self)
{
        return tree_get_expr_value_kind(self) == TVK_LVALUE;
}

static TREE_INLINE bool tree_expr_is_rvalue(const tree_expr* self)
{
        return tree_get_expr_value_kind(self) == TVK_RVALUE;
}

static TREE_INLINE tree_type* tree_get_expr_type(const tree_expr* self)
{
        return self->base.type;
}

static TREE_INLINE bool tree_expr_is_modifiable_lvalue(const tree_expr* self)
{
        return tree_expr_is_lvalue(self) 
                && !(tree_get_type_quals(self->base.type) & TTQ_CONST);
}

static TREE_INLINE tree_location tree_get_expr_loc(const tree_expr* self)
{
        return self->base.loc;
}

static TREE_INLINE void tree_set_expr_kind(tree_expr* self, tree_expr_kind kind)
{
        self->base.kind = kind;
}

static TREE_INLINE void tree_set_expr_value_kind(tree_expr* self, tree_value_kind value_kind)
{
        self->base.value_kind = value_kind;
}

static TREE_INLINE void tree_set_expr_type(tree_expr* self, tree_type* type)
{
        self->base.type = type;
}

static TREE_INLINE void tree_set_expr_loc(tree_expr* self, tree_location loc)
{
        self->base.loc = loc;
}

extern tree_expr* tree_new_binop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_binop_kind kind,
        tree_expr* lhs,
        tree_expr* rhs);

static TREE_INLINE tree_binop_kind tree_get_binop_kind(const tree_expr* self)
{
        return self->binop.kind;
}

static TREE_INLINE bool tree_binop_is(const tree_expr* self, tree_binop_kind kind)
{
        return tree_get_binop_kind(self) == kind;
}

static TREE_INLINE tree_expr* tree_get_binop_lhs(const tree_expr* self)
{
        return self->binop.lhs;
}

static TREE_INLINE tree_expr* tree_get_binop_rhs(const tree_expr* self)
{
        return self->binop.rhs;
}

static TREE_INLINE void tree_set_binop_kind(tree_expr* self, tree_binop_kind kind)
{
        self->binop.kind = kind;
}

static TREE_INLINE void tree_set_binop_lhs(tree_expr* self, tree_expr* lhs)
{
        self->binop.lhs = lhs;
}

static TREE_INLINE void tree_set_binop_rhs(tree_expr* self, tree_expr* rhs)
{
        self->binop.rhs = rhs;
}

extern tree_expr* tree_new_unop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_unop_kind kind,
        tree_expr* operand);

static TREE_INLINE tree_unop_kind tree_get_unop_kind(const tree_expr* self)
{
        return self->unop.kind;
}

static TREE_INLINE bool tree_unop_is(const tree_expr* self, tree_unop_kind kind)
{
        return tree_get_unop_kind(self) == kind;
}

static TREE_INLINE tree_expr* tree_get_unop_operand(const tree_expr* self)
{
        return self->unop.operand;
}

static TREE_INLINE void tree_set_unop_kind(tree_expr* self, tree_unop_kind kind)
{
        self->unop.kind = kind;
}

static TREE_INLINE void tree_set_unop_operand(tree_expr* self, tree_expr* operand)
{
        self->unop.operand = operand;
}

extern tree_expr* tree_new_cast_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_expr* expr,
        bool is_implicit);

static TREE_INLINE tree_expr* tree_get_cast_operand(const tree_expr* self)
{
        return self->cast.operand;
}

static TREE_INLINE bool tree_cast_is_implicit(const tree_expr* self)
{
        return self->cast.is_implicit;
}

static TREE_INLINE void tree_set_cast_operand(tree_expr* self, tree_expr* operand)
{
        self->cast.operand = operand;
}

static TREE_INLINE void tree_set_cast_implicit(tree_expr* self, bool is_implicit)
{
        self->cast.is_implicit = is_implicit;
}

extern tree_expr* tree_new_call_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs);

extern serrcode tree_add_call_arg(tree_expr* self, tree_context* context, tree_expr* arg);

static TREE_INLINE void tree_set_call_args(tree_expr* self, tree_array* args)
{
        self->call.args = *args;
}

static TREE_INLINE ssize tree_get_call_args_size(const tree_expr* self)
{
        return self->call.args.size;
}

static TREE_INLINE tree_expr* tree_get_call_lhs(const tree_expr* self)
{
        return self->call.lhs;
}

static TREE_INLINE tree_expr** tree_get_call_args_begin(const tree_expr* self)
{
        return (tree_expr**)self->call.args.data;
}

static TREE_INLINE tree_expr** tree_get_call_args_end(const tree_expr* self)
{
        return tree_get_call_args_begin(self) + tree_get_call_args_size(self);
}

static TREE_INLINE tree_expr* tree_get_call_arg(const tree_expr* self, ssize i)
{
        return tree_get_call_args_begin(self)[i];
}

static TREE_INLINE void tree_set_call_lhs(tree_expr* self, tree_expr* lhs)
{
        self->call.lhs = lhs;
}

#define TREE_FOREACH_CALL_ARG(PEXP, ITNAME) \
        for (tree_expr** ITNAME = tree_get_call_args_begin(PEXP); \
                ITNAME != tree_get_call_args_end(PEXP); ITNAME++)

extern tree_expr* tree_new_subscript_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs,
        tree_expr* rhs);

static TREE_INLINE tree_expr* tree_get_subscript_lhs(const tree_expr* self)
{
        return self->subscript.lhs;
}

static TREE_INLINE tree_expr* tree_get_subscript_rhs(const tree_expr* self)
{
        return self->subscript.rhs;
}

static TREE_INLINE void tree_set_subscript_lhs(tree_expr* self, tree_expr* lhs)
{
        self->subscript.lhs = lhs;
}

static TREE_INLINE void tree_set_subscript_rhs(tree_expr* self, tree_expr* rhs)
{
        self->subscript.rhs = rhs;
}

extern tree_expr* tree_new_conditional_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs);

static TREE_INLINE tree_expr* tree_get_conditional_lhs(const tree_expr* self)
{
        return self->conditional.lhs;
}

static TREE_INLINE tree_expr* tree_get_conditional_rhs(const tree_expr* self)
{
        return self->conditional.rhs;
}

static TREE_INLINE tree_expr* tree_get_conditional_condition(const tree_expr* self)
{
        return self->conditional.condition;
}

static TREE_INLINE void tree_set_conditional_lhs(tree_expr* self, tree_expr* lhs)
{
        self->conditional.lhs = lhs;
}

static TREE_INLINE void tree_set_conditional_rhs(tree_expr* self, tree_expr* rhs)
{
        self->conditional.rhs = rhs;
}

static TREE_INLINE void tree_set_conditional_condition(tree_expr* self, tree_expr* condition)
{
        self->conditional.condition = condition;
}

extern tree_expr* tree_new_integer_literal(
        tree_context* context, tree_type* type, tree_location loc, suint64 value);

static TREE_INLINE suint64 tree_get_integer_literal(const tree_expr* self)
{
        return self->int_literal.value;
}

static TREE_INLINE void tree_set_integer_literal(tree_expr* self, suint64 value)
{
        self->int_literal.value = value;
}

extern tree_expr* tree_new_character_literal(
        tree_context* context, tree_type* type, tree_location loc, int value);

static TREE_INLINE int tree_get_character_literal(const tree_expr* self)
{
        return self->char_literal.value;
}

static TREE_INLINE void tree_set_character_literal(tree_expr* self, int value)
{
        self->char_literal.value = value;
}

extern tree_expr* tree_new_floating_literal(
        tree_context* context,
        tree_type* type,
        tree_location loc,
        const float_value* value);

static TREE_INLINE const float_value* tree_get_floating_literal_cvalue(const tree_expr* self)
{
        return &self->floating_literal.value;
}

static TREE_INLINE void tree_set_floating_literal_value(tree_expr* self, const float_value* value)
{
        self->floating_literal.value = *value;
}

extern tree_expr* tree_new_string_literal(
        tree_context* context,
        tree_value_kind value,
        tree_type* type,
        tree_location loc,
        tree_id id);

static TREE_INLINE tree_id tree_get_string_literal(const tree_expr* self)
{
        return self->string_literal.id;
}

static TREE_INLINE void tree_set_string_literal(tree_expr* self, tree_id id)
{
        self->string_literal.id = id;
}

extern tree_expr* tree_new_decl_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_decl* decl);

static TREE_INLINE tree_decl* tree_get_decl_expr_entity(const tree_expr* self)
{
        return self->decl.entity;
}

static TREE_INLINE void tree_set_decl_expr_entity(tree_expr* self, tree_decl* decl)
{
        self->decl.entity = decl;
}

extern tree_expr* tree_new_member_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs,
        tree_decl* decl,
        bool is_arrow);

static TREE_INLINE tree_expr* tree_get_member_expr_lhs(const tree_expr* self)
{
        return self->member.lhs;
}

static TREE_INLINE tree_decl* tree_get_member_expr_decl(const tree_expr* self)
{
        return self->member.decl;
}

static TREE_INLINE bool tree_member_expr_is_arrow(const tree_expr* self)
{
        return self->member.is_arrow;
}

static TREE_INLINE void tree_set_member_expr_lhs(tree_expr* self, tree_expr* lhs)
{
        self->member.lhs = lhs;
}

static TREE_INLINE void tree_set_member_expr_decl(tree_expr* self, tree_decl* decl)
{
        self->member.decl = decl;
}

static TREE_INLINE void tree_set_member_expr_arrow(tree_expr* self, bool val)
{
        self->member.is_arrow = val;
}

extern tree_expr* tree_new_sizeof_expr(
        tree_context* context,
        tree_type* type,
        tree_location loc,
        void* operand,
        bool contains_type);

static TREE_INLINE bool tree_sizeof_contains_type(const tree_expr* self)
{
        return self->sizeof_expr.contains_type;
}

static TREE_INLINE tree_expr* tree_get_sizeof_expr(const tree_expr* self)
{
        return self->sizeof_expr.expr;
}

static TREE_INLINE tree_type* tree_get_sizeof_type(const tree_expr* self)
{
        return self->sizeof_expr.type;
}

static TREE_INLINE void tree_set_sizeof_expr(tree_expr* self, tree_expr* expr)
{
        self->sizeof_expr.expr = expr;
}

static TREE_INLINE void tree_set_sizeof_type(tree_expr* self, tree_type* type)
{
        self->sizeof_expr.type = type;
}

static TREE_INLINE void tree_set_sizeof_contains_type(tree_expr* self, bool val)
{
        self->sizeof_expr.contains_type = val;
}

extern tree_expr* tree_new_paren_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* expr);

static TREE_INLINE tree_expr* tree_get_paren_expr(const tree_expr* self)
{
        return self->paren.expr;
}

static TREE_INLINE void tree_set_paren_expr(tree_expr* self, tree_expr* expr)
{
        self->paren.expr = expr;
}

extern tree_designator* tree_new_field_designator(
        tree_context* context, tree_location loc, tree_id field);

extern tree_designator* tree_new_array_designator(
        tree_context* context, tree_location loc, tree_expr* index);

static TREE_INLINE tree_location tree_get_designator_loc(const tree_designator* self)
{
        return self->loc;
}

static TREE_INLINE bool tree_designator_is_field(const tree_designator* self)
{
        return self->is_field;
}

static TREE_INLINE tree_id tree_get_designator_field(const tree_designator* self)
{
        return self->field;
}

static TREE_INLINE tree_expr* tree_get_designator_index(const tree_designator* self)
{
        return self->index;
}

static TREE_INLINE void tree_set_designator_loc(tree_designator* self, tree_location loc)
{
        self->loc = loc;
}

static TREE_INLINE void tree_set_designator_member(tree_designator* self, tree_id field)
{
        self->field = field;
}

static TREE_INLINE void tree_set_designator_index(tree_designator* self, tree_expr* index)
{
        self->index = index;
}

extern tree_expr* tree_new_designation(tree_context* context, tree_expr* init);

extern serrcode tree_add_designation_designator(
        tree_expr* self, tree_context* context, tree_designator* d);

static TREE_INLINE tree_designator** tree_get_designation_designators_begin(const tree_expr* self)
{
        return (tree_designator**)self->designation.designators.data;
}

static TREE_INLINE tree_designator** tree_get_designation_designators_end(const tree_expr* self)
{
        return tree_get_designation_designators_begin(self) + self->designation.designators.size;
}

static TREE_INLINE tree_expr* tree_get_designation_init(const tree_expr* self)
{
        return self->designation.init;
}

static TREE_INLINE void tree_set_designation_init(tree_expr* self, tree_expr* init)
{
        self->designation.init = init;
}

#define TREE_FOREACH_DESIGNATION_DESIGNATOR(P, ITNAME, ENDNAME)\
        for (tree_designator** ITNAME = tree_get_designation_designators_begin(P),\
                **ENDNAME = tree_get_designation_designators_end(P);\
                ITNAME != ENDNAME; ITNAME++)

extern tree_expr* tree_new_init_list_expr(tree_context* context, tree_location loc);
extern serrcode tree_add_init_list_expr(tree_expr* self, tree_context* context, tree_expr* expr);

static TREE_INLINE tree_expr** tree_get_init_list_exprs_begin(const tree_expr* self)
{
        return (tree_expr**)self->init_list.exprs.data;
}

static TREE_INLINE tree_expr** tree_get_init_list_exprs_end(const tree_expr* self)
{
        return tree_get_init_list_exprs_begin(self) + self->init_list.exprs.size;
}

static TREE_INLINE bool tree_init_list_has_trailing_comma(const tree_expr* self)
{
        return self->init_list.has_trailing_comma;
}

static TREE_INLINE void tree_set_init_list_has_trailing_comma(tree_expr* self, bool val)
{
        self->init_list.has_trailing_comma = val;
}

#define TREE_FOREACH_INIT_LIST_EXPR(PLIST, ITNAME, ENDNAME)\
        for (tree_expr** ITNAME = tree_get_init_list_exprs_begin(PLIST),\
                **ENDNAME = tree_get_init_list_exprs_end(PLIST);\
                ITNAME != ENDNAME; ITNAME++)

extern tree_expr* tree_new_impl_init_expr(tree_context* context, tree_expr* expr);

static TREE_INLINE tree_expr* tree_get_impl_init_expr(const tree_expr* self)
{
        return self->impl_init.expr;
}

static TREE_INLINE void tree_set_impl_init_expr(tree_expr* self, tree_expr* expr)
{
        self->impl_init.expr = expr;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_EXPR_H
