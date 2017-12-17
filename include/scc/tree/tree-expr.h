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
typedef struct _tree_designation tree_designation;
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
        
        // var-decl, enumerator, unresolved
        TEK_DECL, 
        TEK_MEMBER,
        TEK_EXPLICIT_CAST,
        TEK_IMPLICIT_CAST,
        TEK_SIZEOF,
        TEK_PAREN,
        TEK_INIT,
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
        tree_expr_kind _kind;
        tree_value_kind _value_kind;
        tree_type* _type;
        tree_location _loc;
};

extern tree_expr* tree_new_expr(
        tree_context* context,
        tree_expr_kind kind,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        ssize size);

static inline struct _tree_expr_base* _tree_get_expr(tree_expr* self);
static inline const struct _tree_expr_base* _tree_get_cexp(const tree_expr* self);

static inline bool tree_expr_is(const tree_expr* self, tree_expr_kind k);
static inline tree_expr_kind tree_get_expr_kind(const tree_expr* self);
static inline tree_value_kind tree_get_expr_value_kind(const tree_expr* self);
static inline bool tree_expr_is_lvalue(const tree_expr* self);
static inline bool tree_expr_is_modifiable_lvalue(const tree_expr* self);
static inline tree_type* tree_get_expr_type(const tree_expr* self);
static inline tree_location tree_get_expr_loc(const tree_expr* self);

static inline void tree_set_expr_kind(tree_expr* self, tree_expr_kind k);
static inline void tree_set_expr_value_kind(tree_expr* self, tree_value_kind vk);
static inline void tree_set_expr_type(tree_expr* self, tree_type* t);
static inline void tree_set_expr_loc(tree_expr* self, tree_location loc);

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
        struct _tree_expr_base _base;
        tree_binop_kind _kind;
        tree_expr* _lhs;
        tree_expr* _rhs;
};

extern tree_expr* tree_new_binop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_binop_kind kind,
        tree_expr* lhs,
        tree_expr* rhs);

static inline struct _tree_binop* _tree_get_binop(tree_expr* self);
static inline const struct _tree_binop* _tree_get_cbinop(const tree_expr* self);

static inline bool tree_binop_is(const tree_expr* self, tree_binop_kind k);
static inline tree_binop_kind tree_get_binop_kind(const tree_expr* self);
static inline tree_expr* tree_get_binop_lhs(const tree_expr* self);
static inline tree_expr* tree_get_binop_rhs(const tree_expr* self);

static inline void tree_set_binop_kind(tree_expr* self, tree_binop_kind k);
static inline void tree_set_binop_lhs(tree_expr* self, tree_expr* lhs);
static inline void tree_set_binop_rhs(tree_expr* self, tree_expr* rhs);

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
        struct _tree_expr_base _base;
        tree_unop_kind _kind;
        tree_expr* _expr;
};

extern tree_expr* tree_new_unop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_unop_kind kind,
        tree_expr* expr);

static inline struct _tree_unop* _tree_get_unop(tree_expr* self);
static inline const struct _tree_unop* _tree_get_cunop(const tree_expr* self);

static inline bool tree_unop_is(const tree_expr* self, tree_unop_kind k);
static inline tree_unop_kind tree_get_unop_kind(const tree_expr* self);
static inline tree_expr* tree_get_unop_expr(const tree_expr* self);

static inline void tree_set_unop_kind(tree_expr* self, tree_unop_kind k);
static inline void tree_set_unop_expr(tree_expr* self, tree_expr* expr);

struct _tree_cast_expr
{
        struct _tree_expr_base _base;
        tree_expr* _expr;
};

extern tree_expr* tree_new_explicit_cast_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_expr* expr);

extern tree_expr* tree_new_implicit_cast_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_expr* expr);

static inline struct _tree_cast_expr* _tree_get_cast(tree_expr* self);
static inline const struct _tree_cast_expr* _tree_get_ccast(const tree_expr* self);

static inline tree_expr* tree_get_cast_expr(const tree_expr* self);
static inline void tree_set_cast_expr(tree_expr* self, tree_expr* expr);

struct _tree_call_expr
{
        struct _tree_expr_base _base;
        tree_expr* _lhs;
        dseq _args;
};

extern tree_expr* tree_new_call_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs);

extern void tree_set_call_args(tree_expr* self, dseq* args);
extern void tree_add_call_arg(tree_expr* self, tree_expr* arg);

static inline struct _tree_call_expr* _tree_get_call(tree_expr* self);
static inline const struct _tree_call_expr* _tree_get_ccall(const tree_expr* self);

static inline ssize tree_get_call_nargs(const tree_expr* self);
static inline tree_expr* tree_get_call_lhs(const tree_expr* self);
static inline tree_expr** tree_get_call_args_begin(const tree_expr* self);
static inline tree_expr** tree_get_call_args_end(const tree_expr* self);

