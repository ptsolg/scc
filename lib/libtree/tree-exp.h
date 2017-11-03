#ifndef TREE_EXP_H
#define TREE_EXP_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-common.h"
#include "tree-type.h" // TTQ_CONST

typedef struct _tree_exp tree_exp;
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
} tree_exp_kind;

typedef enum
{
        TVK_LVALUE,
        TVK_RVALUE,
} tree_value_kind;

struct _tree_exp_base
{
        tree_exp_kind _kind;
        tree_value_kind _value_kind;
        tree_type* _type;
        tree_location _loc;
};

extern tree_exp* tree_new_exp(
        tree_context* context,
        tree_exp_kind kind,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        ssize size);

static inline struct _tree_exp_base* _tree_get_exp(tree_exp* self);
static inline const struct _tree_exp_base* _tree_get_cexp(const tree_exp* self);

static inline bool tree_exp_is(const tree_exp* self, tree_exp_kind k);
static inline tree_exp_kind tree_get_exp_kind(const tree_exp* self);
static inline tree_value_kind tree_get_exp_value_kind(const tree_exp* self);
static inline bool tree_exp_is_lvalue(const tree_exp* self);
static inline bool tree_exp_is_modifiable_lvalue(const tree_exp* self);
static inline tree_type* tree_get_exp_type(const tree_exp* self);
static inline tree_location tree_get_exp_loc(const tree_exp* self);

static inline void tree_set_exp_kind(tree_exp* self, tree_exp_kind k);
static inline void tree_set_exp_value_kind(tree_exp* self, tree_value_kind vk);
static inline void tree_set_exp_type(tree_exp* self, tree_type* t);
static inline void tree_set_exp_loc(tree_exp* self, tree_location loc);

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

struct _tree_binop
{
        struct _tree_exp_base _base;
        tree_binop_kind _kind;
        tree_exp* _lhs;
        tree_exp* _rhs;
};

extern tree_exp* tree_new_binop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_binop_kind kind,
        tree_exp* lhs,
        tree_exp* rhs);

static inline struct _tree_binop* _tree_get_binop(tree_exp* self);
static inline const struct _tree_binop* _tree_get_cbinop(const tree_exp* self);

static inline bool tree_binop_is(const tree_exp* self, tree_binop_kind k);
static inline tree_binop_kind tree_get_binop_kind(const tree_exp* self);
static inline tree_exp* tree_get_binop_lhs(const tree_exp* self);
static inline tree_exp* tree_get_binop_rhs(const tree_exp* self);

static inline void tree_set_binop_kind(tree_exp* self, tree_binop_kind k);
static inline void tree_set_binop_lhs(tree_exp* self, tree_exp* lhs);
static inline void tree_set_binop_rhs(tree_exp* self, tree_exp* rhs);

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

struct _tree_unop
{
        struct _tree_exp_base _base;
        tree_unop_kind _kind;
        tree_exp* _exp;
};

extern tree_exp* tree_new_unop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_unop_kind kind,
        tree_exp* exp);

static inline struct _tree_unop* _tree_get_unop(tree_exp* self);
static inline const struct _tree_unop* _tree_get_cunop(const tree_exp* self);

static inline bool tree_unop_is(const tree_exp* self, tree_unop_kind k);
static inline tree_unop_kind tree_get_unop_kind(const tree_exp* self);
static inline tree_exp* tree_get_unop_exp(const tree_exp* self);

static inline void tree_set_unop_kind(tree_exp* self, tree_unop_kind k);
static inline void tree_set_unop_exp(tree_exp* self, tree_exp* exp);

struct _tree_cast_exp
{
        struct _tree_exp_base _base;
        tree_exp* _exp;
};

extern tree_exp* tree_new_explicit_cast_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_exp* exp);

extern tree_exp* tree_new_implicit_cast_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_exp* exp);

static inline struct _tree_cast_exp* _tree_get_cast(tree_exp* self);
static inline const struct _tree_cast_exp* _tree_get_ccast(const tree_exp* self);

static inline tree_exp* tree_get_cast_exp(const tree_exp* self);
static inline void tree_set_cast_exp(tree_exp* self, tree_exp* exp);

struct _tree_call_exp
{
        struct _tree_exp_base _base;
        tree_exp* _lhs;
        objgroup _args;
};

