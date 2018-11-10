#ifndef TREE_TYPE_H
#define TREE_TYPE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "decl.h" // tree_decl_kind

typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;

typedef enum
{
        TTQ_UNQUALIFIED = 0,
        TTQ_CONST = 1,
        TTQ_VOLATILE = 2,
        TTQ_RESTRICT = 4,
} tree_type_quals;

struct _tree_modified_type
{
        tree_type* type;
        unsigned quals : 3;
        unsigned transaction_safe : 1;
};

#define _TREE_MODIFIED_TYPE_BIT ((size_t)1)

extern tree_type* tree_new_modified_type(tree_context* context, tree_type* type);

extern tree_type* tree_new_qualified_type(
        tree_context* context, tree_type* type, tree_type_quals quals);

static TREE_INLINE bool tree_type_is_modified(const tree_type* self)
{
        return ((size_t)self & _TREE_MODIFIED_TYPE_BIT) != 0;
}

static TREE_INLINE struct _tree_modified_type* _tree_modified_type(tree_type* self)
{
        self = (tree_type*)((size_t)self & ~_TREE_MODIFIED_TYPE_BIT);
        assert(self);
        return (struct _tree_modified_type*)self;
}

static TREE_INLINE const struct _tree_modified_type* _tree_modified_type_c(const tree_type* self)
{
        self = (const tree_type*)((size_t)self & ~_TREE_MODIFIED_TYPE_BIT);
        assert(self);
        return (const struct _tree_modified_type*)self;
}

static TREE_INLINE const tree_type* tree_get_modified_type_c(const tree_type* self)
{
        return tree_type_is_modified(self) ? _tree_modified_type_c(self)->type : self;
}

static TREE_INLINE tree_type* tree_get_modified_type(tree_type* self)
{
        return tree_type_is_modified(self) ? _tree_modified_type(self)->type : self;
}

static TREE_INLINE tree_type_quals tree_get_type_quals(const tree_type* self)
{
        return tree_type_is_modified(self) ? _tree_modified_type_c(self)->quals : TTQ_UNQUALIFIED;
}

static TREE_INLINE void tree_set_type_quals(tree_type* self, tree_type_quals quals)
{
        assert(tree_type_is_modified(self));
        _tree_modified_type(self)->quals = quals;
}

static TREE_INLINE void tree_add_type_quals(tree_type* self, tree_type_quals quals)
{
        assert(tree_type_is_modified(self));
        _tree_modified_type(self)->quals |= quals;
}

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

        TTK_SIZE,
} tree_type_kind;

struct _tree_type_base
{
#define TREE_TYPE_KIND_BITS 3
#define TREE_TYPE_BITS TREE_TYPE_KIND_BITS

        union
        {
                struct
                {
                        unsigned kind : TREE_TYPE_KIND_BITS;
                };

                struct
                {
                        unsigned base : TREE_TYPE_BITS;
                        unsigned vararg : 1;
                        unsigned cc : 3;
                } function_type_bits;

                struct
                {
                        unsigned base : TREE_TYPE_BITS;
                        unsigned kind : 4;
                } builtin_type_bits;

                struct
                {
                        unsigned base : TREE_TYPE_BITS;
                        unsigned kind : 1;
                } array_type_bits;

                struct
                {
                        unsigned base : TREE_TYPE_BITS;
                        unsigned referenced : 1;
                } decl_type_bits;
        };
#undef TREE_NUM_TYPE_BITS   
#undef TREE_TYPE_KIND_BITS
#undef TREE_TYPE_QUALS_BITS
};

extern tree_type* tree_new_type(tree_context* context, tree_type_kind kind, size_t size);

static TREE_INLINE struct _tree_type_base* _tree_type_base(tree_type* self)
{
        return (struct _tree_type_base*)tree_get_modified_type(self);
}

static TREE_INLINE const struct _tree_type_base* _tree_type_base_c(const tree_type* self)
{
        return (const struct _tree_type_base*)tree_get_modified_type_c(self);
}

static TREE_INLINE tree_type_kind tree_get_type_kind(const tree_type* self)
{
        return _tree_type_base_c(self)->kind;
}

static TREE_INLINE bool tree_type_is(const tree_type* self, tree_type_kind k)
{
        return tree_get_type_kind(self) == k;
}

static TREE_INLINE bool tree_type_is_qualified(const tree_type* self)
{
        return tree_type_is_modified(self);
}

