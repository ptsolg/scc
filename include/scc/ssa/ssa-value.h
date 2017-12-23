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
        SVK_LABEL,
        SVK_DECL,
        SVK_PARAM,
        SVK_STRING,
        SVK_NULL,
} ssa_value_kind;

struct _ssa_value_base
{
        ssa_value_kind _kind;
        ssa_id _id;
};

extern void ssa_init_value_base(ssa_value* self, ssa_value_kind k, ssa_id id);

static inline struct _ssa_value_base* _ssa_get_value_base(ssa_value* self);
static inline const struct _ssa_value_base* _ssa_get_value_cbase(const ssa_value* self);

static inline ssa_value_kind ssa_get_value_kind(const ssa_value* self);
static inline ssa_id ssa_get_value_id(const ssa_value* self);

static inline void ssa_set_value_kind(ssa_value* self, ssa_value_kind kind);
static inline void ssa_set_value_id(ssa_value* self, ssa_id id);

typedef struct _ssa_label
{
        struct _ssa_value_base _base;
} ssa_label;

extern void ssa_init_label(ssa_value* self, ssa_id id);

struct _ssa_typed_value_base
{
        struct _ssa_value_base _base;
        tree_type* _type;
};

extern void ssa_init_typed_value(ssa_value* self, ssa_value_kind k, ssa_id id, tree_type* t);
extern bool ssa_value_has_type(const ssa_value* self);

static inline struct _ssa_typed_value_base* _ssa_get_typed_value_base(ssa_value* self);
static inline const struct _ssa_typed_value_base* _ssa_get_typed_value_cbase(const ssa_value* self);

static inline tree_type* ssa_get_value_type(const ssa_value* self);

static inline void ssa_set_value_type(ssa_value* self, tree_type* type);

typedef struct _ssa_variable
{
        struct _ssa_typed_value_base _base;
} ssa_variable;

extern void ssa_init_variable(ssa_value* self, ssa_id id, tree_type* t);

typedef struct _ssa_constant
{
        struct _ssa_typed_value_base _base;
        avalue _val;
} ssa_constant;

extern void ssa_init_constant(ssa_value* self, tree_type* t, avalue val);

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* t, avalue val);

static inline ssa_constant* _ssa_get_constant(ssa_value* self);
static inline const ssa_constant* _ssa_get_cconstant(const ssa_value* self);

static inline avalue ssa_get_constant_value(const ssa_value* self);

static inline void ssa_set_constant_value(ssa_value* self, avalue val);

typedef struct _ssa_decl
{
        struct _ssa_typed_value_base _base;
        tree_decl* _entity;
} ssa_decl;

extern ssa_value* ssa_new_decl(ssa_context* context, tree_type* type, tree_decl* entity);

static inline ssa_decl* _ssa_get_decl(ssa_value* self);
static inline const ssa_decl* _ssa_get_cdecl(const ssa_value* self);

static inline tree_decl* ssa_get_decl_entity(const ssa_value* self);

static inline void ssa_set_decl_entity(ssa_value* self, tree_decl* entity);

typedef struct _ssa_param
{
        struct _ssa_typed_value_base _base;
} ssa_param;

extern ssa_value* ssa_new_param(ssa_context* context, ssa_id id, tree_type* type);

typedef struct _ssa_string
{
        struct _ssa_typed_value_base _base;
        tree_id _id;
} ssa_string;

extern ssa_value* ssa_new_string(ssa_context* context, ssa_id uid, tree_type* type, tree_id id);

static inline ssa_string* _ssa_get_string(ssa_value* self);
static inline const ssa_string* _ssa_get_cstring(const ssa_value* self);

static inline tree_id ssa_get_string_value(const ssa_value* self);

static inline void ssa_set_string_value(ssa_value* self, tree_id id);

typedef struct _ssa_null_pointer
{
        struct _ssa_typed_value_base _base;
} ssa_null_pointer;

extern ssa_value* ssa_new_null_pointer(ssa_context* context, tree_type* type);

typedef struct _ssa_value
{
        union
        {
                ssa_label _label;
                ssa_variable _var;
                ssa_constant _const;
                ssa_decl _decl;
                ssa_null_pointer _null;
        };
} ssa_value;

static inline struct _ssa_value_base* _ssa_get_value_base(ssa_value* self)
{
        S_ASSERT(self);
        return (struct _ssa_value_base*)self;
}

static inline const struct _ssa_value_base* _ssa_get_value_cbase(const ssa_value* self)
{
        S_ASSERT(self);
        return (const struct _ssa_value_base*)self;
}

static inline ssa_value_kind ssa_get_value_kind(const ssa_value* self)
{
        return _ssa_get_value_cbase(self)->_kind;
}

static inline ssa_id ssa_get_value_id(const ssa_value* self)
{
        return _ssa_get_value_cbase(self)->_id;
}

static inline void ssa_set_value_kind(ssa_value* self, ssa_value_kind kind)
{
        _ssa_get_value_base(self)->_kind = kind;
}

static inline void ssa_set_value_id(ssa_value* self, ssa_id id)
{
        _ssa_get_value_base(self)->_id = id;
}

static inline struct _ssa_typed_value_base* _ssa_get_typed_value_base(ssa_value* self)
{
        S_ASSERT(ssa_value_has_type(self));
        return (struct _ssa_typed_value_base*)self;
}

static inline const struct _ssa_typed_value_base* _ssa_get_typed_value_cbase(const ssa_value* self)
{
        S_ASSERT(ssa_value_has_type(self));
        return (const struct _ssa_typed_value_base*)self;
}

static inline tree_type* ssa_get_value_type(const ssa_value* self)
{
        return _ssa_get_typed_value_cbase(self)->_type;
}

static inline void ssa_set_value_type(ssa_value* self, tree_type* type)
{
        _ssa_get_typed_value_base(self)->_type = type;
}

#define SSA_ASSERT_VALUE(P, K) S_ASSERT((P) && ssa_get_value_kind(P) == (K))

static inline ssa_constant* _ssa_get_constant(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_CONSTANT);
        return (ssa_constant*)self;
}
static inline const ssa_constant* _ssa_get_cconstant(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_CONSTANT);
        return (const ssa_constant*)self;
}

static inline avalue ssa_get_constant_value(const ssa_value* self)
{
        return _ssa_get_cconstant(self)->_val;
}

static inline void ssa_set_constant_value(ssa_value* self, avalue val)
{
        _ssa_get_constant(self)->_val = val;
}

static inline ssa_decl* _ssa_get_decl(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_DECL);
        return (ssa_decl*)self;
}

static inline const ssa_decl* _ssa_get_cdecl(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_DECL);
        return (const ssa_decl*)self;
}

static inline tree_decl* ssa_get_decl_entity(const ssa_value* self)
{
        return _ssa_get_cdecl(self)->_entity;
}

static inline void ssa_set_decl_entity(ssa_value* self, tree_decl* entity)
{
        _ssa_get_decl(self)->_entity = entity;
}

static inline ssa_string* _ssa_get_string(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_STRING);
        return (ssa_string*)self;
}

static inline const ssa_string* _ssa_get_cstring(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_STRING);
        return (const ssa_string*)self;
}

static inline tree_id ssa_get_string_value(const ssa_value* self)
{
        return _ssa_get_cstring(self)->_id;
}

static inline void ssa_set_string_value(ssa_value* self, tree_id id)
{
        _ssa_get_string(self)->_id = id;
}

#ifdef __cplusplus
}
#endif

#endif