extern tree_exp* tree_new_call_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_exp* lhs);

extern void tree_set_call_args(tree_exp* self, objgroup* args);
extern void tree_add_call_arg(tree_exp* self, tree_exp* arg);

static inline struct _tree_call_exp* _tree_get_call(tree_exp* self);
static inline const struct _tree_call_exp* _tree_get_ccall(const tree_exp* self);

static inline ssize tree_get_call_nargs(const tree_exp* self);
static inline tree_exp* tree_get_call_lhs(const tree_exp* self);
static inline tree_exp** tree_get_call_begin(const tree_exp* self);
static inline tree_exp** tree_get_call_end(const tree_exp* self);

static inline void tree_set_call_lhs(tree_exp* self, tree_exp* lhs);

#define TREE_FOREACH_CALL(PEXP, ITNAME) \
        for (tree_exp** ITNAME = tree_get_call_begin(PEXP); \
                ITNAME != tree_get_call_end(PEXP); ITNAME++)

struct _tree_subscript_exp
{
        struct _tree_exp_base _base;
        tree_exp* _lhs;
        tree_exp* _rhs;
};

extern tree_exp* tree_new_subscript_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_exp* lhs,
        tree_exp* rhs);

static inline struct _tree_subscript_exp* _tree_get_subscript(tree_exp* self);
static inline const struct _tree_subscript_exp* _tree_get_csubscript(const tree_exp* self);

static inline tree_exp* tree_get_subscript_lhs(const tree_exp* self);
static inline tree_exp* tree_get_subscript_rhs(const tree_exp* self);

static inline void tree_set_subscript_lhs(tree_exp* self, tree_exp* lhs);
static inline void tree_set_subscript_rhs(tree_exp* self, tree_exp* rhs);

struct _tree_conditional_exp
{
        struct _tree_exp_base _base;
        tree_exp* _condition;
        tree_exp* _lhs;
        tree_exp* _rhs;
};

extern tree_exp* tree_new_conditional_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_exp* condition,
        tree_exp* lhs,
        tree_exp* rhs);

static inline struct _tree_conditional_exp* _tree_get_conditional(tree_exp* self);
static inline const struct _tree_conditional_exp* _tree_get_cconditional(const tree_exp* self);

static inline tree_exp* tree_get_conditional_lhs(const tree_exp* self);
static inline tree_exp* tree_get_conditional_rhs(const tree_exp* self);
static inline tree_exp* tree_get_conditional_condition(const tree_exp* self);

static inline void tree_set_conditional_lhs(tree_exp* self, tree_exp* lhs);
static inline void tree_set_conditional_rhs(tree_exp* self, tree_exp* rhs);
static inline void tree_set_conditional_condition(tree_exp* self, tree_exp* condition);

struct _tree_integer_literal_exp
{
        struct _tree_exp_base _base;
        suint64 _value;
};

extern tree_exp* tree_new_integer_literal(
        tree_context* context, tree_type* type, tree_location loc, suint64 value);

static inline struct _tree_integer_literal_exp* _tree_get_integer_literal(tree_exp* self);
static inline const struct _tree_integer_literal_exp* _tree_get_cinteger_literal(const tree_exp* self);

static inline suint64 tree_get_integer_literal(const tree_exp* self);
static inline void tree_set_integer_literal(tree_exp* self, suint64 value);

struct _tree_character_literal_exp
{
        struct _tree_exp_base _base;
        int _value;
};

extern tree_exp* tree_new_character_literal(
        tree_context* context, tree_type* type, tree_location loc, int value);

static inline struct _tree_character_literal_exp* _tree_get_character_literal(tree_exp* self);
static inline const struct _tree_character_literal_exp* _tree_get_ccharacter_literal(const tree_exp* self);

static inline int tree_get_character_literal(const tree_exp* self);
static inline void tree_set_character_literal(tree_exp* self, int value);

struct _tree_floating_literal_exp
{
        struct _tree_exp_base _base;
        union
        {
                float _float;
                ldouble _double;
        };
};

extern tree_exp* tree_new_floating_literal(
        tree_context* context, tree_type* type, tree_location loc, float value);

extern tree_exp* tree_new_floating_lliteral(
        tree_context* context, tree_type* type, tree_location loc, ldouble value);

