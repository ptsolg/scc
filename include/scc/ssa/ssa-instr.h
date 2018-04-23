#ifndef SSA_INSTR_H
#define SSA_INSTR_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-value.h"

typedef struct _tree_type tree_type;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_block ssa_block;
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
        SIK_GETFIELDADDR,
        SIK_CALL,
        SIK_PHI,
        SIK_TERMINATOR,
        SIK_ATOMIC_RMW,
        SIK_FENCE,
        SIK_ATOMIC_CMPXCHG,
        SIK_SIZE,
} ssa_instr_kind;

#define SSA_ASSERT_INSTR_KIND(K) assert((K) > SIK_INVALID && (K) < SIK_SIZE)

struct _ssa_instr_base
{
        list_node _node;
        ssa_instr_kind _kind;
        struct _ssa_variable _val;
        ssa_array _operands;
};

extern ssa_instr* ssa_new_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        size_t reserved_operands,
        size_t size);

extern ssa_instr* ssa_new_unary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* operand,
        size_t size);

extern ssa_instr* ssa_new_binary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* first,
        ssa_value* second,
        size_t size);

extern ssa_value_use* ssa_add_instr_operand(ssa_instr* self, ssa_context* context, ssa_value* value);
extern bool ssa_instr_has_var(const ssa_instr* self);
extern ssa_value* ssa_get_instr_operand_value(const ssa_instr* self, size_t i);
extern ssa_value_use* ssa_get_instr_operand(const ssa_instr* self, size_t i);
extern ssa_value_use* ssa_get_instr_operands_begin(const ssa_instr* self);
extern ssa_value_use* ssa_get_instr_operands_end(const ssa_instr* self);
extern size_t ssa_get_instr_operands_size(const ssa_instr* self);

extern void ssa_add_instr_after(ssa_instr* self, ssa_instr* pos);
extern void ssa_add_instr_before(ssa_instr* self, ssa_instr* pos);
extern void ssa_remove_instr(ssa_instr* self);
extern void ssa_set_instr_operand_value(ssa_instr* self, size_t i, ssa_value* val);

static inline struct _ssa_instr_base* _ssa_instr_base(ssa_instr* self);
static inline const struct _ssa_instr_base* _ssa_instr_cbase(const ssa_instr* self);

static inline ssa_instr* ssa_get_var_instr(ssa_value* self);
static inline const ssa_instr* ssa_get_var_cinstr(const ssa_value* self);

static inline ssa_instr_kind ssa_get_instr_kind(const ssa_instr* self);
static inline ssa_value* ssa_get_instr_var(ssa_instr* self);
static inline const ssa_value* ssa_get_instr_cvar(const ssa_instr* self);
static inline ssa_instr* ssa_get_next_instr(const ssa_instr* self);
static inline ssa_instr* ssa_get_prev_instr(const ssa_instr* self);

static inline void ssa_set_instr_kind(ssa_instr* self, ssa_instr_kind kind);

#define SSA_FOREACH_INSTR_OPERAND(PINSTR, ITNAME, ENDNAME) \
        for (ssa_value_use* ITNAME = ssa_get_instr_operands_begin(PINSTR),\
                *ENDNAME = ssa_get_instr_operands_end(PINSTR); \
                ITNAME != ENDNAME; ITNAME++)

struct _ssa_alloca
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_alloca(ssa_context* context, ssa_id id, tree_type* type);
extern tree_type* ssa_get_allocated_type(const ssa_instr* self);

struct _ssa_load
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_load(
        ssa_context* context, ssa_id id, tree_type* type, ssa_value* what);

struct _ssa_cast
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_cast(
        ssa_context* context, ssa_id id, tree_type* type, ssa_value* operand);

typedef enum
{
        SBIK_INVALID,

        SBIK_MUL,
        SBIK_DIV,
        SBIK_MOD,
        SBIK_ADD,
        SBIK_PTRADD,
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
} ssa_binop_kind;

struct _ssa_binop
{
        struct _ssa_instr_base _base;
        ssa_binop_kind _kind;
};

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_binop_kind kind,
        ssa_value* lhs,
        ssa_value* rhs);

static inline struct _ssa_binop* _ssa_binop(ssa_instr* self);
static inline const struct _ssa_binop* _ssa_cbinop(const ssa_instr* self);

static inline ssa_binop_kind ssa_get_binop_kind(const ssa_instr* self);
static inline void ssa_set_binop_kind(ssa_instr* self, ssa_binop_kind kind);

struct _ssa_store
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where);

struct _ssa_getfieldaddr
{
        struct _ssa_instr_base _base;
        unsigned _field_index;
};

