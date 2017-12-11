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

        SIK_BINARY,
        SIK_CAST,
        SIK_CALL,
        SIK_GETADDR,
        SIK_PHI,
        SIK_ALLOCA,
        SIK_LOAD,
        SIK_STORE,

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

extern bool ssa_instr_has_var(const ssa_instr* self);

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

struct _ssa_binary_instr
{
        struct _ssa_instr_base _base;
        ssa_binary_instr_kind _opcode;
        ssa_value* _lhs;
        ssa_value* _rhs;
};

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_binary_instr_kind opcode,
        ssa_value* lhs,
        ssa_value* rhs);

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
        ssa_value* _operand;
};

extern ssa_instr* ssa_new_cast(ssa_context* context, ssa_id id, tree_type* type, ssa_value* operand);

static inline struct _ssa_cast_instr* _ssa_get_cast(ssa_instr* self);
static inline const struct _ssa_cast_instr* _ssa_get_ccast(const ssa_instr* self);

static inline ssa_value* ssa_get_cast_operand(const ssa_instr* self);

static inline void ssa_set_cast_operand(ssa_instr* self, ssa_value* operand);

struct _ssa_call_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _func;
        dseq _args;
};

extern ssa_instr* ssa_new_call(ssa_context* context, ssa_id id, ssa_value* func);

extern void ssa_set_call_args(ssa_instr* self, dseq* args);
extern void ssa_add_call_arg(ssa_instr* self, ssa_value* arg);
extern bool ssa_call_returns_void(const ssa_instr* self);

static inline struct _ssa_call_instr* _ssa_get_call(ssa_instr* self);
static inline const struct _ssa_call_instr* _ssa_get_ccall(const ssa_instr* self);

static inline ssa_value* ssa_get_called_func(const ssa_instr* self);
static inline ssa_value** ssa_get_call_args_begin(const ssa_instr* self);
static inline ssa_value** ssa_get_call_args_end(const ssa_instr* self);

static inline void ssa_set_called_func(ssa_instr* self, ssa_value* func);

#define SSA_FOREACH_CALL_ARG(PCALL, ITNAME) \
        for (ssa_value** ITNAME = ssa_get_call_args_begin(PCALL); \
                ITNAME != ssa_get_call_args_end(PCALL); ITNAME++)

struct _ssa_getaddr_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _ptr;
        ssa_value* _index;
        ssa_value* _offset;
};

extern ssa_instr* ssa_new_getaddr(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_value* pointer,
        ssa_value* index,
        ssa_value* offset);

static inline struct _ssa_getaddr_instr* _ssa_get_getaddr(ssa_instr* self);
static inline const struct _ssa_getaddr_instr* _ssa_get_cgetaddr(const ssa_instr* self);

static inline ssa_value* ssa_get_getaddr_operand(const ssa_instr* self);
static inline ssa_value* ssa_get_getaddr_index(const ssa_instr* self);
static inline ssa_value* ssa_get_getaddr_offset(const ssa_instr* self);

static inline void ssa_set_getaddr_operand(ssa_instr* self, ssa_value* operand);
static inline void ssa_set_getaddr_index(ssa_instr* self, ssa_value* index);
static inline void ssa_set_getaddr_offset(ssa_instr* self, ssa_value* offset);

struct _ssa_phi_instr
{
        struct _ssa_instr_base _base;
        dseq _args;
};

extern ssa_instr* ssa_new_phi(ssa_context* context, ssa_id id, tree_type* restype);

extern void ssa_add_phi_arg(ssa_instr* self, ssa_value* arg);

static inline struct _ssa_phi_instr* _ssa_get_phi(ssa_instr* self);
static inline const struct _ssa_phi_instr* _ssa_get_cphi(const ssa_instr* self);

static inline ssa_value** ssa_get_phi_args_begin(const ssa_instr* self);
static inline ssa_value** ssa_get_phi_args_end(const ssa_instr* self);

#define SSA_FOREACH_PHI_ARG(PPHI, ITNAME)\
        for (ssa_value** ITNAME = ssa_get_phi_args_begin(PPHI);\
                ITNAME != ssa_get_phi_args_end(PPHI); ITNAME++)

struct _ssa_alloca_instr
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_alloca(ssa_context* context, ssa_id id, tree_type* type);

static inline tree_type* ssa_get_allocated_type(const ssa_instr* self);

struct _ssa_load_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _what;
};

extern ssa_instr* ssa_new_load(ssa_context* context,
        ssa_id id, tree_type* type, ssa_value* what);

static inline struct _ssa_load_instr* _ssa_get_load(ssa_instr* self);
static inline const struct _ssa_load_instr* _ssa_get_cload(const ssa_instr* self);

static inline ssa_value* ssa_get_load_what(const ssa_instr* self);

static inline void ssa_set_load_what(ssa_instr* self, ssa_value* what);