static inline void tree_set_call_lhs(tree_expr* self, tree_expr* lhs);

#define TREE_FOREACH_CALL_ARG(PEXP, ITNAME) \
        for (tree_expr** ITNAME = tree_get_call_args_begin(PEXP); \
                ITNAME != tree_get_call_args_end(PEXP); ITNAME++)

struct _tree_subscript_expr
{
        struct _tree_expr_base _base;
        tree_expr* _lhs;
        tree_expr* _rhs;
};

extern tree_expr* tree_new_subscript_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs,
        tree_expr* rhs);

static inline struct _tree_subscript_expr* _tree_get_subscript(tree_expr* self);
static inline const struct _tree_subscript_expr* _tree_get_csubscript(const tree_expr* self);

static inline tree_expr* tree_get_subscript_lhs(const tree_expr* self);
static inline tree_expr* tree_get_subscript_rhs(const tree_expr* self);

static inline void tree_set_subscript_lhs(tree_expr* self, tree_expr* lhs);
static inline void tree_set_subscript_rhs(tree_expr* self, tree_expr* rhs);

struct _tree_conditional_expr
{
        struct _tree_expr_base _base;
        tree_expr* _condition;
        tree_expr* _lhs;
        tree_expr* _rhs;
};

extern tree_expr* tree_new_conditional_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs);

static inline struct _tree_conditional_expr* _tree_get_conditional(tree_expr* self);
static inline const struct _tree_conditional_expr* _tree_get_cconditional(const tree_expr* self);

static inline tree_expr* tree_get_conditional_lhs(const tree_expr* self);
static inline tree_expr* tree_get_conditional_rhs(const tree_expr* self);
static inline tree_expr* tree_get_conditional_condition(const tree_expr* self);

static inline void tree_set_conditional_lhs(tree_expr* self, tree_expr* lhs);
static inline void tree_set_conditional_rhs(tree_expr* self, tree_expr* rhs);
static inline void tree_set_conditional_condition(tree_expr* self, tree_expr* condition);

struct _tree_integer_literal_expr
{
        struct _tree_expr_base _base;
        suint64 _value;
};

extern tree_expr* tree_new_integer_literal(
        tree_context* context, tree_type* type, tree_location loc, suint64 value);

static inline struct _tree_integer_literal_expr* _tree_get_integer_literal(tree_expr* self);
static inline const struct _tree_integer_literal_expr* _tree_get_cinteger_literal(const tree_expr* self);

static inline suint64 tree_get_integer_literal(const tree_expr* self);
static inline void tree_set_integer_literal(tree_expr* self, suint64 value);

struct _tree_character_literal_expr
{
        struct _tree_expr_base _base;
        int _value;
};

extern tree_expr* tree_new_character_literal(
        tree_context* context, tree_type* type, tree_location loc, int value);

static inline struct _tree_character_literal_expr* _tree_get_character_literal(tree_expr* self);
static inline const struct _tree_character_literal_expr* _tree_get_ccharacter_literal(const tree_expr* self);

static inline int tree_get_character_literal(const tree_expr* self);
static inline void tree_set_character_literal(tree_expr* self, int value);

struct _tree_floating_literal_expr
{
        struct _tree_expr_base _base;
        float_value _value;
};

extern tree_expr* tree_new_floating_literal(
        tree_context* context,
        tree_type* type,
        tree_location loc,
        const float_value* value);

static inline struct _tree_floating_literal_expr*
_tree_get_floating_literal(tree_expr* self);

static inline const struct _tree_floating_literal_expr*
_tree_get_cfloating_literal(const tree_expr* self);

static inline const float_value* tree_get_floating_literal_cvalue(const tree_expr* self);
static inline void tree_set_floating_literal_value(tree_expr* self, const float_value* value);

struct _tree_string_literal_expr
{
        struct _tree_expr_base _base;
        tree_id _ref;
};

extern tree_expr* tree_new_string_literal(
        tree_context* context,
        tree_value_kind value,
        tree_type* type,
        tree_location loc,
        tree_id ref);

static inline struct _tree_string_literal_expr* _tree_get_string_literal(tree_expr* self);
static inline const struct _tree_string_literal_expr* _tree_get_cstring_literal(const tree_expr* self);

static inline tree_id tree_get_string_literal(const tree_expr* self);
static inline void tree_set_string_literal(tree_expr* self, tree_id ref);

struct _tree_decl_expr
{
        struct _tree_expr_base _base;
        tree_decl* _entity;
};

extern tree_expr* tree_new_decl_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_decl* decl);

