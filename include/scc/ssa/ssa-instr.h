#ifndef SSA_INSTR_H
#define SSA_INSTR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-value.h"

typedef struct _tree_type tree_type;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_context ssa_context;
typedef struct _tree_decl tree_decl;

typedef enum
{
        SIK_INVALID,
        SIK_ALLOCA,
        SIK_LOAD,
        SIK_CAST,
        SIK_BINARY,
        SIK_STORE,
        SIK_GETADDR,
        SIK_CALL,
        SIK_PHI,
        SIK_SIZE,
} ssa_instr_kind;

#define SSA_ASSERT_INSTR_KIND(K) S_ASSERT((K) > SIK_INVALID && (K) < SIK_SIZE)

struct _ssa_instr_base
{
        list_node _node;
        ssa_instr_kind _kind;
        ssa_variable _val;
};

extern ssa_instr* ssa_new_instr(
        ssa_context* context, ssa_instr_kind kind, ssa_id id, tree_type* type, ssize size);

extern bool ssa_instr_is_unary(const ssa_instr* self);
extern bool ssa_instr_is_binary(const ssa_instr* self);
extern bool ssa_instr_is_ternary(const ssa_instr* self);
extern bool ssa_instr_is_nary(const ssa_instr* self);
extern bool ssa_instr_has_var(const ssa_instr* self);

extern ssa_value** ssa_get_instr_operands_begin(ssa_instr* self);
extern ssa_value** ssa_get_instr_operands_end(ssa_instr* self);

static inline struct _ssa_instr_base* _ssa_get_instr_base(ssa_instr* self);
static inline const struct _ssa_instr_base* _ssa_get_instr_cbase(const ssa_instr* self);

static inline ssa_instr* ssa_get_var_instr(ssa_value* self);
static inline const ssa_instr* ssa_get_var_cinstr(const ssa_value* self);

static inline ssa_instr_kind ssa_get_instr_kind(const ssa_instr* self);
static inline ssa_value* ssa_get_instr_var(ssa_instr* self);
static inline const ssa_value* ssa_get_instr_cvar(const ssa_instr* self);
static inline ssa_instr* ssa_get_next_instr(const ssa_instr* self);
static inline ssa_instr* ssa_get_prev_instr(const ssa_instr* self);

static inline void ssa_set_instr_kind(ssa_instr* self, ssa_instr_kind kind);
static inline void ssa_remove_instr(ssa_instr* self);

struct _ssa_unary_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _operand;
};

extern ssa_instr* ssa_new_unary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* operand);

static inline struct _ssa_unary_instr* _ssa_get_unary_instr(ssa_instr* self);
static inline const struct _ssa_unary_instr* _ssa_get_unary_cinstr(const ssa_instr* self);

static inline ssa_value* ssa_get_unary_instr_operand(const ssa_instr* self);
static inline ssa_value** ssa_get_unary_instr_poperand(ssa_instr* self);
static inline ssa_value* const* ssa_get_unary_instr_cpoperand(const ssa_instr* self);

static inline void ssa_set_unary_instr_operand(ssa_instr* self, ssa_value* value);

struct _ssa_binary_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _operands[2];
};

extern ssa_instr* ssa_new_binary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* first,
        ssa_value* second,
        ssize size);

static inline struct _ssa_binary_instr* _ssa_get_binary_instr(ssa_instr* self);
static inline const struct _ssa_binary_instr* _ssa_get_binary_cinstr(const ssa_instr* self);

static inline ssa_value* const* ssa_get_binary_instr_operands_cbegin(const ssa_instr* self);
static inline ssa_value** ssa_get_binary_instr_operands_begin(ssa_instr* self);
static inline ssa_value* const* ssa_get_binary_instr_operands_cend(const ssa_instr* self);
static inline ssa_value** ssa_get_binary_instr_operands_end(ssa_instr* self);

struct _ssa_ternary_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _operands[3];
};

extern ssa_instr* ssa_new_ternary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* first,
        ssa_value* second,
        ssa_value* third);