extern ssa_instr* ssa_new_getfieldaddr(
        ssa_context* context,
        ssa_id id,
        tree_type* type,
        ssa_value* operand,
        unsigned field_index);

static inline struct _ssa_getfieldaddr* _ssa_getfieldaddr(ssa_instr* self);
static inline const struct _ssa_getfieldaddr* _ssa_cgetfieldaddr(const ssa_instr* self);

static inline unsigned ssa_get_getfieldaddr_index(const ssa_instr* self);
static inline void ssa_set_getfieldaddr_index(ssa_instr* self, unsigned i);

struct _ssa_call
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_call(ssa_context* context, ssa_id id, tree_type* type, ssa_value* func);
extern bool ssa_call_returns_void(const ssa_instr* self);

static inline ssa_value* ssa_get_called_func(const ssa_instr* self);

struct _ssa_phi
{
        struct _ssa_instr_base _base;
};

extern ssa_instr* ssa_new_phi(ssa_context* context, ssa_id id, tree_type* restype);

extern void ssa_add_phi_operand(
        ssa_instr* self, ssa_context* context, ssa_value* value, ssa_value* label);

typedef enum
{
        STIK_INVALID,
        STIK_INDERECT_JUMP,
        STIK_CONDITIONAL_JUMP,
        STIK_SWITCH,
        STIK_RETURN,
        STIK_SIZE,
} ssa_terminator_instr_kind;

struct _ssa_terminator_instr
{
        struct _ssa_instr_base _base;
        ssa_terminator_instr_kind _kind;
};

extern ssa_instr* ssa_new_terminator_instr(
        ssa_context* context,
        ssa_terminator_instr_kind kind,
        size_t reserved_operands,
        size_t size);

extern ssa_value_use* ssa_get_terminator_instr_successors_begin(const ssa_instr* self);

static inline struct _ssa_terminator_instr* _ssa_terminator_instr(ssa_instr* self);
static inline const struct _ssa_terminator_instr* _ssa_terminator_cinstr(const ssa_instr* self);

static inline ssa_terminator_instr_kind ssa_get_terminator_instr_kind(const ssa_instr* self);
static inline void ssa_set_terminator_instr_kind(ssa_instr* self, ssa_terminator_instr_kind kind);

#define SSA_FOREACH_TERMINATOR_SUCCESSOR(PINSTR, ITNAME, ENDNAME) \
        for (ssa_value_use* ITNAME = ssa_get_terminator_instr_successors_begin(PINSTR),\
                *ENDNAME = ssa_get_instr_operands_end(PINSTR);\
                ITNAME != ENDNAME; ITNAME++)

struct _ssa_inderect_jump
{
        struct _ssa_terminator_instr _base;
};

extern ssa_instr* ssa_new_inderect_jump(ssa_context* context, ssa_value* dest);

struct _ssa_conditional_jump
{
        struct _ssa_terminator_instr _base;
};

extern ssa_instr* ssa_new_conditional_jump(
        ssa_context* context, ssa_value* condition, ssa_value* on_true, ssa_value* on_false);

struct _ssa_switch_instr
{
        struct _ssa_terminator_instr _base;
};

extern ssa_instr* ssa_new_switch_instr(ssa_context* context, ssa_value* condition);
extern void ssa_add_switch_case(ssa_instr* self,
        ssa_context* context, ssa_value* value, ssa_value* label);

struct _ssa_ret_instr
{
        struct _ssa_terminator_instr _base;
};

extern ssa_instr* ssa_new_ret_void(ssa_context* context);
extern ssa_instr* ssa_new_ret(ssa_context* context, ssa_value* value);

typedef enum
{
        SARIK_INVALID,
        SARIK_ADD,
        SARIK_XCHG,
        SARIK_SIZE,
} ssa_atomic_rmw_instr_kind;

typedef enum
{
        SMK_UNORDERED,
        SMK_MONOTONIC,
        SMK_ACQUIRE,
        SMK_RELEASE,
        SMK_ACQ_REL,
        SMK_SEQ_CST,
} ssa_memorder_kind;

struct _ssa_atomic_rmw_instr
{
        struct _ssa_instr_base _base;
        ssa_atomic_rmw_instr_kind _kind;
        ssa_memorder_kind _ordering;
};

extern ssa_instr* ssa_new_atomic_rmw_instr(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_atomic_rmw_instr_kind kind,
        ssa_value* ptr,
        ssa_value* val,
        ssa_memorder_kind ordering);

static inline struct _ssa_atomic_rmw_instr* _ssa_atomic_rmw_instr(ssa_instr* self)
{
        return (struct _ssa_atomic_rmw_instr*)self;
}