static inline struct _tree_floating_literal_exp* _tree_get_floating_literal(tree_exp* self);
static inline const struct _tree_floating_literal_exp* _tree_get_cfloating_literal(const tree_exp* self);

static inline float tree_get_floating_literal(const tree_exp* self);
static inline ldouble tree_get_floating_lliteral(const tree_exp* self);
static inline void tree_set_floating_literal(tree_exp* self, float value);
static inline void tree_set_floating_lliteral(tree_exp* self, ldouble value);

struct _tree_string_literal_exp
{
        struct _tree_exp_base _base;
        tree_id _ref;
};

extern tree_exp* tree_new_string_literal(
        tree_context* context, tree_type* type, tree_location loc, tree_id ref);

static inline struct _tree_string_literal_exp* _tree_get_string_literal(tree_exp* self);
static inline const struct _tree_string_literal_exp* _tree_get_cstring_literal(const tree_exp* self);

static inline tree_id tree_get_string_literal(const tree_exp* self);
static inline void tree_set_string_literal(tree_exp* self, tree_id ref);

struct _tree_decl_exp
{
        struct _tree_exp_base _base;
        tree_decl* _entity;
};

extern tree_exp* tree_new_decl_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_decl* decl);

static inline struct _tree_decl_exp* _tree_get_decl_exp(tree_exp* self);
static inline const struct _tree_decl_exp* _tree_get_decl_cexp(const tree_exp* self);

static inline tree_decl* tree_get_decl_exp_entity(const tree_exp* self);
static inline void tree_set_decl_exp_entity(tree_exp* self, tree_decl* decl);

struct _tree_member_exp
{
        struct _tree_exp_base _base;
        tree_exp* _lhs;
        tree_decl* _decl;
        bool _is_arrow;
};

extern tree_exp* tree_new_member_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_exp* lhs,
        tree_decl* decl,
        bool is_arrow);

static inline struct _tree_member_exp* _tree_get_member_exp(tree_exp* self);
static inline const struct _tree_member_exp* _tree_get_member_cexp(const tree_exp* self);

static inline tree_exp* tree_get_member_exp_lhs(const tree_exp* self);
static inline tree_decl* tree_get_member_exp_decl(const tree_exp* self);
static inline bool tree_member_exp_is_arrow(const tree_exp* self);

static inline void tree_set_member_exp_lhs(tree_exp* self, tree_exp* lhs);
static inline void tree_set_member_exp_decl(tree_exp* self, tree_decl* decl);
static inline void tree_set_member_exp_arrow(tree_exp* self, bool val);

struct _tree_sizeof_exp
{
        struct _tree_exp_base _base;
        bool _is_unary;

        union
        {
                tree_exp* _exp;
                tree_type* _type;
        };
};

extern tree_exp* tree_new_sizeof_exp(
        tree_context* context, tree_type* type, tree_location loc, void* rhs, bool is_unary);

static inline struct _tree_sizeof_exp* _tree_get_sizeof(tree_exp* self);
static inline const struct _tree_sizeof_exp* _tree_get_csizeof(const tree_exp* self);

static inline bool tree_sizeof_is_unary(const tree_exp* self);
static inline tree_exp* tree_get_sizeof_exp(const tree_exp* self);
static inline tree_type* tree_get_sizeof_type(const tree_exp* self);

static inline void tree_set_sizeof_exp(tree_exp* self, tree_exp* exp);
static inline void tree_set_sizeof_type(tree_exp* self, tree_type* type);
static inline void tree_set_sizeof_unary(tree_exp* self, bool val);

struct _tree_paren_exp
{
        struct _tree_exp_base _base;
        tree_exp* _exp;
}; 

extern tree_exp* tree_new_paren_exp(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_exp* exp);

static inline struct _tree_paren_exp* _tree_get_paren_exp(tree_exp* self);
static inline const struct _tree_paren_exp* _tree_get_paren_cexp(const tree_exp* self);

static inline tree_exp* tree_get_paren_exp(const tree_exp* self);
static inline void tree_set_paren_exp(tree_exp* self, tree_exp* exp);

struct _tree_init_exp
{
        struct _tree_exp_base _base;
        list_head _designations;
}; 