static TREE_INLINE void tree_set_type_kind(tree_type* self, tree_type_kind k)
{
        assert(k >= TTK_UNKNOWN && k < TTK_SIZE);
        _tree_type_base(self)->kind = k;
}

struct _tree_chain_type
{
        struct _tree_type_base base;
        tree_type* next;
};

extern tree_type* tree_new_chain_type(
        tree_context* context, tree_type_kind kind, tree_type* next, size_t size);

#define _TREE_ASSERT_CHAIN_TYPE(K)\
        assert((K) == TTK_POINTER || (K) == TTK_FUNCTION || (K) == TTK_PAREN || (K) == TTK_ARRAY)

static TREE_INLINE struct _tree_chain_type* _tree_chain_type(tree_type* self)
{
        _TREE_ASSERT_CHAIN_TYPE(tree_get_type_kind(self));
        return (struct _tree_chain_type*)_tree_type_base(self);
}

static TREE_INLINE const struct _tree_chain_type* _tree_chain_type_c(const tree_type* self)
{
        _TREE_ASSERT_CHAIN_TYPE(tree_get_type_kind(self));
        return (const struct _tree_chain_type*)_tree_type_base_c(self);
}

static TREE_INLINE tree_type* tree_get_chain_type_next(const tree_type* self)
{
        return _tree_chain_type_c(self)->next;
}

static TREE_INLINE void tree_set_chain_type_next(tree_type* self, tree_type* next)
{
        _tree_chain_type(self)->next = next;
}

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
        TBTK_SIZE,
} tree_builtin_type_kind;

struct _tree_builtin_type
{
        struct _tree_type_base base;
};

extern void tree_init_builtin_type(tree_type* self, tree_builtin_type_kind kind);
extern tree_type* tree_new_builtin_type(tree_context* context, tree_builtin_type_kind kind);
// uint32_t or uint64_t
extern tree_type* tree_new_size_type(tree_context* context);

static TREE_INLINE tree_builtin_type_kind tree_get_builtin_type_kind(const tree_type* self)
{
        assert(tree_type_is(self, TTK_BUILTIN));
        return _tree_type_base_c(self)->builtin_type_bits.kind;
}

static TREE_INLINE void tree_set_builtin_type_kind(tree_type* self, tree_builtin_type_kind kind)
{
        assert(tree_type_is(self, TTK_BUILTIN) && kind >= TBTK_INVALID && kind < TBTK_SIZE);
        _tree_type_base(self)->builtin_type_bits.kind = kind;
}

typedef enum
{
        TCC_DEFAULT,
        TCC_STDCALL,
} tree_calling_convention;

struct _tree_func_type
{
        struct _tree_chain_type base;
        tree_array params;
};

extern tree_type* tree_new_func_type(tree_context* context, tree_type* restype);
extern errcode tree_add_func_type_param(tree_type* self, tree_context* context, tree_type* param);

static TREE_INLINE struct _tree_func_type* _tree_func_type(tree_type* self)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        return (struct _tree_func_type*)_tree_chain_type(self);
}

static TREE_INLINE const struct _tree_func_type* _tree_func_type_c(const tree_type* self)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        return (const struct _tree_func_type*)_tree_chain_type_c(self);
}

static TREE_INLINE tree_type* tree_get_func_type_result(const tree_type* self)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        return tree_get_chain_type_next(self);
}

static TREE_INLINE tree_type** tree_get_func_type_params_begin(const tree_type* self)
{
        return (tree_type**)_tree_func_type_c(self)->params.data;
}

static TREE_INLINE size_t tree_get_func_type_params_size(const tree_type* self)
{
        return _tree_func_type_c(self)->params.size;
}

static TREE_INLINE tree_type** tree_get_func_type_params_end(const tree_type* self)
{
        return tree_get_func_type_params_begin(self) + tree_get_func_type_params_size(self);
}

static TREE_INLINE tree_type* tree_get_func_type_param(const tree_type* self, size_t n)
{
        assert(n < tree_get_func_type_params_size(self));
        return tree_get_func_type_params_begin(self)[n];
}

static TREE_INLINE bool tree_func_type_is_vararg(const tree_type* self)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        return _tree_type_base_c(self)->function_type_bits.vararg;
}

static TREE_INLINE tree_calling_convention tree_get_func_type_cc(const tree_type* self)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        return _tree_type_base_c(self)->function_type_bits.cc;
}