static inline const struct _ssa_atomic_rmw_instr* _ssa_atomic_rmw_cinstr(const ssa_instr* self)
{
        return (const struct _ssa_atomic_rmw_instr*)self;
}

static inline ssa_atomic_rmw_instr_kind ssa_get_atomic_rmw_instr_kind(const ssa_instr* self)
{
        return _ssa_atomic_rmw_cinstr(self)->_kind;
}

static inline void ssa_set_atomic_rmw_instr_kind(ssa_instr* self, ssa_atomic_rmw_instr_kind kind)
{
        _ssa_atomic_rmw_instr(self)->_kind = kind;
}

static inline ssa_memorder_kind ssa_get_atomic_rmw_instr_ordering(const ssa_instr* self)
{
        return _ssa_atomic_rmw_cinstr(self)->_ordering;
}

static inline void ssa_set_atomic_rmw_instr_ordering(ssa_instr* self, ssa_memorder_kind ordering)
{
        _ssa_atomic_rmw_instr(self)->_ordering = ordering;
}

typedef enum
{
        SSK_NONE,
        SSK_SINGLE_THREAD,
} ssa_syncscope_kind;

struct _ssa_fence_instr
{
        struct _ssa_instr_base _base;
        ssa_syncscope_kind _syncscope;
        ssa_memorder_kind _ordering;
};

extern ssa_instr* ssa_new_fence_instr(
        ssa_context* context, ssa_syncscope_kind syncscope, ssa_memorder_kind ordering);

static inline struct _ssa_fence_instr* _ssa_fence_instr(ssa_instr* self)
{
        return (struct _ssa_fence_instr*)self;
}

static inline const struct _ssa_fence_instr* _ssa_fence_cinstr(const ssa_instr* self)
{
        return (const struct _ssa_fence_instr*)self;
}

static inline ssa_syncscope_kind ssa_get_fence_instr_syncscope(const ssa_instr* self)
{
        return _ssa_fence_cinstr(self)->_syncscope;
}

static inline void ssa_set_fence_instr_syncscope(ssa_instr* self, ssa_syncscope_kind syncscope)
{
        _ssa_fence_instr(self)->_syncscope = syncscope;
}

static inline ssa_memorder_kind ssa_get_fence_instr_ordering(const ssa_instr* self)
{
        return _ssa_fence_cinstr(self)->_ordering;
}

static inline void ssa_set_fence_instr_ordering(ssa_instr* self, ssa_memorder_kind ordering)
{
        _ssa_fence_instr(self)->_ordering = ordering;
}

struct _ssa_atomic_cmpxchg_instr
{
        struct _ssa_instr_base _base;
        ssa_memorder_kind _success_ordering;
        ssa_memorder_kind _failure_ordering;
};

extern ssa_instr* ssa_new_atomic_cmpxchg_instr(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_value* ptr,
        ssa_value* expected,
        ssa_value* desired,
        ssa_memorder_kind success_ordering,
        ssa_memorder_kind failure_ordering);

static inline struct _ssa_atomic_cmpxchg_instr* _ssa_atomic_cmpxchg_instr(ssa_instr* self)
{
        return (struct _ssa_atomic_cmpxchg_instr*)self;
}

static inline const struct _ssa_atomic_cmpxchg_instr* _ssa_atomic_cmpxchg_cinstr(const ssa_instr* self)
{
        return (const struct _ssa_atomic_cmpxchg_instr*)self;
}

static inline ssa_memorder_kind ssa_get_atomic_cmpxchg_instr_success_ordering(const ssa_instr* self)
{
        return _ssa_atomic_cmpxchg_cinstr(self)->_success_ordering;
}

static inline ssa_memorder_kind ssa_get_atomic_cmpxchg_instr_failure_ordering(const ssa_instr* self)
{
        return _ssa_atomic_cmpxchg_cinstr(self)->_failure_ordering;
}

static inline void ssa_set_atomic_cmpxchg_instr_success_ordering(ssa_instr* self, ssa_memorder_kind ordering)
{
        _ssa_atomic_cmpxchg_instr(self)->_success_ordering = ordering;
}

static inline void ssa_set_atomic_cmpxchg_instr_failure_ordering(ssa_instr* self, ssa_memorder_kind ordering)
{
        _ssa_atomic_cmpxchg_instr(self)->_failure_ordering = ordering;
}