extern tree_exp* tree_new_init_exp(tree_context* context, tree_location loc);
extern void tree_add_init_designation(tree_exp* self, tree_designation* d);

static inline struct _tree_init_exp* _tree_get_init_exp(tree_exp* self);
static inline const struct _tree_init_exp* _tree_get_init_cexp(const tree_exp* self); 

static inline tree_designation* tree_get_init_begin(const tree_exp* self);
static inline const tree_designation* tree_get_init_end(const tree_exp* self);

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
static inline bool tree_designator_is(const tree_designator* self,
                                                        tree_designator_kind k);

static inline void tree_set_designator_kind(tree_designator* self, tree_designator_kind k);

struct _tree_array_designator
{
        struct _tree_designator_base _base;
        tree_type* _eltype;
        tree_exp* _index;
};

extern tree_designator* tree_new_array_designator(
        tree_context* context, tree_type* eltype, tree_exp* index);

static inline struct _tree_array_designator* _tree_get_array_designator(tree_designator* self);
static inline const struct _tree_array_designator* _tree_get_array_cdesignator(const tree_designator* self);

static inline tree_type* tree_get_array_designator_type(const tree_designator* self);
static inline tree_exp* tree_get_array_designator_index(const tree_designator* self);

static inline void tree_set_array_designator_type(tree_designator* self, tree_type* t);
static inline void tree_set_array_designator_index(tree_designator* self, tree_exp* i);

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
        tree_exp* _initializer;
} tree_designation;

extern tree_designation* tree_new_designation(tree_context* context, tree_exp* initializer);

extern void tree_add_designation_designator(tree_designation* self, tree_designator* d);

static inline tree_designation* tree_get_next_designation(const tree_designation* self);
static inline tree_designator* tree_get_designation_begin(const tree_designation* self);
static inline tree_designator* tree_get_designation_last(const tree_designation* self);
static inline const tree_designator* tree_get_designation_end(const tree_designation* self);
static inline tree_exp* tree_get_designation_initializer(const tree_designation* self);

static inline void tree_set_designation_initializer(tree_designation* self, tree_exp* initializer);

#define TREE_FOREACH_DESIGNATOR(PEXP, ITNAME) \
        for (tree_designator* ITNAME = tree_get_designation_begin(PEXP); \
                ITNAME != tree_get_designation_end(PEXP); \
                ITNAME = tree_get_next_designator(ITNAME))

struct _tree_impl_init_exp
{
        struct _tree_exp_base _base;
        tree_exp* _init;
};

extern tree_exp* tree_new_impl_init_exp(tree_context* context, tree_exp* init);

static inline struct _tree_impl_init_exp* _tree_get_impl_init_exp(tree_exp* self);
static inline const struct _tree_impl_init_exp* _tree_get_impl_init_cexp(const tree_exp* self);

static inline tree_exp* tree_get_impl_init_exp(const tree_exp* self);
static inline void tree_set_impl_init_exp(tree_exp* self, tree_exp* init);

typedef struct _tree_exp
{
        union
        {
                struct _tree_binop _binary;
                struct _tree_unop _unary;
                struct _tree_cast_exp _cast;
                struct _tree_call_exp _call;
                struct _tree_subscript_exp _subscript;
                struct _tree_conditional_exp _conditional;
                struct _tree_string_literal_exp _string;
                struct _tree_integer_literal_exp _int;
                struct _tree_floating_literal_exp _float;
                struct _tree_character_literal_exp _char;
                struct _tree_decl_exp _decl;
                struct _tree_member_exp _member;
                struct _tree_sizeof_exp _sizeof;
                struct _tree_paren_exp _paren;
                struct _tree_init_exp _init;
        };
} tree_exp;

extern bool tree_exp_is_literal(const tree_exp* self);
extern const tree_exp* tree_ignore_ccasts(const tree_exp* self);
extern tree_exp* tree_ignore_impl_casts(tree_exp* self);
extern const tree_exp* tree_ignore_impl_ccasts(const tree_exp* self);
extern tree_exp* tree_ignore_paren_exps(tree_exp* self);
extern const tree_exp* tree_ignore_paren_cexps(const tree_exp* self);
// ignores implicit casts and parenthesises, if any
extern tree_exp* tree_desugar_exp(tree_exp* self);
extern const tree_exp* tree_desugar_cexp(const tree_exp* self);

