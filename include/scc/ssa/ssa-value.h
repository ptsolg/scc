#ifndef SSA_VALUE_H
#define SSA_VALUE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <scc/tree/tree.h>
#include "ssa-common.h"

typedef struct _tree_type tree_type;
typedef struct _tree_expr tree_expr;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_context ssa_context;

typedef enum
{
        SVK_INVALID,
        SVK_VARIABLE,
        SVK_CONSTANT,
} ssa_value_kind;

struct _ssa_value_base
{
        ssa_value_kind _kind;
        tree_type* _type;
};

extern ssa_value* ssa_new_value_base(
        ssa_context* context, tree_value_kind kind, tree_type* type, ssize size);

static inline struct _ssa_value_base* _ssa_get_value_base(ssa_value* self);
static inline const struct _ssa_value_base* _ssa_get_value_cbase(const ssa_value* self);

static inline ssa_value_kind ssa_get_value_kind(const ssa_value* self);
static inline tree_type* ssa_get_value_type(const ssa_value* self);

static inline void ssa_set_value_kind(ssa_value* self, ssa_value_kind kind);
static inline void ssa_set_value_type(ssa_value* self, tree_type* type);

struct _ssa_constant
{
        struct _ssa_value_base _base;
        avalue _val;
};

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* type, avalue val);

static inline struct _ssa_constant* _ssa_get_constant(ssa_value* self);
static inline const struct _ssa_constant* _ssa_get_cconstant(const ssa_value* self);

static inline avalue ssa_get_constant_value(const ssa_value* self);

static inline void ssa_set_constant_value(ssa_value* self, avalue val);

struct _ssa_var
{
        struct _ssa_value_base _base;
        ssa_id _id;
        ssa_instr* _init;
};

extern ssa_value* ssa_new_var(
        ssa_context* context, tree_type* type, ssa_id id, ssa_instr* init);

static inline struct _ssa_var* _ssa_get_var(ssa_value* self);
static inline const struct _ssa_var* _ssa_get_cvar(const ssa_value* self);

static inline ssa_id ssa_get_var_id(const ssa_value* self);
static inline ssa_instr* ssa_get_var_init(const ssa_value* self);

static inline void ssa_set_var_id(ssa_value* self, ssa_id id);
static inline void ssa_set_var_init(ssa_value* self, ssa_instr* init);

typedef struct _ssa_value
{
        union
        {
                struct _ssa_constant _const;
                struct _ssa_var _var;
        };
} ssa_value;

#define SSA_ASSERT_VALUE_BASE(P) S_ASSERT(P)

static inline struct _ssa_value_base* _ssa_get_value_base(ssa_value* self)
{
        SSA_ASSERT_VALUE_BASE(self);
        return (struct _ssa_value_base*)self;
}

static inline const struct _ssa_value_base* _ssa_get_value_cbase(const ssa_value* self)
{
        SSA_ASSERT_VALUE_BASE(self);
        return (const struct _ssa_value_base*)self;
}

static inline ssa_value_kind ssa_get_value_kind(const ssa_value* self)
{
        return _ssa_get_value_cbase(self)->_kind;
}

static inline tree_type* ssa_get_value_type(const ssa_value* self)
{
        return _ssa_get_value_cbase(self)->_type;
}

static inline void ssa_set_value_kind(ssa_value* self, ssa_value_kind kind)
{
        _ssa_get_value_base(self)->_kind = kind;
}

static inline void ssa_set_value_type(ssa_value* self, tree_type* type)
{
        _ssa_get_value_base(self)->_type = type;
}

#define SSA_ASSERT_VALUE(P, K) S_ASSERT((P) && ssa_get_value_kind(P) == (K))

static inline struct _ssa_constant* _ssa_get_constant(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_CONSTANT);
        return (struct _ssa_constant*)self;
}

static inline const struct _ssa_constant* _ssa_get_cconstant(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_CONSTANT);
        return (const struct _ssa_constant*)self;
}

static inline avalue ssa_get_constant_value(const ssa_value* self)
{
        return _ssa_get_cconstant(self)->_val;
}

static inline void ssa_set_constant_value(ssa_value* self, avalue val)
{
        _ssa_get_constant(self)->_val = val;
}

static inline struct _ssa_var* _ssa_get_var(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_VARIABLE);
        return (struct _ssa_var*)self;
}

static inline const struct _ssa_var* _ssa_get_cvar(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_VARIABLE);
        return (const struct _ssa_var*)self;
}

static inline ssa_id ssa_get_var_id(const ssa_value* self)
{
        return _ssa_get_cvar(self)->_id;
}

static inline ssa_instr* ssa_get_var_init(const ssa_value* self)
{
        return _ssa_get_cvar(self)->_init;
}

static inline void ssa_set_var_id(ssa_value* self, ssa_id id)
{
        _ssa_get_var(self)->_id = id;
}

static inline void ssa_set_var_init(ssa_value* self, ssa_instr* init)
{
        _ssa_get_var(self)->_init = init;
}

#ifdef __cplusplus
}
#endif

#endif