static inline struct _tree_decl_expr* _tree_get_decl_expr(tree_expr* self);
static inline const struct _tree_decl_expr* _tree_get_decl_cexp(const tree_expr* self);

static inline tree_decl* tree_get_decl_expr_entity(const tree_expr* self);
static inline void tree_set_decl_expr_entity(tree_expr* self, tree_decl* decl);

struct _tree_member_expr
{
        struct _tree_expr_base _base;
        tree_expr* _lhs;
        tree_decl* _decl;
        bool _is_arrow;
};

extern tree_expr* tree_new_member_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs,
        tree_decl* decl,
        bool is_arrow);

static inline struct _tree_member_expr* _tree_get_member_expr(tree_expr* self);
static inline const struct _tree_member_expr* _tree_get_member_cexp(const tree_expr* self);

static inline tree_expr* tree_get_member_expr_lhs(const tree_expr* self);
static inline tree_decl* tree_get_member_expr_decl(const tree_expr* self);
static inline bool tree_member_expr_is_arrow(const tree_expr* self);

static inline void tree_set_member_expr_lhs(tree_expr* self, tree_expr* lhs);
static inline void tree_set_member_expr_decl(tree_expr* self, tree_decl* decl);
static inline void tree_set_member_expr_arrow(tree_expr* self, bool val);

struct _tree_sizeof_expr
{
        struct _tree_expr_base _base;
        bool _is_unary;

        union
        {
                tree_expr* _expr;
                tree_type* _type;
        };
};

extern tree_expr* tree_new_sizeof_expr(
        tree_context* context, tree_type* type, tree_location loc, void* rhs, bool is_unary);

static inline struct _tree_sizeof_expr* _tree_get_sizeof(tree_expr* self);
static inline const struct _tree_sizeof_expr* _tree_get_csizeof(const tree_expr* self);

static inline bool tree_sizeof_is_unary(const tree_expr* self);
static inline tree_expr* tree_get_sizeof_expr(const tree_expr* self);
static inline tree_type* tree_get_sizeof_type(const tree_expr* self);

static inline void tree_set_sizeof_expr(tree_expr* self, tree_expr* expr);
static inline void tree_set_sizeof_type(tree_expr* self, tree_type* type);
static inline void tree_set_sizeof_unary(tree_expr* self, bool val);

struct _tree_paren_expr
{
        struct _tree_expr_base _base;
        tree_expr* _expr;
}; 

extern tree_expr* tree_new_paren_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* expr);

static inline struct _tree_paren_expr* _tree_get_paren_expr(tree_expr* self);
static inline const struct _tree_paren_expr* _tree_get_paren_cexp(const tree_expr* self);

static inline tree_expr* tree_get_paren_expr(const tree_expr* self);
static inline void tree_set_paren_expr(tree_expr* self, tree_expr* expr);

struct _tree_init_expr
{
        struct _tree_expr_base _base;
        list_head _designations;
}; 

extern tree_expr* tree_new_init_expr(tree_context* context, tree_location loc);
extern void tree_add_init_designation(tree_expr* self, tree_designation* d);

static inline struct _tree_init_expr* _tree_get_init_expr(tree_expr* self);
static inline const struct _tree_init_expr* _tree_get_init_cexp(const tree_expr* self); 

static inline tree_designation* tree_get_init_begin(const tree_expr* self);
static inline const tree_designation* tree_get_init_end(const tree_expr* self);

#define TREE_FOREACH_DESIGNATION(PEXP, ITNAME) \
        for (tree_designation* ITNAME = tree_get_init_begin(PEXP); \
                ITNAME != tree_get_init_end(PEXP); \
                ITNAME = tree_get_next_designation(ITNAME))

typedef enum
{
        TDK_DES_MEMBER,
        TDK_DES_ARRAY,
} tree_designator_kind;

struct _tree_designator_base
{
        list_node _node;
        tree_designator_kind _kind;
};

static inline struct _tree_designator_base* _tree_get_designator(tree_designator* self);
static inline const struct _tree_designator_base* _tree_get_cdesignator(const tree_designator* self);

static inline tree_designator* tree_get_next_designator(const tree_designator* self);
static inline tree_designator_kind tree_get_designator_kind(const tree_designator* self);
static inline bool tree_designator_is(const tree_designator* self, tree_designator_kind k);

static inline void tree_set_designator_kind(tree_designator* self, tree_designator_kind k);

struct _tree_array_designator
{
        struct _tree_designator_base _base;
        tree_type* _eltype;
        tree_expr* _index;
};

extern tree_designator* tree_new_array_designator(
        tree_context* context, tree_type* eltype, tree_expr* index);

static inline struct _tree_array_designator* _tree_get_array_designator(tree_designator* self);
static inline const struct _tree_array_designator* _tree_get_array_cdesignator(const tree_designator* self);