extern bool tree_exp_is_null_pointer_constant(const tree_exp* self);
extern bool tree_exp_designates_bitfield(const tree_exp* self);

#define TREE_ASSERT_EXP(E) S_ASSERT(E)

static inline struct _tree_exp_base* _tree_get_exp(tree_exp* self)
{
        TREE_ASSERT_EXP(self);
        return (struct _tree_exp_base*)self;
}

static inline const struct _tree_exp_base* _tree_get_cexp(const tree_exp* self)
{
        TREE_ASSERT_EXP(self);
        return (const struct _tree_exp_base*)self;
}

static inline tree_exp_kind tree_get_exp_kind(const tree_exp* self)
{
        return _tree_get_cexp(self)->_kind;
}

static inline tree_value_kind tree_get_exp_value_kind(const tree_exp* self)
{
        return _tree_get_cexp(self)->_value_kind;
}

static inline bool tree_exp_is_lvalue(const tree_exp* self)
{
        return tree_get_exp_value_kind(self) == TVK_LVALUE;
}

static inline bool tree_exp_is_modifiable_lvalue(const tree_exp* self)
{
        return tree_exp_is_lvalue(self)
                && !(tree_get_type_quals(tree_get_exp_type(self)) & TTQ_CONST);
}

static inline tree_type* tree_get_exp_type(const tree_exp* self)
{
        return _tree_get_cexp(self)->_type;
}

static inline tree_location tree_get_exp_loc(const tree_exp* self)
{
        return _tree_get_cexp(self)->_loc;
}

static inline void tree_set_exp_kind(tree_exp* self, tree_exp_kind k)
{
        _tree_get_exp(self)->_kind = k;
}

static inline void tree_set_exp_value_kind(tree_exp* self, tree_value_kind vk)
{
        _tree_get_exp(self)->_value_kind = vk;
}

static inline void tree_set_exp_type(tree_exp* self, tree_type* t)
{
        _tree_get_exp(self)->_type = t;
}

static inline void tree_set_exp_loc(tree_exp* self, tree_location loc)
{
        _tree_get_exp(self)->_loc = loc;
}

#undef TREE_ASSERT_EXP
#define TREE_ASSERT_EXP(E, K) S_ASSERT((E) && tree_get_exp_kind(E) == (K))

static inline struct _tree_binop* _tree_get_binop(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_BINARY);
        return (struct _tree_binop*)self;
}

static inline const struct _tree_binop* _tree_get_cbinop(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_BINARY);
        return (const struct _tree_binop*)self;
}

static inline bool tree_binop_is(const tree_exp* self, tree_binop_kind k)
{
        return tree_get_binop_kind(self) == k;
}

static inline tree_binop_kind tree_get_binop_kind(const tree_exp* self)
{
        return _tree_get_cbinop(self)->_kind;
}

static inline tree_exp* tree_get_binop_lhs(const tree_exp* self)
{
        return _tree_get_cbinop(self)->_lhs;
}

static inline tree_exp* tree_get_binop_rhs(const tree_exp* self)
{
        return _tree_get_cbinop(self)->_rhs;
}

static inline void tree_set_binop_kind(tree_exp* self, tree_binop_kind k)
{
        _tree_get_binop(self)->_kind = k;
}

static inline void tree_set_binop_lhs(tree_exp* self, tree_exp* lhs)
{
        _tree_get_binop(self)->_lhs = lhs;
}

static inline void tree_set_binop_rhs(tree_exp* self, tree_exp* rhs)
{
        _tree_get_binop(self)->_rhs = rhs;
}

static inline struct _tree_unop* _tree_get_unop(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_UNARY);
        return (struct _tree_unop*)self;
}

static inline const struct _tree_unop* _tree_get_cunop(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_UNARY);
        return (const struct _tree_unop*)self;
}

static inline bool tree_unop_is(const tree_exp* self, tree_unop_kind k)
{
        return tree_get_unop_kind(self) == k;
}

static inline tree_unop_kind tree_get_unop_kind(const tree_exp* self)
{
        return _tree_get_cunop(self)->_kind;
}

static inline tree_exp* tree_get_unop_exp(const tree_exp* self)
{
        return _tree_get_cunop(self)->_exp;
}