static inline struct _ssa_ternary_instr* _ssa_get_ternary_instr(ssa_instr* self);
static inline const struct _ssa_ternary_instr* _ssa_get_ternary_cinstr(const ssa_instr* self);

static inline ssa_value* const* ssa_get_ternary_instr_operands_cbegin(const ssa_instr* self);
static inline ssa_value** ssa_get_ternary_instr_operands_begin(ssa_instr* self);
static inline ssa_value* const* ssa_get_ternary_instr_operands_cend(const ssa_instr* self);
static inline ssa_value** ssa_get_ternary_instr_operands_end(ssa_instr* self);

struct _ssa_nary_instr
{
        struct _ssa_instr_base _base;
        dseq _operands;
};

extern ssa_instr* ssa_new_nary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type);

extern void ssa_add_nary_instr_operand(ssa_instr* self, ssa_value* operand);

static inline struct _ssa_nary_instr* _ssa_get_nary_instr(ssa_instr* self);
static inline const struct _ssa_nary_instr* _ssa_get_nary_cinstr(const ssa_instr* self);

static inline ssa_value** ssa_get_nary_instr_operands_begin(const ssa_instr* self);
static inline ssa_value** ssa_get_nary_instr_operands_end(const ssa_instr* self);

#define SSA_FOREACH_NARY_INSTR_OPERAND(PINSTR, ITNAME, ENDNAME) \
        for (ssa_value** ITNAME = ssa_get_nary_instr_operands_begin(PINSTR), \
                **ENDNAME = ssa_get_nary_instr_operands_end(PINSTR); \
                ITNAME != ENDNAME; ITNAME++)

typedef enum
{
        SBIK_INVALID,

        SBIK_MUL,
        SBIK_DIV,
        SBIK_MOD,
        SBIK_ADD,
        SBIK_SUB,
        SBIK_SHL,
        SBIK_SHR,
        SBIK_AND,
        SBIK_OR,
        SBIK_XOR,

        SBIK_LE,
        SBIK_GR,
        SBIK_LEQ,
        SBIK_GEQ,
        SBIK_EQ,
        SBIK_NEQ,

        SBIK_SIZE,
} ssa_binary_instr_kind;

#define SSA_ASSERT_BINARY_INSTR_KIND(K) S_ASSERT((K) > SBIK_INVALID && (K) < SBIK_SIZE)

struct _ssa_binop_instr
{
        struct _ssa_binary_instr _base;
        ssa_binary_instr_kind _kind;
};

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_binary_instr_kind kind,
        ssa_value* lhs,
        ssa_value* rhs);

#define SSA_BINOP_LHS_INDEX 0
#define SSA_BINOP_RHS_INDEX 1

static inline struct _ssa_binop_instr* _ssa_get_binop(ssa_instr* self);
static inline const struct _ssa_binop_instr* _ssa_get_cbinop(const ssa_instr* self);

static inline ssa_binary_instr_kind ssa_get_binop_kind(const ssa_instr* self);
static inline ssa_value* ssa_get_binop_lhs(const ssa_instr* self);
static inline ssa_value* ssa_get_binop_rhs(const ssa_instr* self);

static inline void ssa_set_binop_kind(ssa_instr* self, ssa_binary_instr_kind kind);
static inline void ssa_set_binop_lhs(ssa_instr* self, ssa_value* lhs);
static inline void ssa_set_binop_rhs(ssa_instr* self, ssa_value* rhs);

struct _ssa_cast_instr
{
        struct _ssa_unary_instr _base;
};

extern ssa_instr* ssa_new_cast(ssa_context* context,
        ssa_id id, tree_type* type, ssa_value* operand);

static inline ssa_value* ssa_get_cast_operand(const ssa_instr* self);

static inline void ssa_set_cast_operand(ssa_instr* self, ssa_value* operand);

struct _ssa_call_instr
{
        struct _ssa_nary_instr _base;
};