static inline tree_type* tree_get_array_designator_type(const tree_designator* self);
static inline tree_expr* tree_get_array_designator_index(const tree_designator* self);

static inline void tree_set_array_designator_type(tree_designator* self, tree_type* t);
static inline void tree_set_array_designator_index(tree_designator* self, tree_expr* i);

struct _tree_member_designator
{
        struct _tree_designator_base _base;
        tree_decl* _member;
};

extern tree_designator* tree_new_member_designator(tree_context* context, tree_decl* member);

static inline struct _tree_member_designator* _tree_get_member_designator(tree_designator* self);
static inline const struct _tree_member_designator* _tree_get_member_cdesignator(const tree_designator* self);

static inline tree_decl* tree_get_member_designator_decl(const tree_designator* self);
static inline void tree_set_member_designator_decl(tree_designator* self, tree_decl* d);

typedef struct _tree_designator
{
        union
        {
                struct _tree_array_designator _array;
                struct _tree_member_designator _member;
        };
} tree_designator;

typedef struct _tree_designation
{
        list_node _node;
        list_head _designators;
        tree_expr* _initializer;
} tree_designation;

extern tree_designation* tree_new_designation(tree_context* context, tree_expr* initializer);

extern void tree_add_designation_designator(tree_designation* self, tree_designator* d);

static inline tree_designation* tree_get_next_designation(const tree_designation* self);
static inline tree_designator* tree_get_designation_begin(const tree_designation* self);
static inline tree_designator* tree_get_designation_last(const tree_designation* self);
static inline const tree_designator* tree_get_designation_end(const tree_designation* self);
static inline tree_expr* tree_get_designation_initializer(const tree_designation* self);

static inline void tree_set_designation_initializer(tree_designation* self, tree_expr* initializer);

#define TREE_FOREACH_DESIGNATOR(PEXP, ITNAME) \
        for (tree_designator* ITNAME = tree_get_designation_begin(PEXP); \
                ITNAME != tree_get_designation_end(PEXP); \
                ITNAME = tree_get_next_designator(ITNAME))

struct _tree_impl_init_expr
{
        struct _tree_expr_base _base;
        tree_expr* _init;
};

extern tree_expr* tree_new_impl_init_expr(tree_context* context, tree_expr* init);

static inline struct _tree_impl_init_expr* _tree_get_impl_init_expr(tree_expr* self);
static inline const struct _tree_impl_init_expr* _tree_get_impl_init_cexp(const tree_expr* self);

static inline tree_expr* tree_get_impl_init_expr(const tree_expr* self);
static inline void tree_set_impl_init_expr(tree_expr* self, tree_expr* init);

typedef struct _tree_expr
{
        union
        {
                struct _tree_binop _binary;
                struct _tree_unop _unary;
                struct _tree_cast_expr _cast;
                struct _tree_call_expr _call;
                struct _tree_subscript_expr _subscript;
                struct _tree_conditional_expr _conditional;
                struct _tree_string_literal_expr _string;
                struct _tree_integer_literal_expr _int;
                struct _tree_floating_literal_expr _float;
                struct _tree_character_literal_expr _char;
                struct _tree_decl_expr _decl;
                struct _tree_member_expr _member;
                struct _tree_sizeof_expr _sizeof;
                struct _tree_paren_expr _paren;
                struct _tree_init_expr _init;
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
extern const tree_expr* tree_desugar_cexp(const tree_expr* self);

extern bool tree_expr_is_null_pointer_constant(const tree_expr* self);
extern bool tree_expr_designates_bitfield(const tree_expr* self);

#define TREE_ASSERT_EXPR(E) S_ASSERT(E)

static inline struct _tree_expr_base* _tree_get_expr(tree_expr* self)
{
        TREE_ASSERT_EXPR(self);
        return (struct _tree_expr_base*)self;
}

static inline const struct _tree_expr_base* _tree_get_cexp(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self);
        return (const struct _tree_expr_base*)self;
}

static inline tree_expr_kind tree_get_expr_kind(const tree_expr* self)
{
        return _tree_get_cexp(self)->_kind;
}

static inline tree_value_kind tree_get_expr_value_kind(const tree_expr* self)
{
        return _tree_get_cexp(self)->_value_kind;
}

static inline bool tree_expr_is_lvalue(const tree_expr* self)
{
        return tree_get_expr_value_kind(self) == TVK_LVALUE;
}

static inline bool tree_expr_is_modifiable_lvalue(const tree_expr* self)
{
        return tree_expr_is_lvalue(self)
                && !(tree_get_type_quals(tree_get_expr_type(self)) & TTQ_CONST);
}

static inline tree_type* tree_get_expr_type(const tree_expr* self)
{
        return _tree_get_cexp(self)->_type;
}

static inline tree_location tree_get_expr_loc(const tree_expr* self)
{
        return _tree_get_cexp(self)->_loc;
}

static inline void tree_set_expr_kind(tree_expr* self, tree_expr_kind k)
{
        _tree_get_expr(self)->_kind = k;
}

static inline void tree_set_expr_value_kind(tree_expr* self, tree_value_kind vk)
{
        _tree_get_expr(self)->_value_kind = vk;
}

static inline void tree_set_expr_type(tree_expr* self, tree_type* t)
{
        _tree_get_expr(self)->_type = t;
}

static inline void tree_set_expr_loc(tree_expr* self, tree_location loc)
{
        _tree_get_expr(self)->_loc = loc;
}

#undef TREE_ASSERT_EXPR
#define TREE_ASSERT_EXPR(E, K) S_ASSERT((E) && tree_get_expr_kind(E) == (K))

static inline struct _tree_binop* _tree_get_binop(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_BINARY);
        return (struct _tree_binop*)self;
}

