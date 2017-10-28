#ifndef TREE_TYPE_H
#define TREE_TYPE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libscl/list.h>
#include "tree-common.h"
#include "tree-decl.h" // tree_decl_kind

typedef struct _tree_type        tree_type;
typedef struct _tree_decl        tree_decl;
typedef struct _tree_const_exp   tree_const_exp;
typedef struct _tree_target_info tree_target_info;

typedef enum _tree_type_kind
{
        TTK_UNKNOWN,
        TTK_BUILTIN,
        TTK_FUNCTION,
        TTK_POINTER,
        TTK_ARRAY,

        // record, typedef, enum
        TTK_DECL,
        TTK_PAREN,
} tree_type_kind;

struct _tree_type_base
{
        tree_type_kind _kind;
};

extern tree_type* tree_new_type(tree_context* context, tree_type_kind kind, ssize size);

static inline struct _tree_type_base*       _tree_get_type(tree_type* self);
static inline const struct _tree_type_base* _tree_get_ctype(const tree_type* self);

static inline bool           tree_type_is(const tree_type* self, tree_type_kind k);
static inline bool           tree_type_is_qualified(const tree_type* self);
static inline tree_type_kind tree_get_type_kind(const tree_type* self);

static inline void tree_set_type_kind(tree_type* self, tree_type_kind k);

struct _tree_chain_type
{
        struct _tree_type_base _base;
        tree_type*             _next;
};

extern tree_type* tree_new_chain_type(
        tree_context* context, tree_type_kind kind, tree_type* next, ssize size);

static inline struct _tree_chain_type*       _tree_get_chain_type(tree_type* self);
static inline const struct _tree_chain_type* _tree_get_chain_ctype(const tree_type* self);

static inline tree_type* tree_get_chain_type_next(const tree_type* self);
static inline void       tree_set_chain_type_next(tree_type* self, tree_type* next);

typedef enum
{
        TBTK_INVALID,
        TBTK_VOID,
        TBTK_INT8,
        TBTK_UINT8,
        TBTK_INT16,
        TBTK_UINT16,
        TBTK_INT32,
        TBTK_UINT32,
        TBTK_INT64,
        TBTK_UINT64,
        TBTK_FLOAT,
        TBTK_DOUBLE,
        TBTK_VA_ARG,
        TBTK_SIZE,
} tree_builtin_type_kind;

struct _tree_builtin_type
{
        struct _tree_type_base _base;
        tree_builtin_type_kind _kind;
};

extern tree_type* tree_new_builtin_type(tree_context* context, tree_builtin_type_kind kind);

static inline struct _tree_builtin_type*       _tree_get_builtin_type(tree_type* self);
static inline const struct _tree_builtin_type* _tree_get_builtin_ctype(const tree_type* self);

static inline tree_builtin_type_kind tree_get_builtin_type_kind(const tree_type* self);

static inline void tree_set_builtin_type_kind(tree_type* self, tree_builtin_type_kind kind);

struct _tree_function_type
{
        struct _tree_chain_type _base;
        objgroup                _params;
};

extern tree_type* tree_new_function_type(tree_context* context, tree_type* restype);
extern void       tree_set_function_type_params(tree_type* self, objgroup* params);
extern void       tree_add_function_type_param(tree_type* self, tree_type* param);

static inline struct _tree_function_type*       _tree_get_function_type(tree_type* self);
static inline const struct _tree_function_type* _tree_get_function_ctype(const tree_type* self);

static inline ssize       tree_get_function_type_nparams(const tree_type* self);
static inline tree_type*  tree_get_function_restype(const tree_type* self);
static inline tree_type** tree_get_function_type_begin(const tree_type* self);
static inline tree_type** tree_get_function_type_end(const tree_type* self);

static inline void tree_set_function_restype(tree_type* self, tree_type* restype);

#define TREE_FOREACH_FUNC_PARAM(PFUNC, ITNAME)                         \
        for (tree_type** ITNAME = tree_get_function_type_begin(PFUNC); \
                ITNAME != tree_get_function_type_end(PFUNC); ITNAME++)

struct _tree_array_type
{
        struct _tree_chain_type _base;
        tree_const_exp*         _size;
};

extern tree_type* tree_new_array_type(
        tree_context* context, tree_type* eltype, tree_const_exp* size);

static inline struct _tree_array_type*       _tree_get_array_type(tree_type* self);
static inline const struct _tree_array_type* _tree_get_array_ctype(const tree_type* self);

static inline tree_type*      tree_get_array_eltype(const tree_type* self);
static inline tree_const_exp* tree_get_array_size(const tree_type* self);