struct _ssa_store_instr
{
        struct _ssa_instr_base _base;
        ssa_value* _what;
        ssa_value* _where;
};

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where);

static inline struct _ssa_store_instr* _ssa_get_store(ssa_instr* self);
static inline const struct _ssa_store_instr* _ssa_get_cstore(const ssa_instr* self);

static inline ssa_value* ssa_get_store_what(const ssa_instr* self);
static inline ssa_value* ssa_get_store_where(const ssa_instr* self);

static inline void ssa_set_store_what(ssa_instr* self, ssa_value* what);
static inline void ssa_set_store_where(ssa_instr* self, ssa_value* where);

typedef struct _ssa_instr
{
        union
        {
                struct _ssa_binary_instr _binary;
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

static inline ssa_value* ssa_get_cast_operand(const ssa_instr* self)
{
        return _ssa_get_ccast(self)->_operand;
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

static inline ssa_value* ssa_get_called_func(const ssa_instr* self)
{
        return _ssa_get_ccall(self)->_func;
}

static inline ssa_value** ssa_get_call_args_begin(const ssa_instr* self)
{
        return (ssa_value**)dseq_begin_ptr(&_ssa_get_ccall(self)->_args);
}

static inline ssa_value** ssa_get_call_args_end(const ssa_instr* self)
{
        return (ssa_value**)dseq_end_ptr(&_ssa_get_ccall(self)->_args);
}

static inline void ssa_set_called_func(ssa_instr* self, ssa_value* func)
{
        _ssa_get_call(self)->_func = func;
        if (!func || ssa_call_returns_void(self))
                return;

        tree_type* ft = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));

        ssa_set_value_type(ssa_get_instr_var(self), tree_get_function_type_result(ft));
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
        return _ssa_get_cgetaddr(self)->_ptr;
}

static inline ssa_value* ssa_get_getaddr_index(const ssa_instr* self)
{
        return _ssa_get_cgetaddr(self)->_index;
}

static inline ssa_value* ssa_get_getaddr_offset(const ssa_instr* self)
{
        return _ssa_get_cgetaddr(self)->_offset;
}

static inline void ssa_set_getaddr_operand(ssa_instr* self, ssa_value* operand)
{
        _ssa_get_getaddr(self)->_ptr = operand;
}

static inline void ssa_set_getaddr_index(ssa_instr* self, ssa_value* index)
{
        _ssa_get_getaddr(self)->_index = index;
}

static inline void ssa_set_getaddr_offset(ssa_instr* self, ssa_value* offset)
{
        _ssa_get_getaddr(self)->_offset = offset;
}

static inline struct _ssa_phi_instr* _ssa_get_phi(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_PHI);
        return (struct _ssa_phi_instr*)self;
}

static inline const struct _ssa_phi_instr* _ssa_get_cphi(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_PHI);
        return (const struct _ssa_phi_instr*)self;
}

static inline ssa_value** ssa_get_phi_args_begin(const ssa_instr* self)
{
        return (ssa_value**)dseq_begin_ptr(&_ssa_get_cphi(self)->_args);
}

static inline ssa_value** ssa_get_phi_args_end(const ssa_instr* self)
{
        return (ssa_value**)dseq_end_ptr(&_ssa_get_cphi(self)->_args);
}

static inline tree_type* ssa_get_allocated_type(const ssa_instr* self)
{
        return tree_get_pointer_target(ssa_get_value_type(ssa_get_instr_cvar(self)));
}

static inline struct _ssa_load_instr* _ssa_get_load(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_LOAD);
        return (struct _ssa_load_instr*)self;
}

static inline const struct _ssa_load_instr* _ssa_get_cload(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_LOAD);
        return (const struct _ssa_load_instr*)self;
}

static inline ssa_value* ssa_get_load_what(const ssa_instr* self)
{
        return _ssa_get_cload(self)->_what;
}

static inline void ssa_set_load_what(ssa_instr* self, ssa_value* what)
{
        _ssa_get_load(self)->_what = what;
}

static inline struct _ssa_store_instr* _ssa_get_store(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_STORE);
        return (struct _ssa_store_instr*)self;
}

static inline const struct _ssa_store_instr* _ssa_get_cstore(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_STORE);
        return (const struct _ssa_store_instr*)self;
}

static inline ssa_value* ssa_get_store_what(const ssa_instr* self)
{
        return _ssa_get_cstore(self)->_what;
}

static inline ssa_value* ssa_get_store_where(const ssa_instr* self)
{
        return _ssa_get_cstore(self)->_where;
}

static inline void ssa_set_store_what(ssa_instr* self, ssa_value* what)
{
        _ssa_get_store(self)->_what = what;
}

static inline void ssa_set_store_where(ssa_instr* self, ssa_value* where)
{
        _ssa_get_store(self)->_where = where;
}

#ifdef __cplusplus
}
#endif

#endif