static inline const struct _tree_binop* _tree_get_cbinop(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_BINARY);
        return (const struct _tree_binop*)self;
}

static inline bool tree_binop_is(const tree_expr* self, tree_binop_kind k)
{
        return tree_get_binop_kind(self) == k;
}

static inline tree_binop_kind tree_get_binop_kind(const tree_expr* self)
{
        return _tree_get_cbinop(self)->_kind;
}

static inline tree_expr* tree_get_binop_lhs(const tree_expr* self)
{
        return _tree_get_cbinop(self)->_lhs;
}

static inline tree_expr* tree_get_binop_rhs(const tree_expr* self)
{
        return _tree_get_cbinop(self)->_rhs;
}

static inline void tree_set_binop_kind(tree_expr* self, tree_binop_kind k)
{
        _tree_get_binop(self)->_kind = k;
}

static inline void tree_set_binop_lhs(tree_expr* self, tree_expr* lhs)
{
        _tree_get_binop(self)->_lhs = lhs;
}

static inline void tree_set_binop_rhs(tree_expr* self, tree_expr* rhs)
{
        _tree_get_binop(self)->_rhs = rhs;
}

static inline struct _tree_unop* _tree_get_unop(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_UNARY);
        return (struct _tree_unop*)self;
}

static inline const struct _tree_unop* _tree_get_cunop(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_UNARY);
        return (const struct _tree_unop*)self;
}

static inline bool tree_unop_is(const tree_expr* self, tree_unop_kind k)
{
        return tree_get_unop_kind(self) == k;
}

static inline tree_unop_kind tree_get_unop_kind(const tree_expr* self)
{
        return _tree_get_cunop(self)->_kind;
}

static inline tree_expr* tree_get_unop_expr(const tree_expr* self)
{
        return _tree_get_cunop(self)->_expr;
}

static inline void tree_set_unop_kind(tree_expr* self, tree_unop_kind k)
{
        _tree_get_unop(self)->_kind = k;
}

static inline void tree_set_unop_expr(tree_expr* self, tree_expr* expr)
{
        _tree_get_unop(self)->_expr = expr;
}

#define TREE_ASSERT_CAST(P) \
        S_ASSERT((P) && tree_get_expr_kind(P) == TEK_EXPLICIT_CAST \
                     || tree_get_expr_kind(P) == TEK_IMPLICIT_CAST)

static inline struct _tree_cast_expr* _tree_get_cast(tree_expr* self)
{
        TREE_ASSERT_CAST(self);
        return (struct _tree_cast_expr*)self;
}

static inline const struct _tree_cast_expr* _tree_get_ccast(const tree_expr* self)
{
        TREE_ASSERT_CAST(self);
        return (const struct _tree_cast_expr*)self;
}

static inline tree_expr* tree_get_cast_expr(const tree_expr* self)
{
        return _tree_get_ccast(self)->_expr;
}

static inline void tree_set_cast_expr(tree_expr* self, tree_expr* expr)
{
        _tree_get_cast(self)->_expr = expr;
}

static inline struct _tree_call_expr* _tree_get_call(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_CALL);
        return (struct _tree_call_expr*)self;
}

static inline const struct _tree_call_expr* _tree_get_ccall(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_CALL);
        return (const struct _tree_call_expr*)self;
}

static inline ssize tree_get_call_nargs(const tree_expr* self)
{
        return dseq_size(&_tree_get_ccall(self)->_args);
}

static inline tree_expr* tree_get_call_lhs(const tree_expr* self)
{
        return _tree_get_ccall(self)->_lhs;
}

static inline tree_expr** tree_get_call_args_begin(const tree_expr* self)
{
        return (tree_expr**)dseq_begin_ptr(&_tree_get_ccall(self)->_args);
}