#define SSA_CALLED_FUNC_INDEX 0
#define SSA_CALL_ARGS_BEGIN_INDEX 1

extern ssa_instr* ssa_new_call(ssa_context* context, ssa_id id, ssa_value* func);

extern void ssa_add_call_arg(ssa_instr* self, ssa_value* arg);
extern bool ssa_call_returns_void(const ssa_instr* self);

static inline ssa_value* ssa_get_called_func(const ssa_instr* self);
static inline ssa_value** ssa_get_call_args_begin(const ssa_instr* self);
static inline ssa_value** ssa_get_call_args_end(const ssa_instr* self);

static inline void ssa_set_called_func(ssa_instr* self, ssa_value* func);

#define SSA_FOREACH_CALL_ARG(PINSTR, ITNAME, ENDNAME)\
        for (ssa_value** ITNAME = ssa_get_call_args_begin(PINSTR),\
                **ENDNAME = ssa_get_call_args_end(PINSTR);\
                ITNAME != ENDNAME; ITNAME++)

struct _ssa_getaddr_instr
{
        struct _ssa_ternary_instr _base;
};

#define SSA_GETADDR_OPERAND_INDEX 0
#define SSA_GETADDR_INDEX_INDEX 1
#define SSA_GETADDR_OFFSET_INDEX 2

extern ssa_instr* ssa_new_getaddr(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_value* pointer,
        ssa_value* index,
        ssa_value* offset);

static inline ssa_value* ssa_get_getaddr_operand(const ssa_instr* self);
static inline ssa_value* ssa_get_getaddr_index(const ssa_instr* self);
static inline ssa_value* ssa_get_getaddr_offset(const ssa_instr* self);

static inline void ssa_set_getaddr_operand(ssa_instr* self, ssa_value* operand);
static inline void ssa_set_getaddr_index(ssa_instr* self, ssa_value* index);
static inline void ssa_set_getaddr_offset(ssa_instr* self, ssa_value* offset);

struct _ssa_phi_instr
{
        struct _ssa_nary_instr _base;
};

extern ssa_instr* ssa_new_phi(ssa_context* context, ssa_id id, tree_type* restype);

extern void ssa_add_phi_arg(ssa_instr* self, ssa_value* arg);

static inline ssa_value** ssa_get_phi_args_begin(const ssa_instr* self);
static inline ssa_value** ssa_get_phi_args_end(const ssa_instr* self);

#define SSA_FOREACH_PHI_ARG(PINSTR, ITNAME, ENDNAME) \
        SSA_FOREACH_NARY_INSTR_OPERAND(PINSTR, ITNAME, ENDNAME)

struct _ssa_alloca_instr
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_alloca(ssa_context* context, ssa_id id, tree_type* type);

static inline tree_type* ssa_get_allocated_type(const ssa_instr* self);

struct _ssa_load_instr
{
        struct _ssa_unary_instr _base;
};

extern ssa_instr* ssa_new_load(ssa_context* context,
        ssa_id id, tree_type* type, ssa_value* what);

static inline ssa_value* ssa_get_load_what(const ssa_instr* self);

static inline void ssa_set_load_what(ssa_instr* self, ssa_value* what);

struct _ssa_store_instr
{
        struct _ssa_binary_instr _base;
};

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where);

#define SSA_STORE_WHAT_INDEX 0
#define SSA_STORE_WHERE_INDEX 1

static inline ssa_value* ssa_get_store_what(const ssa_instr* self);
static inline ssa_value* ssa_get_store_where(const ssa_instr* self);

static inline void ssa_set_store_what(ssa_instr* self, ssa_value* what);
static inline void ssa_set_store_where(ssa_instr* self, ssa_value* where);

typedef struct _ssa_instr
{
        union
        {
                struct _ssa_binop_instr _binop;
                struct _ssa_cast_instr _cast;
                struct _ssa_call_instr _call;
                struct _ssa_getaddr_instr _getptrval;
                struct _ssa_phi_instr _phi;
                struct _ssa_alloca_instr _alloca;
                struct _ssa_store_instr _store;
                struct _ssa_load_instr _load;
        };
} ssa_instr;