static inline void tree_set_array_eltype(tree_type* self, tree_type* eltype);
static inline void tree_set_array_size(tree_type* self, tree_const_exp* size);

struct _tree_decl_type
{
        struct _tree_type_base _base;
        tree_decl*             _entity;
        bool                   _referenced;
};

extern tree_type* tree_new_decl_type(tree_context* context, tree_decl* decl, bool referenced);

static inline struct _tree_decl_type*       _tree_get_decl_type(tree_type* self);
static inline const struct _tree_decl_type* _tree_get_decl_ctype(const tree_type* self);

static inline bool       tree_decl_type_is_referenced(const tree_type* self);
static inline tree_decl* tree_get_decl_type_entity(const tree_type* self);

static inline void tree_set_decl_type_entity(tree_type* self, tree_decl* decl);
static inline void tree_set_decl_type_referenced(tree_type* self, bool val);

struct _tree_pointer_type
{
        struct _tree_chain_type _base;
};

extern tree_type* tree_new_pointer_type(tree_context* context, tree_type* target);

static inline struct _tree_pointer_type*       _tree_get_pointer(tree_type* self);
static inline const struct _tree_pointer_type* _tree_get_cpointer(const tree_type* self);

static inline tree_type* tree_get_pointer_target(const tree_type* self);
static inline void       tree_set_pointer_target(tree_type* self, tree_type* target);

struct _tree_paren_type
{
        struct _tree_chain_type _base;
};

extern tree_type* tree_new_paren_type(tree_context* context, tree_type* next);

static inline tree_type* tree_get_paren_type(const tree_type* self);
static inline void       tree_set_paren_type(tree_type* self, tree_type* next);

union _tree_unqualified_type
{
        struct _tree_builtin_type  _builtin;
        struct _tree_function_type _func;
        struct _tree_pointer_type  _pointer;
        struct _tree_array_type    _array;
        struct _tree_decl_type     _decl;
};

typedef enum
{
        TTQ_UNQUALIFIED = 0,
        TTQ_CONST       = 1,
        TTQ_VOLATILE    = 2,
        TTQ_RESTRICT    = 4,
} tree_type_quals;

struct _tree_qualified_type
{
        union _tree_unqualified_type* _type;
        tree_type_quals               _quals;
};

#define _TREE_QUAL_FLAG   ((ssize)1)
#define _TREE_UNQUAL_MASK (~_TREE_QUAL_FLAG)

extern tree_type* tree_new_qual_type(
        tree_context* context, tree_type_quals quals, tree_type* type);

static inline struct _tree_qualified_type*       _tree_get_qtype(tree_type* self);
static inline const struct _tree_qualified_type* _tree_get_cqtype(const tree_type* self);

static inline tree_type_quals  tree_get_type_quals(const tree_type* self);
static inline const tree_type* tree_get_unqualified_ctype(const tree_type* self);
static inline tree_type*       tree_get_unqualified_type(tree_type* self);

static inline void tree_add_type_quals(tree_type* self, tree_type_quals q);
static inline void tree_set_type_quals(tree_type* self, tree_type_quals q);

// if the lowest bit of tree_type* is 1 then it is pointer to qualified type
typedef struct _tree_type
{
        union
        {
                union _tree_unqualified_type _utype;
                struct _tree_qualified_type  _qtype;
        };
} tree_type;

extern tree_type*       tree_ignore_typedefs(tree_type* self);
extern const tree_type* tree_ignore_ctypedefs(const tree_type* self);
extern tree_type*       tree_ignore_paren_types(tree_type* self);
extern const tree_type* tree_ignore_paren_ctypes(const tree_type* self);
// ignores typedefs and ignore parenthesises, if any
extern tree_type*       tree_desugar_type(tree_type* self);
extern const tree_type* tree_desugar_ctype(const tree_type* self);

extern bool tree_builtin_type_is(const tree_type* self, tree_builtin_type_kind k);
extern bool tree_declared_type_is(const tree_type* self, tree_decl_kind k);

extern bool tree_type_is_void(const tree_type* self);
extern bool tree_type_is_signed_integer(const tree_type* self);
extern bool tree_type_is_unsigned_integer(const tree_type* self);
extern bool tree_type_is_real_floating(const tree_type* self);
extern bool tree_type_is_complex_floating(const tree_type* self);
// pointer, function, array or record
extern bool tree_type_is_derived(const tree_type* self);
extern bool tree_type_is_enumerated(const tree_type* self);
extern bool tree_type_is_floating(const tree_type* self);
extern bool tree_type_is_integer(const tree_type* self);
// every type except function
extern bool tree_type_is_object(const tree_type* self);
extern bool tree_type_is_real(const tree_type* self);
// arithmetic or pointer
extern bool tree_type_is_scalar(const tree_type* self);
extern bool tree_type_is_arithmetic(const tree_type* self);
extern bool tree_type_is_pointer(const tree_type* self);
extern bool tree_type_is_record(const tree_type* self);
extern bool tree_type_is_array(const tree_type* self);
extern bool tree_type_is_function_pointer(const tree_type* self);
extern bool tree_type_is_object_pointer(const tree_type* self);
extern bool tree_type_is_void_pointer(const tree_type* self);