static inline tree_expr** tree_get_call_args_end(const tree_expr* self)
{
        return (tree_expr**)dseq_end_ptr(&_tree_get_ccall(self)->_args);
}

static inline void tree_set_call_lhs(tree_expr* self, tree_expr* lhs)
{
        _tree_get_call(self)->_lhs = lhs;
}

static inline struct _tree_subscript_expr* _tree_get_subscript(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_SUBSCRIPT);
        return (struct _tree_subscript_expr*)self;
}

static inline const struct _tree_subscript_expr* _tree_get_csubscript(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_SUBSCRIPT);
        return (const struct _tree_subscript_expr*)self;
}

static inline tree_expr* tree_get_subscript_lhs(const tree_expr* self)
{
        return _tree_get_csubscript(self)->_lhs;
}

static inline tree_expr* tree_get_subscript_rhs(const tree_expr* self)
{
        return _tree_get_csubscript(self)->_rhs;
}

static inline void tree_set_subscript_lhs(tree_expr* self, tree_expr* lhs)
{
        _tree_get_subscript(self)->_lhs = lhs;
}

static inline void tree_set_subscript_rhs(tree_expr* self, tree_expr* rhs)
{
        _tree_get_subscript(self)->_rhs = rhs;
}

static inline struct _tree_conditional_expr* _tree_get_conditional(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_CONDITIONAL);
        return (struct _tree_conditional_expr*)self;
}

static inline const struct _tree_conditional_expr* _tree_get_cconditional(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_CONDITIONAL);
        return (const struct _tree_conditional_expr*)self;
}

static inline tree_expr* tree_get_conditional_lhs(const tree_expr* self)
{
        return _tree_get_cconditional(self)->_lhs;
}

static inline tree_expr* tree_get_conditional_rhs(const tree_expr* self)
{
        return _tree_get_cconditional(self)->_rhs;
}

static inline tree_expr* tree_get_conditional_condition(const tree_expr* self)
{
        return _tree_get_cconditional(self)->_condition;
}

static inline void tree_set_conditional_lhs(tree_expr* self, tree_expr* lhs)
{
        _tree_get_conditional(self)->_lhs = lhs;
}

static inline void tree_set_conditional_rhs(tree_expr* self, tree_expr* rhs)
{
        _tree_get_conditional(self)->_rhs = rhs;
}

static inline void tree_set_conditional_condition(tree_expr* self, tree_expr* condition)
{
        _tree_get_conditional(self)->_condition = condition;
}

static inline struct _tree_integer_literal_expr* _tree_get_integer_literal(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_INTEGER_LITERAL);
        return (struct _tree_integer_literal_expr*)self;
}

static inline const struct _tree_integer_literal_expr* _tree_get_cinteger_literal(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_INTEGER_LITERAL);
        return (const struct _tree_integer_literal_expr*)self;
}

static inline suint64 tree_get_integer_literal(const tree_expr* self)
{
        return _tree_get_cinteger_literal(self)->_value;
}

static inline void tree_set_integer_literal(tree_expr* self, suint64 value)
{
        _tree_get_integer_literal(self)->_value = value;
}

static inline struct _tree_character_literal_expr* _tree_get_character_literal(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_CHARACTER_LITERAL);
        return (struct _tree_character_literal_expr*)self;
}

static inline const struct _tree_character_literal_expr* _tree_get_ccharacter_literal(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_CHARACTER_LITERAL);
        return (const struct _tree_character_literal_expr*)self;
}

static inline int tree_get_character_literal(const tree_expr* self)
{
        return _tree_get_ccharacter_literal(self)->_value;
}

static inline void tree_set_character_literal(tree_expr* self, int value)
{
        _tree_get_character_literal(self)->_value = value;
}

static inline struct _tree_floating_literal_expr*
_tree_get_floating_literal(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_FLOATING_LITERAL);
        return (struct _tree_floating_literal_expr*)self;
}

static inline const struct _tree_floating_literal_expr*
_tree_get_cfloating_literal(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_FLOATING_LITERAL);
        return (const struct _tree_floating_literal_expr*)self;
}

static inline const float_value* tree_get_floating_literal_cvalue(const tree_expr* self)
{
        return &_tree_get_cfloating_literal(self)->_value;
}

static inline void tree_set_floating_literal_value(tree_expr* self, const float_value* value)
{
        _tree_get_floating_literal(self)->_value = *value;
}

static inline struct _tree_string_literal_expr* _tree_get_string_literal(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_STRING_LITERAL);
        return (struct _tree_string_literal_expr*)self;
}

static inline const struct _tree_string_literal_expr* _tree_get_cstring_literal(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_STRING_LITERAL);
        return (const struct _tree_string_literal_expr*)self;
}