#define SSA_ASSERT_INSTR_BASE(P) S_ASSERT(P)

static inline struct _ssa_instr_base* _ssa_get_instr_base(ssa_instr* self)
{
        SSA_ASSERT_INSTR_BASE(self);
        return (struct _ssa_instr_base*)self;
}

static inline const struct _ssa_instr_base* _ssa_get_instr_cbase(const ssa_instr* self)
{
        SSA_ASSERT_INSTR_BASE(self);
        return (const struct _ssa_instr_base*)self;
}

static inline ssa_instr* ssa_get_var_instr(ssa_value* self)
{
        S_ASSERT(ssa_get_value_kind(self) == SVK_VARIABLE);
        return (ssa_instr*)((suint8*)self - offsetof(struct _ssa_instr_base, _val));
}

static inline const ssa_instr* ssa_get_var_cinstr(const ssa_value* self)
{
        S_ASSERT(ssa_get_value_kind(self) == SVK_VARIABLE);
        return (const ssa_instr*)(
                (const suint8*)self - offsetof(struct _ssa_instr_base, _val));
}

static inline ssa_instr_kind ssa_get_instr_kind(const ssa_instr* self)
{
        return _ssa_get_instr_cbase(self)->_kind;
}

static inline ssa_value* ssa_get_instr_var(ssa_instr* self)
{
        return (ssa_value*)&_ssa_get_instr_base(self)->_val;
}

static inline const ssa_value* ssa_get_instr_cvar(const ssa_instr* self)
{
        return (const ssa_value*)&_ssa_get_instr_cbase(self)->_val;
}

static inline ssa_instr* ssa_get_next_instr(const ssa_instr* self)
{
        return (ssa_instr*)list_node_next(&_ssa_get_instr_cbase(self)->_node);
}

static inline ssa_instr* ssa_get_prev_instr(const ssa_instr* self)
{
        return (ssa_instr*)list_node_prev(&_ssa_get_instr_cbase(self)->_node);
}

static inline void ssa_set_instr_kind(ssa_instr* self, ssa_instr_kind kind)
{
        _ssa_get_instr_base(self)->_kind = kind;
}

static inline void ssa_remove_instr(ssa_instr* self)
{
        list_node_remove(&_ssa_get_instr_base(self)->_node);
}

static inline struct _ssa_unary_instr* _ssa_get_unary_instr(ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_unary(self));
        return (struct _ssa_unary_instr*)self;
}

static inline const struct _ssa_unary_instr* _ssa_get_unary_cinstr(const ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_unary(self));
        return (const struct _ssa_unary_instr*)self;
}

static inline ssa_value* ssa_get_unary_instr_operand(const ssa_instr* self)
{
        return _ssa_get_unary_cinstr(self)->_operand;
}

static inline ssa_value** ssa_get_unary_instr_poperand(ssa_instr* self)
{
        return &_ssa_get_unary_instr(self)->_operand;
}

static inline ssa_value* const* ssa_get_unary_instr_cpoperand(const ssa_instr* self)
{
        return &_ssa_get_unary_cinstr(self)->_operand;
}

static inline void ssa_set_unary_instr_operand(ssa_instr* self, ssa_value* value)
{
        _ssa_get_unary_instr(self)->_operand = value;
}

static inline struct _ssa_binary_instr* _ssa_get_binary_instr(ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_binary(self));
        return (struct _ssa_binary_instr*)self;
}

static inline const struct _ssa_binary_instr* _ssa_get_binary_cinstr(const ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_binary(self));
        return (const struct _ssa_binary_instr*)self;
}

static inline ssa_value* const* ssa_get_binary_instr_operands_cbegin(const ssa_instr* self)
{
        return _ssa_get_binary_cinstr(self)->_operands;
}