// returns false if type size cannot be computed
extern bool tree_type_is_incomplete(const tree_type* self);

extern bool tree_builtin_types_are_same(const tree_type* a, const tree_type* b);
extern bool tree_types_are_compatible(const tree_type* a, const tree_type* b);
// if type is pointer returns pointer target
// if type is array returns array element type
// if type is function returns function result type
// in the other case returns NULL
extern tree_type* tree_get_type_next(const tree_type* self);

extern tree_builtin_type_kind tree_get_integer_counterpart(const tree_type* self);

extern ssize tree_get_sizeof(const tree_target_info* info, const tree_type* t);
extern ssize tree_get_alignof(const tree_target_info* info, const tree_type* t);

#define TREE_ASSERT_TYPE(T) S_ASSERT(T)

static inline struct _tree_type_base* _tree_get_type(tree_type* self)
{
        TREE_ASSERT_TYPE(self);
        return (struct _tree_type_base*)tree_get_unqualified_type(self);
}

static inline const struct _tree_type_base* _tree_get_ctype(const tree_type* self)
{
        TREE_ASSERT_TYPE(self);
        return (const struct _tree_type_base*)tree_get_unqualified_ctype(self);
}

static inline tree_type_kind tree_get_type_kind(const tree_type* self)
{
        return _tree_get_ctype(self)->_kind;
}

static inline void tree_set_type_kind(tree_type* self, tree_type_kind k)
{
        _tree_get_type(self)->_kind = k;
}

static inline struct _tree_chain_type* _tree_get_chain_type(tree_type* self)
{
        TREE_ASSERT_TYPE(self);
        return (struct _tree_chain_type*)_tree_get_type(self);
}

static inline const struct _tree_chain_type* _tree_get_chain_ctype(const tree_type* self)
{
        TREE_ASSERT_TYPE(self);
        return (const struct _tree_chain_type*)_tree_get_ctype(self);
}

static inline tree_type* tree_get_chain_type_next(const tree_type* self)
{
        return _tree_get_chain_ctype(self)->_next;
}

static inline void tree_set_chain_type_next(tree_type* self, tree_type* next)
{
        _tree_get_chain_type(self)->_next = next;
}

#undef TREE_ASSERT_TYPE
#define TREE_ASSERT_TYPE(T, K) S_ASSERT((T) && tree_get_type_kind(T) == (K))

static inline struct _tree_builtin_type* _tree_get_builtin_type(tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_BUILTIN);
        return (struct _tree_builtin_type*)tree_get_unqualified_type(self);
}

static inline const struct _tree_builtin_type* _tree_get_builtin_ctype(const tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_BUILTIN);
        return (const struct _tree_builtin_type*)tree_get_unqualified_ctype(self);
}

static inline tree_builtin_type_kind tree_get_builtin_type_kind(const tree_type* self)
{
        return _tree_get_builtin_ctype(self)->_kind;
}

static inline void tree_set_builtin_type_kind(tree_type* self, tree_builtin_type_kind kind)
{
        _tree_get_builtin_type(self)->_kind = kind;
}

static inline struct _tree_function_type* _tree_get_function_type(tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_FUNCTION);
        return (struct _tree_function_type*)tree_get_unqualified_type(self);
}

static inline const struct _tree_function_type* _tree_get_function_ctype(const tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_FUNCTION);
        return (const struct _tree_function_type*)tree_get_unqualified_ctype(self);
}

static inline ssize tree_get_function_type_nparams(const tree_type* self)
{
        return objgroup_size(&_tree_get_function_ctype(self)->_params);
}

static inline tree_type* tree_get_function_restype(const tree_type* self)
{
        return tree_get_chain_type_next(self);
}

static inline tree_type** tree_get_function_type_begin(const tree_type* self)
{
        return (tree_type**)objgroup_begin(&_tree_get_function_ctype(self)->_params);
}

static inline tree_type** tree_get_function_type_end(const tree_type* self)
{
        return (tree_type**)objgroup_end(&_tree_get_function_ctype(self)->_params);
}

static inline void tree_set_function_restype(tree_type* self, tree_type* restype)
{
        tree_set_chain_type_next(self, restype);
}