static inline tree_id tree_get_string_literal(const tree_expr* self)
{
        return _tree_get_cstring_literal(self)->_ref;
}

static inline void tree_set_string_literal(tree_expr* self, tree_id ref)
{
        _tree_get_string_literal(self)->_ref = ref;
}

static inline struct _tree_decl_expr* _tree_get_decl_expr(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_DECL);
        return (struct _tree_decl_expr*)self;
}

static inline const struct _tree_decl_expr* _tree_get_decl_cexp(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_DECL);
        return (const struct _tree_decl_expr*)self;
}

static inline tree_decl* tree_get_decl_expr_entity(const tree_expr* self)
{
        return _tree_get_decl_cexp(self)->_entity;
}

static inline void tree_set_decl_expr_entity(tree_expr* self, tree_decl* decl)
{
        _tree_get_decl_expr(self)->_entity = decl;
}

static inline struct _tree_member_expr* _tree_get_member_expr(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_MEMBER);
        return (struct _tree_member_expr*)self;
}

static inline const struct _tree_member_expr* _tree_get_member_cexp(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_MEMBER);
        return (const struct _tree_member_expr*)self;
}

static inline tree_expr* tree_get_member_expr_lhs(const tree_expr* self)
{
        return _tree_get_member_cexp(self)->_lhs;
}

static inline tree_decl* tree_get_member_expr_decl(const tree_expr* self)
{
        return _tree_get_member_cexp(self)->_decl;
}

static inline bool tree_member_expr_is_arrow(const tree_expr* self)
{
        return _tree_get_member_cexp(self)->_is_arrow;
}

static inline void tree_set_member_expr_lhs(tree_expr* self, tree_expr* lhs)
{
        _tree_get_member_expr(self)->_lhs = lhs;
}

static inline void tree_set_member_expr_decl(tree_expr* self, tree_decl* decl)
{
        _tree_get_member_expr(self)->_decl = decl;
}

static inline void tree_set_member_expr_arrow(tree_expr* self, bool val)
{
        _tree_get_member_expr(self)->_is_arrow = val;
}

static inline struct _tree_sizeof_expr* _tree_get_sizeof(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_SIZEOF);
        return (struct _tree_sizeof_expr*)self;
}

static inline const struct _tree_sizeof_expr* _tree_get_csizeof(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_SIZEOF);
        return (const struct _tree_sizeof_expr*)self;
}

static inline bool tree_sizeof_is_unary(const tree_expr* self)
{
        return _tree_get_csizeof(self)->_is_unary;
}

static inline tree_expr* tree_get_sizeof_expr(const tree_expr* self)
{
        return _tree_get_csizeof(self)->_expr;
}

static inline tree_type* tree_get_sizeof_type(const tree_expr* self)
{
        return _tree_get_csizeof(self)->_type;
}

static inline void tree_set_sizeof_expr(tree_expr* self, tree_expr* expr)
{
        _tree_get_sizeof(self)->_expr = expr;
}

static inline void tree_set_sizeof_type(tree_expr* self, tree_type* type)
{
        _tree_get_sizeof(self)->_type = type;
}

static inline void tree_set_sizeof_unary(tree_expr* self, bool val)
{
        _tree_get_sizeof(self)->_is_unary = val;
}

static inline struct _tree_paren_expr* _tree_get_paren_expr(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_PAREN);
        return (struct _tree_paren_expr*)self;
}

static inline const struct _tree_paren_expr* _tree_get_paren_cexp(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_PAREN);
        return (const struct _tree_paren_expr*)self;
}

static inline tree_expr* tree_get_paren_expr(const tree_expr* self)
{
        return _tree_get_paren_cexp(self)->_expr;
}

static inline void tree_set_paren_expr(tree_expr* self, tree_expr* expr)
{
        _tree_get_paren_expr(self)->_expr = expr;
}

static inline struct _tree_init_expr* _tree_get_init_expr(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_INIT);
        return (struct _tree_init_expr*)self;
}
static inline const struct _tree_init_expr* _tree_get_init_cexp(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_INIT);
        return (const struct _tree_init_expr*)self;
}

static inline tree_designation* tree_get_init_begin(const tree_expr* self)
{
        return (tree_designation*)list_begin(&_tree_get_init_cexp(self)->_designations);
}

static inline const tree_designation* tree_get_init_end(const tree_expr* self)
{
        return (const tree_designation*)list_cend(&_tree_get_init_cexp(self)->_designations);
}

#define TREE_ASSERT_DESIGNATOR(P) S_ASSERT(P)

static inline struct _tree_designator_base* _tree_get_designator(tree_designator* self)
{
        TREE_ASSERT_DESIGNATOR(self);
        return (struct _tree_designator_base*)self;
}