static inline void tree_set_unop_kind(tree_exp* self, tree_unop_kind k)
{
        _tree_get_unop(self)->_kind = k;
}

static inline void tree_set_unop_exp(tree_exp* self, tree_exp* exp)
{
        _tree_get_unop(self)->_exp = exp;
}

#define TREE_ASSERT_CAST(P) \
        S_ASSERT((P) && tree_get_exp_kind(P) == TEK_EXPLICIT_CAST \
                     || tree_get_exp_kind(P) == TEK_IMPLICIT_CAST)

static inline struct _tree_cast_exp* _tree_get_cast(tree_exp* self)
{
        TREE_ASSERT_CAST(self);
        return (struct _tree_cast_exp*)self;
}

static inline const struct _tree_cast_exp* _tree_get_ccast(const tree_exp* self)
{
        TREE_ASSERT_CAST(self);
        return (const struct _tree_cast_exp*)self;
}

static inline tree_exp* tree_get_cast_exp(const tree_exp* self)
{
        return _tree_get_ccast(self)->_exp;
}

static inline void tree_set_cast_exp(tree_exp* self, tree_exp* exp)
{
        _tree_get_cast(self)->_exp = exp;
}

static inline struct _tree_call_exp* _tree_get_call(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_CALL);
        return (struct _tree_call_exp*)self;
}

static inline const struct _tree_call_exp* _tree_get_ccall(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_CALL);
        return (const struct _tree_call_exp*)self;
}

static inline ssize tree_get_call_nargs(const tree_exp* self)
{
        return objgroup_size(&_tree_get_ccall(self)->_args);
}

static inline tree_exp* tree_get_call_lhs(const tree_exp* self)
{
        return _tree_get_ccall(self)->_lhs;
}

static inline tree_exp** tree_get_call_begin(const tree_exp* self)
{
        return (tree_exp**)objgroup_begin(&_tree_get_ccall(self)->_args);
}

static inline tree_exp** tree_get_call_end(const tree_exp* self)
{
        return (tree_exp**)objgroup_end(&_tree_get_ccall(self)->_args);
}

static inline void tree_set_call_lhs(tree_exp* self, tree_exp* lhs)
{
        _tree_get_call(self)->_lhs = lhs;
}

static inline struct _tree_subscript_exp* _tree_get_subscript(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_SUBSCRIPT);
        return (struct _tree_subscript_exp*)self;
}

static inline const struct _tree_subscript_exp* _tree_get_csubscript(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_SUBSCRIPT);
        return (const struct _tree_subscript_exp*)self;
}

static inline tree_exp* tree_get_subscript_lhs(const tree_exp* self)
{
        return _tree_get_csubscript(self)->_lhs;
}

static inline tree_exp* tree_get_subscript_rhs(const tree_exp* self)
{
        return _tree_get_csubscript(self)->_rhs;
}

static inline void tree_set_subscript_lhs(tree_exp* self, tree_exp* lhs)
{
        _tree_get_subscript(self)->_lhs = lhs;
}

static inline void tree_set_subscript_rhs(tree_exp* self, tree_exp* rhs)
{
        _tree_get_subscript(self)->_rhs = rhs;
}

static inline struct _tree_conditional_exp* _tree_get_conditional(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_CONDITIONAL);
        return (struct _tree_conditional_exp*)self;
}

static inline const struct _tree_conditional_exp* _tree_get_cconditional(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_CONDITIONAL);
        return (const struct _tree_conditional_exp*)self;
}

static inline tree_exp* tree_get_conditional_lhs(const tree_exp* self)
{
        return _tree_get_cconditional(self)->_lhs;
}

static inline tree_exp* tree_get_conditional_rhs(const tree_exp* self)
{
        return _tree_get_cconditional(self)->_rhs;
}

static inline tree_exp* tree_get_conditional_condition(const tree_exp* self)
{
        return _tree_get_cconditional(self)->_condition;
}

static inline void tree_set_conditional_lhs(tree_exp* self, tree_exp* lhs)
{
        _tree_get_conditional(self)->_lhs = lhs;
}

static inline void tree_set_conditional_rhs(tree_exp* self, tree_exp* rhs)
{
        _tree_get_conditional(self)->_rhs = rhs;
}

