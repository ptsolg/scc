#ifndef SSA_INSTR_H
#define SSA_INSTR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"

typedef struct _tree_type tree_type;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_context ssa_context;
typedef struct _tree_decl tree_decl;

typedef enum
{
        SIK_INVALID,

        SIK_BINARY,
        SIK_CAST,
        SIK_CALL,
        SIK_GETADDR,
        SIK_GETPTRVAL,
        SIK_PHI,
        SIK_INIT,

        SIK_SIZE,
} ssa_instr_kind;

#define SSA_CHECK_INSTR_KIND(K) S_ASSERT((K) > SIK_INVALID && (K) < SIK_SIZE)

struct _ssa_instr_base
{
        ssa_instr_kind _kind;
};

extern ssa_instr* ssa_new_instr(ssa_context* context, ssa_instr_kind kind, ssize size);

static inline struct _ssa_instr_base* _ssa_get_instr_base(ssa_instr* self);
static inline const struct _ssa_instr_base* _ssa_get_instr_cbase(const ssa_instr* self);

static inline ssa_instr_kind ssa_get_instr_kind(const ssa_instr* self);
static inline void ssa_set_instr_kind(ssa_instr* self, ssa_instr_kind kind);

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

#define SSA_CHECK_BINARY_INSTR_KIND(K) S_ASSERT((K) > SBIK_INVALID && (K) < SBIK_SIZE)

struct _ssa_binary_instr
{
        struct _ssa_instr_base _base;
        ssa_binary_instr_kind _opcode;
        ssa_value* _lhs;
        ssa_value* _rhs;
};

extern ssa_instr* ssa_new_binop(
        ssa_context* context, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs);

static inline struct _ssa_binary_instr* _ssa_get_binop(ssa_instr* self);
static inline const struct _ssa_binary_instr* _ssa_get_cbinop(const ssa_instr* self);

static inline ssa_binary_instr_kind ssa_get_binop_opcode(const ssa_instr* self);
static inline ssa_value* ssa_get_binop_lhs(const ssa_instr* self);
static inline ssa_value* ssa_get_binop_rhs(const ssa_instr* self);

static inline void ssa_set_binop_opcode(ssa_instr* self, ssa_binary_instr_kind opcode);
static inline void ssa_set_binop_lhs(ssa_instr* self, ssa_value* lhs);
static inline void ssa_set_binop_rhs(ssa_instr* self, ssa_value* rhs);

struct _ssa_cast_instr
{
        struct _ssa_instr_base _base;
        tree_type* _type;
        ssa_value* _operand;
};

extern ssa_instr* ssa_new_cast(ssa_context* context, tree_type* type, ssa_value* operand);

static inline struct _ssa_cast_instr* _ssa_get_cast(ssa_instr* self);
static inline const struct _ssa_cast_instr* _ssa_get_ccast(const ssa_instr* self);

static inline tree_type* ssa_get_cast_type(const ssa_instr* self);
static inline ssa_value* ssa_get_cast_operand(const ssa_instr* self);

static inline void ssa_set_cast_type(ssa_instr* self, tree_type* type);
static inline void ssa_set_cast_operand(ssa_instr* self, ssa_value* operand);

struct _ssa_call_instr
{
        struct _ssa_instr_base _base;
        tree_decl* _func;
        dseq _args;
};

extern ssa_instr* ssa_new_call(ssa_context* context, tree_decl* func);

static inline struct _ssa_call_instr* _ssa_get_call(ssa_instr* self);
static inline const struct _ssa_call_instr* _ssa_get_ccall(const ssa_instr* self);

static inline tree_decl* ssa_get_call_func(const ssa_instr* self);

static inline void ssa_set_call_func(ssa_instr* self, tree_decl* func);

struct _ssa_getaddr_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _operand;
        ssize _offset;
};

extern ssa_instr* ssa_new_getaddr(
        ssa_context* context, ssa_value* operand, ssize offset);

static inline struct _ssa_getaddr_instr* _ssa_get_getaddr(ssa_instr* self);
static inline const struct _ssa_getaddr_instr* _ssa_get_cgetaddr(const ssa_instr* self);