static inline const struct _tree_designator_base* _tree_get_cdesignator(const tree_designator* self)
{
        TREE_ASSERT_DESIGNATOR(self);
        return (const struct _tree_designator_base*)self;
}

static inline tree_designator* tree_get_next_designator(const tree_designator* self)
{
        return (tree_designator*)list_node_next(&_tree_get_cdesignator(self)->_node);
}

static inline tree_designator_kind tree_get_designator_kind(const tree_designator* self)
{
        return _tree_get_cdesignator(self)->_kind;
}

static inline bool tree_designator_is(const tree_designator* self, tree_designator_kind k)
{
        return tree_get_designator_kind(self) == k;
}

static inline void tree_set_designator_kind(tree_designator* self, tree_designator_kind k)
{
        _tree_get_designator(self)->_kind = k;
}

#undef TREE_ASSERT_DESIGNATOR
#define TREE_ASSERT_DESIGNATOR(P, K) S_ASSERT((P) && tree_get_designator_kind(P) == (K))

static inline struct _tree_array_designator* _tree_get_array_designator(tree_designator* self)
{
        TREE_ASSERT_DESIGNATOR(self, TDK_DES_ARRAY);
        return (struct _tree_array_designator*)self;
}

static inline const struct _tree_array_designator* _tree_get_array_cdesignator(const tree_designator* self)
{
        TREE_ASSERT_DESIGNATOR(self, TDK_DES_ARRAY);
        return (const struct _tree_array_designator*)self;
}

static inline tree_type* tree_get_array_designator_type(const tree_designator* self)
{
        return _tree_get_array_cdesignator(self)->_eltype;
}

static inline tree_expr* tree_get_array_designator_index(const tree_designator* self)
{
        return _tree_get_array_cdesignator(self)->_index;
}

static inline void tree_set_array_designator_type(tree_designator* self, tree_type* t)
{
        _tree_get_array_designator(self)->_eltype = t;
}

static inline void tree_set_array_designator_index(tree_designator* self, tree_expr* i)
{
        _tree_get_array_designator(self)->_index = i;
}

static inline struct _tree_member_designator* _tree_get_member_designator(tree_designator* self)
{
        TREE_ASSERT_DESIGNATOR(self, TDK_DES_MEMBER);
        return (struct _tree_member_designator*)self;
}

static inline const struct _tree_member_designator* _tree_get_member_cdesignator(const tree_designator* self)
{
        TREE_ASSERT_DESIGNATOR(self, TDK_DES_MEMBER);
        return (const struct _tree_member_designator*)self;
}

static inline tree_decl* tree_get_member_designator_decl(const tree_designator* self)
{
        return _tree_get_member_cdesignator(self)->_member;
}

static inline void tree_set_member_designator_decl(tree_designator* self, tree_decl* d)
{
        _tree_get_member_designator(self)->_member = d;
}

static inline tree_designation* tree_get_next_designation(const tree_designation* self)
{
        return (tree_designation*)list_node_next(&self->_node);
}

static inline tree_designator* tree_get_designation_begin(const tree_designation* self)
{
        return (tree_designator*)list_begin(&self->_designators);
}

static inline tree_designator* tree_get_designation_last(const tree_designation* self)
{
        const tree_designator* end = tree_get_designation_end(self);
        return (tree_designator*)list_node_prev(&_tree_get_cdesignator(end)->_node);
}

static inline const tree_designator* tree_get_designation_end(const tree_designation* self)
{
        return (const tree_designator*)list_cend(&self->_designators);
}

static inline tree_expr* tree_get_designation_initializer(const tree_designation* self)
{
        return self->_initializer;
}

static inline void tree_set_designation_initializer(tree_designation* self, tree_expr* initializer)
{
        self->_initializer = initializer;
}

static inline struct _tree_impl_init_expr* _tree_get_impl_init_expr(tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_IMPL_INIT);
        return (struct _tree_impl_init_expr*)self;
}

static inline const struct _tree_impl_init_expr* _tree_get_impl_init_cexp(const tree_expr* self)
{
        TREE_ASSERT_EXPR(self, TEK_IMPL_INIT);
        return (const struct _tree_impl_init_expr*)self;
}

static inline tree_expr* tree_get_impl_init_expr(const tree_expr* self)
{
        return _tree_get_impl_init_cexp(self)->_init;
}

static inline void tree_set_impl_init_expr(tree_expr* self, tree_expr* init)
{
        _tree_get_impl_init_expr(self)->_init = init;
}

static inline bool tree_expr_is(const tree_expr* self, tree_expr_kind k)
{
        return tree_get_expr_kind(self) == k;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_EXPR_H