static TREE_INLINE bool tree_func_type_is_transaction_safe(const tree_type* self)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        return tree_type_is_modified(self)
                ? _tree_modified_type_c(self)->transaction_safe : false;
}

static TREE_INLINE void tree_set_func_type_result(tree_type* self, tree_type* restype)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        tree_set_chain_type_next(self, restype);
}

static TREE_INLINE void tree_set_func_type_vararg(tree_type* self, bool vararg)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        _tree_type_base(self)->function_type_bits.vararg = vararg;
}

static TREE_INLINE void tree_set_func_type_cc(tree_type* self, tree_calling_convention cc)
{
        assert(tree_type_is(self, TTK_FUNCTION));
        _tree_type_base(self)->function_type_bits.cc = cc;
}

static TREE_INLINE void tree_set_func_type_transaction_safe(tree_type* self, bool safe)
{
        assert(tree_type_is_modified(self));
        assert(tree_type_is(self, TTK_FUNCTION));
        _tree_modified_type(self)->transaction_safe = safe;
}

#define TREE_FOREACH_FUNC_TYPE_PARAM(PFUNC, ITNAME) \
        for (tree_type** ITNAME = tree_get_func_type_params_begin(PFUNC); \
                ITNAME != tree_get_func_type_params_end(PFUNC); ITNAME++)

typedef enum _tree_array_kind
{
        TAK_INCOMPLETE,
        TAK_CONSTANT,
} tree_array_kind;

struct _tree_array_type
{
        struct _tree_chain_type base;
        tree_expr* size_expr;
        int_value size_value;
};

extern void tree_init_array_type(tree_type* self, tree_array_kind kind, tree_type* eltype);
extern tree_type* tree_new_array_type(
        tree_context* context, tree_array_kind kind, tree_type* eltype);

extern void tree_init_incomplete_array_type(tree_type* self, tree_type* eltype);
extern tree_type* tree_new_incomplete_array_type(tree_context* context, tree_type* eltype);

extern void tree_init_constant_array_type(
        tree_type* self, tree_type* eltype, tree_expr* size_expr, const int_value* size_value);

extern tree_type* tree_new_constant_array_type(
        tree_context* context,
        tree_type* eltype,
        tree_expr* size_expr,
        const int_value* size_value);

static TREE_INLINE tree_type* tree_get_array_eltype(const tree_type* self)
{
        assert(tree_type_is(self, TTK_ARRAY));
        return tree_get_chain_type_next(self);
}

static TREE_INLINE tree_array_kind tree_get_array_kind(const tree_type* self)
{
        assert(tree_type_is(self, TTK_ARRAY));
        return _tree_type_base_c(self)->array_type_bits.kind;
}

static TREE_INLINE bool tree_array_is(const tree_type* self, tree_array_kind kind)
{
        return tree_get_array_kind(self) == kind;
}

static TREE_INLINE void tree_set_array_eltype(tree_type* self, tree_type* eltype)
{
        assert(tree_type_is(self, TTK_ARRAY));
        tree_set_chain_type_next(self, eltype);
}

static TREE_INLINE void tree_set_array_kind(tree_type* self, tree_array_kind kind)
{
        assert(tree_type_is(self, TTK_ARRAY) && kind == TAK_CONSTANT || kind == TAK_INCOMPLETE);
        _tree_type_base(self)->array_type_bits.kind = kind;
}

static TREE_INLINE struct _tree_array_type* _tree_array_type(tree_type* self)
{
        assert(tree_type_is(self, TTK_ARRAY));
        return (struct _tree_array_type*)_tree_chain_type(self);
}

static TREE_INLINE const struct _tree_array_type* _tree_array_type_c(const tree_type* self)
{
        assert(tree_type_is(self, TTK_ARRAY));
        return (const struct _tree_array_type*)_tree_chain_type_c(self);
}

static TREE_INLINE tree_expr* tree_get_array_size_expr(const tree_type* self)
{
        return _tree_array_type_c(self)->size_expr;
}

static TREE_INLINE const int_value* tree_get_array_size_value_c(const tree_type* self)
{
        return &_tree_array_type_c(self)->size_value;
}

static TREE_INLINE uint tree_get_array_size(const tree_type* self)
{
        return int_get_u32(tree_get_array_size_value_c(self));
}

static TREE_INLINE void tree_set_array_size_expr(tree_type* self, tree_expr* size)
{
        _tree_array_type(self)->size_expr = size;
}