static inline void tree_set_conditional_condition(tree_exp* self, tree_exp* condition)
{
        _tree_get_conditional(self)->_condition = condition;
}

static inline struct _tree_integer_literal_exp* _tree_get_integer_literal(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_INTEGER_LITERAL);
        return (struct _tree_integer_literal_exp*)self;
}

static inline const struct _tree_integer_literal_exp* _tree_get_cinteger_literal(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_INTEGER_LITERAL);
        return (const struct _tree_integer_literal_exp*)self;
}

static inline suint64 tree_get_integer_literal(const tree_exp* self)
{
        return _tree_get_cinteger_literal(self)->_value;
}

static inline void tree_set_integer_literal(tree_exp* self, suint64 value)
{
        _tree_get_integer_literal(self)->_value = value;
}

static inline struct _tree_character_literal_exp* _tree_get_character_literal(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_CHARACTER_LITERAL);
        return (struct _tree_character_literal_exp*)self;
}

static inline const struct _tree_character_literal_exp* _tree_get_ccharacter_literal(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_CHARACTER_LITERAL);
        return (const struct _tree_character_literal_exp*)self;
}

static inline int tree_get_character_literal(const tree_exp* self)
{
        return _tree_get_ccharacter_literal(self)->_value;
}

static inline void tree_set_character_literal(tree_exp* self, int value)
{
        _tree_get_character_literal(self)->_value = value;
}

static inline struct _tree_floating_literal_exp* _tree_get_floating_literal(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_FLOATING_LITERAL);
        return (struct _tree_floating_literal_exp*)self;
}

static inline const struct _tree_floating_literal_exp* _tree_get_cfloating_literal(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_FLOATING_LITERAL);
        return (const struct _tree_floating_literal_exp*)self;
}

static inline float tree_get_floating_literal(const tree_exp* self)
{
        return _tree_get_cfloating_literal(self)->_float;
}

static inline ldouble tree_get_floating_lliteral(const tree_exp* self)
{
        return _tree_get_cfloating_literal(self)->_double;
}

static inline void tree_set_floating_literal(tree_exp* self, float value)
{
        _tree_get_floating_literal(self)->_float = value;
}

static inline void tree_set_floating_lliteral(tree_exp* self, ldouble value)
{
        _tree_get_floating_literal(self)->_double = value;
}

static inline struct _tree_string_literal_exp* _tree_get_string_literal(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_STRING_LITERAL);
        return (struct _tree_string_literal_exp*)self;
}

static inline const struct _tree_string_literal_exp* _tree_get_cstring_literal(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_STRING_LITERAL);
        return (const struct _tree_string_literal_exp*)self;
}

static inline tree_id tree_get_string_literal(const tree_exp* self)
{
        return _tree_get_cstring_literal(self)->_ref;
}

static inline void tree_set_string_literal(tree_exp* self, tree_id ref)
{
        _tree_get_string_literal(self)->_ref = ref;
}

static inline struct _tree_decl_exp* _tree_get_decl_exp(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_DECL);
        return (struct _tree_decl_exp*)self;
}

static inline const struct _tree_decl_exp* _tree_get_decl_cexp(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_DECL);
        return (const struct _tree_decl_exp*)self;
}

static inline tree_decl* tree_get_decl_exp_entity(const tree_exp* self)
{
        return _tree_get_decl_cexp(self)->_entity;
}

static inline void tree_set_decl_exp_entity(tree_exp* self, tree_decl* decl)
{
        _tree_get_decl_exp(self)->_entity = decl;
}

static inline struct _tree_member_exp* _tree_get_member_exp(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_MEMBER);
        return (struct _tree_member_exp*)self;
}

static inline const struct _tree_member_exp* _tree_get_member_cexp(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_MEMBER);
        return (const struct _tree_member_exp*)self;
}

static inline tree_exp* tree_get_member_exp_lhs(const tree_exp* self)
{
        return _tree_get_member_cexp(self)->_lhs;
}

static inline tree_decl* tree_get_member_exp_decl(const tree_exp* self)
{
        return _tree_get_member_cexp(self)->_decl;
}

static inline bool tree_member_exp_is_arrow(const tree_exp* self)
{
        return _tree_get_member_cexp(self)->_is_arrow;
}