static inline ssa_value** ssa_get_binary_instr_operands_begin(ssa_instr* self)
{
        return _ssa_get_binary_instr(self)->_operands;
}

static inline ssa_value* const* ssa_get_binary_instr_operands_cend(const ssa_instr* self)
{
        return ssa_get_binary_instr_operands_cbegin(self) + 2;
}

static inline ssa_value** ssa_get_binary_instr_operands_end(ssa_instr* self)
{
        return ssa_get_binary_instr_operands_begin(self) + 2;
}

static inline struct _ssa_ternary_instr* _ssa_get_ternary_instr(ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_ternary(self));
        return (struct _ssa_ternary_instr*)self;
}

static inline const struct _ssa_ternary_instr* _ssa_get_ternary_cinstr(const ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_ternary(self));
        return (const struct _ssa_ternary_instr*)self;
}

static inline ssa_value* const* ssa_get_ternary_instr_operands_cbegin(const ssa_instr* self)
{
        return _ssa_get_ternary_cinstr(self)->_operands;
}

static inline ssa_value** ssa_get_ternary_instr_operands_begin(ssa_instr* self)
{
        return _ssa_get_ternary_instr(self)->_operands;
}

static inline ssa_value* const* ssa_get_ternary_instr_operands_cend(const ssa_instr* self)
{
        return ssa_get_ternary_instr_operands_cbegin(self) + 3;
}

static inline ssa_value** ssa_get_ternary_instr_operands_end(ssa_instr* self)
{
        return ssa_get_ternary_instr_operands_begin(self) + 3;
}

static inline struct _ssa_nary_instr* _ssa_get_nary_instr(ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_nary(self));
        return (struct _ssa_nary_instr*)self;
}

static inline const struct _ssa_nary_instr* _ssa_get_nary_cinstr(const ssa_instr* self)
{
        S_ASSERT(ssa_instr_is_nary(self));
        return (const struct _ssa_nary_instr*)self;
}

static inline ssa_value** ssa_get_nary_instr_operands_begin(const ssa_instr* self)
{
        return (ssa_value**)dseq_begin_ptr(&_ssa_get_nary_cinstr(self)->_operands);
}

static inline ssa_value** ssa_get_nary_instr_operands_end(const ssa_instr* self)
{
        return (ssa_value**)dseq_end_ptr(&_ssa_get_nary_cinstr(self)->_operands);
}

#define SSA_ASSERT_INSTR(P, K) S_ASSERT((P) && ssa_get_instr_kind(self) == (K))

static inline struct _ssa_binop_instr* _ssa_get_binop(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return (struct _ssa_binop_instr*)self;
}

static inline const struct _ssa_binop_instr* _ssa_get_cbinop(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return (const struct _ssa_binop_instr*)self;
}

static inline ssa_binary_instr_kind ssa_get_binop_kind(const ssa_instr* self)
{
        return _ssa_get_cbinop(self)->_kind;
}

static inline ssa_value* ssa_get_binop_lhs(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return ssa_get_binary_instr_operands_cbegin(self)[SSA_BINOP_LHS_INDEX];
}

static inline ssa_value* ssa_get_binop_rhs(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return ssa_get_binary_instr_operands_cbegin(self)[SSA_BINOP_RHS_INDEX];
}

static inline void ssa_set_binop_kind(ssa_instr* self, ssa_binary_instr_kind kind)
{
        _ssa_get_binop(self)->_kind = kind;
}

static inline void ssa_set_binop_lhs(ssa_instr* self, ssa_value* lhs)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        ssa_get_binary_instr_operands_begin(self)[SSA_BINOP_LHS_INDEX] = lhs;
}

static inline void ssa_set_binop_rhs(ssa_instr* self, ssa_value* rhs)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        ssa_get_binary_instr_operands_begin(self)[SSA_BINOP_RHS_INDEX] = rhs;
}

static inline ssa_value* ssa_get_cast_operand(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CAST);
        return ssa_get_unary_instr_operand(self);
}