static inline ssa_value* ssa_get_getaddr_operand(const ssa_instr* self);
static inline ssize ssa_get_getaddr_offset(const ssa_instr* self);

static inline void ssa_set_getaddr_operand(ssa_instr* self, ssa_value* operand);
static inline void ssa_set_getaddr_offset(ssa_instr* self, ssize offset);

struct _ssa_getptrval_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _ptr;
        ssize _index;
        ssize _offset;
};

extern ssa_instr* ssa_new_getptrval(
        ssa_context* context, ssa_value* pointer, ssize index, ssize offset);

static inline struct _ssa_getptrval_instr* _ssa_get_getptrval(ssa_instr* self);
static inline const struct _ssa_getptrval_instr* _ssa_get_cgetptrval(const ssa_instr* self);

static inline ssa_value* ssa_get_getptrval_operand(const ssa_instr* self);
static inline ssize ssa_get_getptrval_index(const ssa_instr* self);
static inline ssize ssa_get_getptrval_offset(const ssa_instr* self);

static inline void ssa_set_getptrval_operand(ssa_instr* self, ssa_value* operand);
static inline void ssa_set_getptrval_index(ssa_instr* self, ssize index);
static inline void ssa_set_getptrval_offset(ssa_instr* self, ssize offset);

struct _ssa_phi_instr
{
        struct _ssa_instr_base _base;
        // todo
};

extern ssa_instr* ssa_new_phi(ssa_context* context);

struct _ssa_init_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _val;
};

extern ssa_instr* ssa_new_init(ssa_context* context, ssa_value* operand);

static inline struct _ssa_init_instr* _ssa_get_init(ssa_instr* self);
static inline const struct _ssa_init_instr* _ssa_get_cinit(const ssa_instr* self);

static inline ssa_value* ssa_get_init_value(const ssa_instr* self);

static inline void ssa_set_init_value(ssa_instr* self, ssa_value* val);

typedef struct _ssa_instr
{
        union
        {
                struct _ssa_binary_instr _binary;
                struct _ssa_cast_instr _cast;
                struct _ssa_call_instr _call;
                struct _ssa_getaddr_instr _getaddr;
                struct _ssa_getptrval_instr _getptrval;
                struct _ssa_phi_instr _phi;
                struct _ssa_init_instr _init;
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

static inline ssa_instr_kind ssa_get_instr_kind(const ssa_instr* self)
{
        return _ssa_get_instr_cbase(self)->_kind;
}

static inline void ssa_set_instr_kind(ssa_instr* self, ssa_instr_kind kind)
{
        _ssa_get_instr_base(self)->_kind = kind;
}

#define SSA_ASSERT_INSTR(P, K) S_ASSERT((P) && ssa_get_instr_kind(self) == (K))

static inline struct _ssa_binary_instr* _ssa_get_binop(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return (struct _ssa_binary_instr*)self;
}

static inline const struct _ssa_binary_instr* _ssa_get_cbinop(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return (const struct _ssa_binary_instr*)self;
}

static inline ssa_binary_instr_kind ssa_get_binop_opcode(const ssa_instr* self)
{
        return _ssa_get_cbinop(self)->_opcode;
}

static inline ssa_value* ssa_get_binop_lhs(const ssa_instr* self)
{
        return _ssa_get_cbinop(self)->_lhs;
}

static inline ssa_value* ssa_get_binop_rhs(const ssa_instr* self)
{
        return _ssa_get_cbinop(self)->_rhs;
}

static inline void ssa_set_binop_opcode(ssa_instr* self, ssa_binary_instr_kind opcode)
{
        _ssa_get_binop(self)->_opcode = opcode;
}

static inline void ssa_set_binop_lhs(ssa_instr* self, ssa_value* lhs)
{
        _ssa_get_binop(self)->_lhs = lhs;
}

static inline void ssa_set_binop_rhs(ssa_instr* self, ssa_value* rhs)
{
        _ssa_get_binop(self)->_rhs = rhs;
}

static inline struct _ssa_cast_instr* _ssa_get_cast(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CAST);
        return (struct _ssa_cast_instr*)self;
}

static inline const struct _ssa_cast_instr* _ssa_get_ccast(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CAST);
        return (const struct _ssa_cast_instr*)self;
}