static inline struct _tree_array_type* _tree_get_array_type(tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_ARRAY);
        return (struct _tree_array_type*)tree_get_unqualified_type(self);
}

static inline const struct _tree_array_type* _tree_get_array_ctype(const tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_ARRAY);
        return (const struct _tree_array_type*)tree_get_unqualified_ctype(self);
}

static inline tree_type* tree_get_array_eltype(const tree_type* self)
{
        return tree_get_chain_type_next(self);
}

static inline tree_const_exp* tree_get_array_size(const tree_type* self)
{
        return _tree_get_array_ctype(self)->_size;
}

static inline void tree_set_array_eltype(tree_type* self, tree_type* eltype)
{
        tree_set_chain_type_next(self, eltype);
}

static inline void tree_set_array_size(tree_type* self, tree_const_exp* size)
{
        _tree_get_array_type(self)->_size = size;
}

static inline struct _tree_decl_type* _tree_get_decl_type(tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_DECL);
        return (struct _tree_decl_type*)tree_get_unqualified_type(self);
}

static inline const struct _tree_decl_type* _tree_get_decl_ctype(const tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_DECL);
        return (const struct _tree_decl_type*)tree_get_unqualified_ctype(self);
}

static inline bool tree_decl_type_is_referenced(const tree_type* self)
{
        return _tree_get_decl_ctype(self)->_referenced;
}

static inline tree_decl* tree_get_decl_type_entity(const tree_type* self)
{
        return _tree_get_decl_ctype(self)->_entity;
}

static inline void tree_set_decl_type_entity(tree_type* self, tree_decl* decl)
{
        _tree_get_decl_type(self)->_entity = decl;
}

static inline void tree_set_decl_type_referenced(tree_type* self, bool val)
{
        _tree_get_decl_type(self)->_referenced = val;
}

static inline struct _tree_pointer_type* _tree_get_pointer(tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_POINTER);
        return (struct _tree_pointer_type*)tree_get_unqualified_type(self);
}

static inline const struct _tree_pointer_type* _tree_get_cpointer(const tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_POINTER);
        return (const struct _tree_pointer_type*)tree_get_unqualified_ctype(self);
}

static inline tree_type* tree_get_pointer_target(const tree_type* self)
{
        return tree_get_chain_type_next(self);
}

static inline void tree_set_pointer_target(tree_type* self, tree_type* target)
{
        tree_set_chain_type_next(self, target);
}

static inline tree_type* tree_get_paren_type(const tree_type* self)
{
        TREE_ASSERT_TYPE(self, TTK_PAREN);
        return tree_get_chain_type_next(self);
}

static inline void tree_set_paren_type(tree_type* self, tree_type* next)
{
        TREE_ASSERT_TYPE(self, TTK_PAREN);
        tree_set_chain_type_next(self, next);
}

static inline struct _tree_qualified_type* _tree_get_qtype(tree_type* self)
{
        S_ASSERT(tree_type_is_qualified(self));
        self = (tree_type*)((ssize)self & _TREE_UNQUAL_MASK);
        S_ASSERT(self);
        return (struct _tree_qualified_type*)self;
}

static inline const struct _tree_qualified_type* _tree_get_cqtype(const tree_type* self)
{
        S_ASSERT(tree_type_is_qualified(self));
        self = (const tree_type*)((const ssize)self & _TREE_UNQUAL_MASK);
        S_ASSERT(self);
        return (const struct _tree_qualified_type*)self;
}

static inline tree_type_quals tree_get_type_quals(const tree_type* self)
{
        if (!tree_type_is_qualified(self))
                return TTQ_UNQUALIFIED;

        return _tree_get_cqtype(self)->_quals;
}

static inline bool tree_type_is(const tree_type* self, tree_type_kind k)
{
        return tree_get_type_kind(self) == k;
}

static inline bool tree_type_is_qualified(const tree_type* self)
{
        return (bool)(((const ssize)self) & _TREE_QUAL_FLAG);
}

static inline const tree_type* tree_get_unqualified_ctype(const tree_type* self)
{
        if (!tree_type_is_qualified(self))
                return self;

        return (const tree_type*)_tree_get_cqtype(self)->_type;
}

static inline tree_type* tree_get_unqualified_type(tree_type* self)
{
        if (!tree_type_is_qualified(self))
                return self;

        return (tree_type*)_tree_get_qtype(self)->_type;
}

static inline void tree_add_type_quals(tree_type* self, tree_type_quals q)
{
        _tree_get_qtype(self)->_quals |= q;
}

static inline void tree_set_type_quals(tree_type* self, tree_type_quals q)
{
        _tree_get_qtype(self)->_quals = q;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_TYPE_H