static TREE_INLINE void tree_set_array_size_value(tree_type* self, const int_value* size)
{
        _tree_array_type(self)->size_value = *size;
}

struct _tree_decl_type
{
        struct _tree_type_base base;
        tree_decl* entity;
};

extern tree_type* tree_new_decl_type(tree_context* context, tree_decl* decl, bool referenced);

static TREE_INLINE struct _tree_decl_type* _tree_decl_type(tree_type* self)
{
        assert(tree_type_is(self, TTK_DECL));
        return (struct _tree_decl_type*)_tree_type_base(self);
}

static TREE_INLINE const struct _tree_decl_type* _tree_decl_type_c(const tree_type* self)
{
        assert(tree_type_is(self, TTK_DECL));
        return (const struct _tree_decl_type*)_tree_type_base_c(self);
}

static TREE_INLINE bool tree_decl_type_is_referenced(const tree_type* self)
{
        assert(tree_type_is(self, TTK_DECL));
        return _tree_type_base_c(self)->decl_type_bits.referenced;
}

static TREE_INLINE tree_decl* tree_get_decl_type_entity(const tree_type* self)
{
        return _tree_decl_type_c(self)->entity;
}

static TREE_INLINE void tree_set_decl_type_entity(tree_type* self, tree_decl* decl)
{
        _tree_decl_type(self)->entity = decl;
}

static TREE_INLINE void tree_set_decl_type_referenced(tree_type* self, bool val)
{
        assert(tree_type_is(self, TTK_DECL));
        _tree_type_base(self)->decl_type_bits.referenced = val;
}

struct _tree_pointer_type
{
        struct _tree_chain_type base;
};

extern tree_type* tree_new_pointer_type(tree_context* context, tree_type* target);

static TREE_INLINE tree_type* tree_get_pointer_target(const tree_type* self)
{
        assert(tree_type_is(self, TTK_POINTER));
        return tree_get_chain_type_next(self);
}

static TREE_INLINE void tree_set_pointer_target(tree_type* self, tree_type* target)
{
        assert(tree_type_is(self, TTK_POINTER));
        tree_set_chain_type_next(self, target);
}

struct _tree_paren_type
{
        struct _tree_chain_type base;
};

extern tree_type* tree_new_paren_type(tree_context* context, tree_type* next);

static TREE_INLINE tree_type* tree_get_paren_type(const tree_type* self)
{
        assert(tree_type_is(self, TTK_PAREN));
        return tree_get_chain_type_next(self);
}

static TREE_INLINE void tree_set_paren_type(tree_type* self, tree_type* next)
{
        assert(tree_type_is(self, TTK_PAREN));
        tree_set_chain_type_next(self, next);
}

typedef struct _tree_type
{
        union
        {
                struct _tree_builtin_type builtin;
                struct _tree_func_type func;
                struct _tree_pointer_type pointer;
                struct _tree_array_type array;
                struct _tree_decl_type decl;
                struct _tree_paren_type paren;
                struct _tree_modified_type modified;
        };
} tree_type;

extern tree_type* tree_ignore_typedefs(tree_type* self);
extern const tree_type* tree_ignore_ctypedefs(const tree_type* self);
extern tree_type* tree_ignore_paren_types(tree_type* self);
extern const tree_type* tree_ignore_paren_ctypes(const tree_type* self);
// ignores typedefs and ignore parenthesises, if any
extern tree_type* tree_desugar_type(tree_type* self);
extern const tree_type* tree_desugar_ctype(const tree_type* self);
extern tree_type* tree_ignore_chain_types(tree_type* self);

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
extern bool tree_type_is_incomplete_or_object(const tree_type* self);

// if type is pointer returns pointer target
// if type is array returns array element type
// if type is function returns function result type
// in the other case returns NULL
extern tree_type* tree_get_type_next(const tree_type* self);

extern tree_builtin_type_kind tree_get_integer_counterpart(const tree_type* self);

typedef enum
{
        // types are not equal
        TTEK_NEQ,
        // types are equal
        TTEK_EQ,
        // types are equal, but have different qualifiers
        TTEK_DIFFERENT_QUALS,
        // types are equal, but have different attributes
        TTEK_DIFFERENT_ATTRIBS,
} tree_type_equality_kind;

extern tree_type_equality_kind tree_compare_types(const tree_type* a, const tree_type* b);

#ifdef __cplusplus
}
#endif

#endif // !TREE_TYPE_H