static inline tree_type* ssa_get_cast_type(const ssa_instr* self)
{
        return _ssa_get_ccast(self)->_type;
}

static inline ssa_value* ssa_get_cast_operand(const ssa_instr* self)
{
        return _ssa_get_ccast(self)->_operand;
}

static inline void ssa_set_cast_type(ssa_instr* self, tree_type* type)
{
        _ssa_get_cast(self)->_type = type;
}

static inline void ssa_set_cast_operand(ssa_instr* self, ssa_value* operand)
{
        _ssa_get_cast(self)->_operand = operand;
}

static inline struct _ssa_call_instr* _ssa_get_call(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        return (struct _ssa_call_instr*)self;
}

static inline const struct _ssa_call_instr* _ssa_get_ccall(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        return (const struct _ssa_call_instr*)self;
}

static inline tree_decl* ssa_get_call_func(const ssa_instr* self)
{
        return _ssa_get_ccall(self)->_func;
}

static inline void ssa_set_call_func(ssa_instr* self, tree_decl* func)
{
        _ssa_get_call(self)->_func = func;
}

static inline struct _ssa_getaddr_instr* _ssa_get_getaddr(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        return (struct _ssa_getaddr_instr*)self;
}

static inline const struct _ssa_getaddr_instr* _ssa_get_cgetaddr(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETADDR);
        return (const struct _ssa_getaddr_instr*)self;
}

static inline ssa_value* ssa_get_getaddr_operand(const ssa_instr* self)
{
        return _ssa_get_cgetaddr(self)->_operand;
}

static inline ssize ssa_get_getaddr_offset(const ssa_instr* self)
{
        return _ssa_get_cgetaddr(self)->_offset;
}

static inline void ssa_set_getaddr_operand(ssa_instr* self, ssa_value* operand)
{
        _ssa_get_getaddr(self)->_operand = operand;
}

static inline void ssa_set_getaddr_offset(ssa_instr* self, ssize offset)
{
        _ssa_get_getaddr(self)->_offset = offset;
}

static inline struct _ssa_getptrval_instr* _ssa_get_getptrval(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETPTRVAL);
        return (struct _ssa_getptrval_instr*)self;
}

static inline const struct _ssa_getptrval_instr* _ssa_get_cgetptrval(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETPTRVAL);
        return (const struct _ssa_getptrval_instr*)self;
}

static inline ssa_value* ssa_get_getptrval_operand(const ssa_instr* self)
{
        return _ssa_get_cgetptrval(self)->_ptr;
}

static inline ssize ssa_get_getptrval_index(const ssa_instr* self)
{
        return _ssa_get_cgetptrval(self)->_index;
}

static inline ssize ssa_get_getptrval_offset(const ssa_instr* self)
{
        return _ssa_get_cgetptrval(self)->_offset;
}

static inline void ssa_set_getptrval_operand(ssa_instr* self, ssa_value* operand)
{
        _ssa_get_getptrval(self)->_ptr = operand;
}

static inline void ssa_set_getptrval_index(ssa_instr* self, ssize index)
{
        _ssa_get_getptrval(self)->_index = index;
}

static inline void ssa_set_getptrval_offset(ssa_instr* self, ssize offset)
{
        _ssa_get_getptrval(self)->_offset = offset;
}

static inline struct _ssa_init_instr* _ssa_get_init(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_INIT);
        return (struct _ssa_init_instr*)self;
}

static inline const struct _ssa_init_instr* _ssa_get_cinit(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_INIT);
        return (const struct _ssa_init_instr*)self;
}

static inline ssa_value* ssa_get_init_value(const ssa_instr* self)
{
        return _ssa_get_cinit(self)->_val;
}

static inline void ssa_set_init_value(ssa_instr* self, ssa_value* val)
{
        _ssa_get_init(self)->_val = val;
}

#ifdef __cplusplus
}
#endif

#endif