static inline void tree_set_member_exp_lhs(tree_exp* self, tree_exp* lhs)
{
        _tree_get_member_exp(self)->_lhs = lhs;
}

static inline void tree_set_member_exp_decl(tree_exp* self, tree_decl* decl)
{
        _tree_get_member_exp(self)->_decl = decl;
}

static inline void tree_set_member_exp_arrow(tree_exp* self, bool val)
{
        _tree_get_member_exp(self)->_is_arrow = val;
}

static inline struct _tree_sizeof_exp* _tree_get_sizeof(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_SIZEOF);
        return (struct _tree_sizeof_exp*)self;
}

static inline const struct _tree_sizeof_exp* _tree_get_csizeof(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_SIZEOF);
        return (const struct _tree_sizeof_exp*)self;
}

static inline bool tree_sizeof_is_unary(const tree_exp* self)
{
        return _tree_get_csizeof(self)->_is_unary;
}

static inline tree_exp* tree_get_sizeof_exp(const tree_exp* self)
{
        return _tree_get_csizeof(self)->_exp;
}

static inline tree_type* tree_get_sizeof_type(const tree_exp* self)
{
        return _tree_get_csizeof(self)->_type;
}

static inline void tree_set_sizeof_exp(tree_exp* self, tree_exp* exp)
{
        _tree_get_sizeof(self)->_exp = exp;
}

static inline void tree_set_sizeof_type(tree_exp* self, tree_type* type)
{
        _tree_get_sizeof(self)->_type = type;
}

static inline void tree_set_sizeof_unary(tree_exp* self, bool val)
{
        _tree_get_sizeof(self)->_is_unary = val;
}

static inline struct _tree_paren_exp* _tree_get_paren_exp(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_PAREN);
        return (struct _tree_paren_exp*)self;
}

static inline const struct _tree_paren_exp* _tree_get_paren_cexp(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_PAREN);
        return (const struct _tree_paren_exp*)self;
}

static inline tree_exp* tree_get_paren_exp(const tree_exp* self)
{
        return _tree_get_paren_cexp(self)->_exp;
}

static inline void tree_set_paren_exp(tree_exp* self, tree_exp* exp)
{
        _tree_get_paren_exp(self)->_exp = exp;
}

static inline struct _tree_init_exp* _tree_get_init_exp(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_INIT);
        return (struct _tree_init_exp*)self;
}
static inline const struct _tree_init_exp* _tree_get_init_cexp(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_INIT);
        return (const struct _tree_init_exp*)self;
}

static inline tree_designation* tree_get_init_begin(const tree_exp* self)
{
        return (tree_designation*)list_begin(&_tree_get_init_cexp(self)->_designations);
}

static inline const tree_designation* tree_get_init_end(const tree_exp* self)
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

static inline tree_exp* tree_get_array_designator_index(const tree_designator* self)
{
        return _tree_get_array_cdesignator(self)->_index;
}

static inline void tree_set_array_designator_type(tree_designator* self, tree_type* t)
{
        _tree_get_array_designator(self)->_eltype = t;
}

static inline void tree_set_array_designator_index(tree_designator* self, tree_exp* i)
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

static inline tree_exp* tree_get_designation_initializer(const tree_designation* self)
{
        return self->_initializer;
}

static inline void tree_set_designation_initializer(tree_designation* self, tree_exp* initializer)
{
        self->_initializer = initializer;
}

static inline struct _tree_impl_init_exp* _tree_get_impl_init_exp(tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_IMPL_INIT);
        return (struct _tree_impl_init_exp*)self;
}

static inline const struct _tree_impl_init_exp* _tree_get_impl_init_cexp(const tree_exp* self)
{
        TREE_ASSERT_EXP(self, TEK_IMPL_INIT);
        return (const struct _tree_impl_init_exp*)self;
}

static inline tree_exp* tree_get_impl_init_exp(const tree_exp* self)
{
        return _tree_get_impl_init_cexp(self)->_init;
}

static inline void tree_set_impl_init_exp(tree_exp* self, tree_exp* init)
{
        _tree_get_impl_init_exp(self)->_init = init;
}

static inline bool tree_exp_is(const tree_exp* self, tree_exp_kind k)
{
        return tree_get_exp_kind(self) == k;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_EXP_H