static inline void ssa_set_cast_operand(ssa_instr* self, ssa_value* operand)
{
        SSA_ASSERT_INSTR(self, SIK_CAST);
        ssa_set_unary_instr_operand(self, operand);
}

static inline ssa_value* ssa_get_called_func(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        return ssa_get_nary_instr_operands_begin(self)[SSA_CALLED_FUNC_INDEX];
}

static inline ssa_value** ssa_get_call_args_begin(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        return ssa_get_nary_instr_operands_begin(self) + SSA_CALL_ARGS_BEGIN_INDEX;
}

static inline ssa_value** ssa_get_call_args_end(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        return ssa_get_nary_instr_operands_end(self);
}

static inline void ssa_set_called_func(ssa_instr* self, ssa_value* func)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        ssa_get_nary_instr_operands_begin(self)[SSA_CALLED_FUNC_INDEX] = func;
        if (!func || ssa_call_returns_void(self))
                return;

        tree_type* ft = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));

        ssa_set_value_type(ssa_get_instr_var(self), tree_get_function_type_result(ft));
}

static inline ssa_value* ssa_get_getaddr_operand(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        return ssa_get_ternary_instr_operands_cbegin(self)[SSA_GETADDR_OPERAND_INDEX];
}

static inline ssa_value* ssa_get_getaddr_index(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        return ssa_get_ternary_instr_operands_cbegin(self)[SSA_GETADDR_INDEX_INDEX];
}

static inline ssa_value* ssa_get_getaddr_offset(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        return ssa_get_ternary_instr_operands_cbegin(self)[SSA_GETADDR_OFFSET_INDEX];
}

static inline void ssa_set_getaddr_operand(ssa_instr* self, ssa_value* operand)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        ssa_get_ternary_instr_operands_begin(self)[SSA_GETADDR_OPERAND_INDEX] = operand;
}

static inline void ssa_set_getaddr_index(ssa_instr* self, ssa_value* index)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        ssa_get_ternary_instr_operands_begin(self)[SSA_GETADDR_INDEX_INDEX] = index;
}

static inline void ssa_set_getaddr_offset(ssa_instr* self, ssa_value* offset)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        ssa_get_ternary_instr_operands_begin(self)[SSA_GETADDR_OFFSET_INDEX] = offset;
}

static inline ssa_value** ssa_get_phi_args_begin(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_PHI);
        return ssa_get_nary_instr_operands_begin(self);
}

static inline ssa_value** ssa_get_phi_args_end(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_PHI);
        return ssa_get_nary_instr_operands_end(self);
}

static inline tree_type* ssa_get_allocated_type(const ssa_instr* self)
{
        return tree_get_pointer_target(ssa_get_value_type(ssa_get_instr_cvar(self)));
}

static inline ssa_value* ssa_get_load_what(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_LOAD);
        return ssa_get_unary_instr_operand(self);
}

static inline void ssa_set_load_what(ssa_instr* self, ssa_value* what)
{
        SSA_ASSERT_INSTR(self, SIK_LOAD);
        ssa_set_unary_instr_operand(self, what);
}

static inline ssa_value* ssa_get_store_what(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_STORE);
        return ssa_get_binary_instr_operands_cbegin(self)[SSA_STORE_WHAT_INDEX];
}

static inline ssa_value* ssa_get_store_where(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_STORE);
        return ssa_get_binary_instr_operands_cbegin(self)[SSA_STORE_WHERE_INDEX];
}

static inline void ssa_set_store_what(ssa_instr* self, ssa_value* what)
{
        SSA_ASSERT_INSTR(self, SIK_STORE);
        ssa_get_binary_instr_operands_begin(self)[SSA_STORE_WHAT_INDEX] = what;
}

static inline void ssa_set_store_where(ssa_instr* self, ssa_value* where)
{
        SSA_ASSERT_INSTR(self, SIK_STORE);
        ssa_get_binary_instr_operands_begin(self)[SSA_STORE_WHERE_INDEX] = where;
}

#ifdef __cplusplus
}
#endif

#endif