typedef struct _ssa_instr
{
        union
        {
                struct _ssa_alloca _alloca;
                struct _ssa_load _load;
                struct _ssa_cast _cast;
                struct _ssa_binop _binop;
                struct _ssa_store _store;
                struct _ssa_getfieldaddr _getfieldaddr;
                struct _ssa_call _call;
                struct _ssa_phi _phi;
                struct _ssa_terminator_instr _terminator;
                struct _ssa_atomic_rmw_instr _atomic_rmw;
                struct _ssa_fence_instr _fence;
                struct _ssa_atomic_cmpxchg_instr _atomic_cmpxchg;
        };
} ssa_instr;

static inline struct _ssa_instr_base* _ssa_instr_base(ssa_instr* self)
{
        assert(self);
        return (struct _ssa_instr_base*)self;
}

static inline const struct _ssa_instr_base* _ssa_instr_cbase(const ssa_instr* self)
{
        assert(self);
        return (const struct _ssa_instr_base*)self;
}

static inline ssa_instr* ssa_get_var_instr(ssa_value* self)
{
        return (ssa_instr*)((uint8_t*)self - offsetof(struct _ssa_instr_base, _val));
}

static inline const ssa_instr* ssa_get_var_cinstr(const ssa_value* self)
{
        return (const ssa_instr*)(
                (const uint8_t*)self - offsetof(struct _ssa_instr_base, _val));
}

static inline ssa_instr_kind ssa_get_instr_kind(const ssa_instr* self)
{
        return _ssa_instr_cbase(self)->_kind;
}

static inline ssa_value* ssa_get_instr_var(ssa_instr* self)
{
        return (ssa_value*)&_ssa_instr_base(self)->_val;
}

static inline const ssa_value* ssa_get_instr_cvar(const ssa_instr* self)
{
        return (ssa_value*)&_ssa_instr_cbase(self)->_val;
}

static inline ssa_instr* ssa_get_next_instr(const ssa_instr* self)
{
        return (ssa_instr*)list_node_next(&_ssa_instr_cbase(self)->_node);
}

static inline ssa_instr* ssa_get_prev_instr(const ssa_instr* self)
{
        return (ssa_instr*)list_node_prev(&_ssa_instr_cbase(self)->_node);
}

static inline void ssa_set_instr_kind(ssa_instr* self, ssa_instr_kind kind)
{
        _ssa_instr_base(self)->_kind = kind;
}

#define SSA_ASSERT_INSTR(P, K) assert((P) && ssa_get_instr_kind(P) == (K))

static inline struct _ssa_binop* _ssa_binop(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return (struct _ssa_binop*)self;
}

static inline const struct _ssa_binop* _ssa_cbinop(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_BINARY);
        return (const struct _ssa_binop*)self;
}

static inline ssa_binop_kind ssa_get_binop_kind(const ssa_instr* self)
{
        return _ssa_cbinop(self)->_kind;
}

static inline void ssa_set_binop_kind(ssa_instr* self, ssa_binop_kind kind)
{
        _ssa_binop(self)->_kind = kind;
}

static inline struct _ssa_getfieldaddr* _ssa_getfieldaddr(ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETFIELDADDR);
        return (struct _ssa_getfieldaddr*)self;
}

static inline const struct _ssa_getfieldaddr* _ssa_cgetfieldaddr(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_GETFIELDADDR);
        return (const struct _ssa_getfieldaddr*)self;
}

static inline unsigned ssa_get_getfieldaddr_index(const ssa_instr* self)
{
        return _ssa_cgetfieldaddr(self)->_field_index;
}

static inline void ssa_set_getfieldaddr_index(ssa_instr* self, unsigned i)
{
        _ssa_getfieldaddr(self)->_field_index = i;
}

static inline ssa_value* ssa_get_called_func(const ssa_instr* self)
{
        return ssa_get_instr_operand_value(self, 0);
}

static inline struct _ssa_terminator_instr* _ssa_terminator_instr(ssa_instr* self)
{
        assert(self);
        return (struct _ssa_terminator_instr*)self;
}

static inline const struct _ssa_terminator_instr* _ssa_terminator_cinstr(const ssa_instr* self)
{
        assert(self);
        return (const struct _ssa_terminator_instr*)self;
}

static inline ssa_terminator_instr_kind ssa_get_terminator_instr_kind(const ssa_instr* self)
{
        SSA_ASSERT_INSTR(self, SIK_TERMINATOR);
        return _ssa_terminator_cinstr(self)->_kind;
}

static inline void ssa_set_terminator_instr_kind(ssa_instr* self, ssa_terminator_instr_kind kind)
{
        SSA_ASSERT_INSTR(self, SIK_TERMINATOR);
        _ssa_terminator_instr(self)->_kind = kind;
}

#ifdef __cplusplus
}
#endif

#endif