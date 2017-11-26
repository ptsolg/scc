#ifndef TREE_DECL_H
#define TREE_DECL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-common.h"

typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_context tree_context;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;
typedef struct _tree_expr tree_expr;
typedef struct _tree_stmt tree_stmt;

typedef struct _tree_decl_scope
{
        struct _tree_decl_scope* _parent;
        htab _lookup;
        list_head _decls;
} tree_decl_scope;

extern void tree_init_decl_scope(
        tree_decl_scope* self, tree_context* context, tree_decl_scope* parent);

extern tree_decl_scope* tree_new_decl_scope(tree_context* context, tree_decl_scope* parent);

extern bool tree_decl_scopes_are_same(const tree_decl_scope* a, const tree_decl_scope* b);

// adds decl only to lookup table
extern void tree_decl_scope_add_lookup(tree_decl_scope* self, hval key, tree_decl* decl);

// adds decl to this scope, but not to its lookup table
extern void tree_decl_scope_add_hidden(tree_decl_scope* self, tree_decl* decl);

// adds decl to this scope 
extern void tree_decl_scope_add(tree_decl_scope* self, hval key, tree_decl* decl);

extern tree_decl* tree_decl_scope_lookup(
        const tree_decl_scope* self, hval key, bool parent_lookup);

static inline tree_decl_scope* tree_get_decl_scope_parent(const tree_decl_scope* self);
static inline tree_decl* tree_get_decl_scope_begin(const tree_decl_scope* self);
static inline tree_decl* tree_get_decl_scope_end(tree_decl_scope* self);
static inline const tree_decl* tree_get_decl_scope_cend(const tree_decl_scope* self);
static inline hiter tree_get_decl_scope_lookup_begin(const tree_decl_scope* self);
static inline bool tree_decl_scope_is_empty(const tree_decl_scope* self);

#define TREE_FOREACH_DECL_IN_SCOPE(PSCOPE, ITNAME) \
        for (tree_decl* ITNAME = tree_get_decl_scope_begin(PSCOPE); \
                ITNAME != tree_get_decl_scope_cend(PSCOPE); \
                ITNAME = tree_get_next_decl(ITNAME))

#define TREE_FOREACH_DECL_IN_LOOKUP(PSCOPE, ITNAME) \
        for (hiter ITNAME = tree_get_decl_scope_lookup_begin(PSCOPE); \
                hiter_valid(&ITNAME); hiter_advance(&ITNAME))

typedef enum
{
        TDSC_NONE,
        TDSC_EXTERN,
        TDSC_IMPL_EXTERN,
        TDSC_STATIC,
        TDSC_AUTO,
        TDSC_REGISTER,
} tree_decl_storage_class;

typedef enum _tree_decl_kind
{
        TDK_UNKNOWN,
        TDK_TYPEDEF,
        TDK_RECORD,
        TDK_ENUM,
        TDK_FUNCTION,
        TDK_MEMBER,
        TDK_VAR,
        TDK_ENUMERATOR,
        TDK_LABEL,

        TDK_GROUP,

        TDK_SIZE,
} tree_decl_kind;

#define TREE_ASSERT_DECL_KIND(K) S_ASSERT((K) > TDK_UNKNOWN && (K) < TDK_SIZE)

struct _tree_decl_base
{
        list_node _node;
        tree_decl_kind _kind;
        tree_decl_scope* _scope;
        tree_xlocation _loc;
        bool _is_implicit;
};

extern tree_decl* tree_new_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        ssize size);

static inline struct _tree_decl_base* _tree_get_decl(tree_decl* self);
static inline const struct _tree_decl_base* _tree_get_cdecl(const tree_decl* self);

static inline tree_decl* tree_get_next_decl(const tree_decl* self);
static inline tree_decl* tree_get_prev_decl(const tree_decl* self);
static inline tree_decl_kind tree_get_decl_kind(const tree_decl* self);
static inline bool tree_decl_is(const tree_decl* self, tree_decl_kind k);
static inline tree_decl_scope* tree_get_decl_scope(const tree_decl* self);
static inline tree_xlocation tree_get_decl_loc(const tree_decl* self);
static inline tree_location tree_get_decl_loc_begin(const tree_decl* self);
static inline tree_location tree_get_decl_loc_end(const tree_decl* self);
static inline bool tree_decl_is_implicit(const tree_decl* self);

static inline void tree_set_decl_scope(tree_decl* self, tree_decl_scope* scope);
static inline void tree_set_decl_kind(tree_decl* self, tree_decl_kind k);
static inline void tree_set_decl_loc(tree_decl* self, tree_xlocation l);
static inline void tree_set_decl_begin_loc(tree_decl* self, tree_location l);
static inline void tree_set_decl_end_loc(tree_decl* self, tree_location l);
static inline void tree_set_decl_implicit(tree_decl* self, bool v);

struct _tree_named_decl
{
        struct _tree_decl_base _base;
        tree_id _name;
};

extern tree_decl* tree_new_named_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        ssize size);

static inline struct _tree_named_decl* _tree_get_named_decl(tree_decl* self);
static inline const struct _tree_named_decl* _tree_get_named_cdecl(const tree_decl* self);

static inline tree_id tree_get_decl_name(const tree_decl* self);
static inline bool tree_decl_is_unnamed(const tree_decl* self);
static inline void tree_set_decl_name(tree_decl* self, tree_id name);

struct _tree_typed_decl
{
        struct _tree_named_decl _base;
        tree_type* _type;
};

extern tree_decl* tree_new_typed_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        ssize size);

static inline struct _tree_typed_decl* _tree_get_typed_decl(tree_decl* self);
static inline const struct _tree_typed_decl* _tree_get_typed_cdecl(const tree_decl* self);

static inline tree_type* tree_get_decl_type(const tree_decl* self);
static inline void tree_set_decl_type(tree_decl* self, tree_type* type);


struct _tree_value_decl
{
        struct _tree_typed_decl _base;
        tree_decl_storage_class _class;
};

extern tree_decl* tree_new_value_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_decl_storage_class class_,
        tree_type* type,
        ssize size);

static inline struct _tree_value_decl* _tree_get_value_decl(tree_decl* self);
static inline const struct _tree_value_decl* _tree_get_value_cdecl(const tree_decl* self);

static inline tree_decl_storage_class tree_get_decl_storage_class(const tree_decl* self);

static inline void tree_set_decl_storage_class(tree_decl* self, tree_decl_storage_class class_);

struct _tree_typedef_decl
{
        struct _tree_typed_decl _base;
};

extern tree_decl* tree_new_typedef_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type);

static inline struct _tree_typedef_decl* _tree_get_typedef(tree_decl* self);
static inline const struct _tree_typedef_decl* _tree_get_ctypedef(const tree_decl* self);

struct _tree_record_decl
{
        struct _tree_named_decl _base;
        tree_decl_scope _scope;
        bool _complete;
        bool _is_union;
};

extern tree_decl* tree_new_record_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        bool is_union);

extern tree_decl* tree_get_record_begin(tree_decl* self);
extern const tree_decl* tree_get_record_cbegin(const tree_decl* self);
extern tree_decl* tree_get_record_end(tree_decl* self);
extern const tree_decl* tree_get_record_cend(const tree_decl* self);
extern const tree_decl* tree_get_next_cmember(const tree_decl* member, const tree_decl* record);
extern tree_decl* tree_get_next_member(tree_decl* member, tree_decl* record);

static inline struct _tree_record_decl* _tree_get_record(tree_decl* self);
static inline const struct _tree_record_decl* _tree_get_crecord(const tree_decl* self);

static inline tree_decl_scope* tree_get_record_scope(tree_decl* self);
static inline const tree_decl_scope* tree_get_record_cscope(const tree_decl* self);
static inline bool tree_record_is_union(const tree_decl* self);
static inline bool tree_record_is_complete(const tree_decl* self);

static inline void tree_set_record_union(tree_decl* self, bool val);
static inline void tree_set_record_complete(tree_decl* self, bool complete);

struct _tree_enum_decl
{
        struct _tree_named_decl _base;
        tree_decl_scope _scope;
};

extern tree_decl* tree_new_enum_decl(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc, tree_id name);

static inline struct _tree_enum_decl* _tree_get_enum(tree_decl* self);
static inline const struct _tree_enum_decl* _tree_get_cenum(const tree_decl* self);

static inline tree_decl_scope* tree_get_enum_scope(tree_decl* self);
static inline const tree_decl_scope* tree_get_enum_cscope(const tree_decl* self);

typedef enum
{
        TFSK_NONE,
        TFSK_INLINE,
} tree_function_specifier_kind;

struct _tree_function_decl
{
        struct _tree_value_decl _base;
        tree_function_specifier_kind _specs;
        tree_decl_scope _params;
        tree_decl_scope _labels;
        tree_stmt* _body;
};

extern tree_decl* tree_new_function_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_decl_storage_class class_,
        tree_type* type,
        tree_function_specifier_kind spec,
        tree_stmt* body);

static inline struct _tree_function_decl* _tree_get_function(tree_decl* self);
static inline const struct _tree_function_decl* _tree_get_cfunction(const tree_decl* self);

static inline tree_function_specifier_kind tree_get_function_specifier(const tree_decl* self);
static inline tree_decl_scope* tree_get_function_params(tree_decl* self);
static inline const tree_decl_scope* tree_get_function_cparams(const tree_decl* self);
static inline tree_decl_scope* tree_get_function_labels(tree_decl* self);
static inline const tree_decl_scope* tree_get_function_clabels(const tree_decl* self);
static inline tree_stmt* tree_get_function_body(const tree_decl* self);

static inline void tree_set_function_specifier(tree_decl* self, tree_function_specifier_kind specs);
static inline void tree_set_function_body(tree_decl* self, tree_stmt* body);

struct _tree_var_decl
{
        struct _tree_value_decl _base;
        tree_expr* _init;
};

extern tree_decl* tree_new_var_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name, 
        tree_decl_storage_class class_,
        tree_type* type,
        tree_expr* init);

static inline struct _tree_var_decl* _tree_get_var(tree_decl* self);
static inline const struct _tree_var_decl* _tree_get_cvar(const tree_decl* self);

static inline tree_expr* tree_get_var_init(const tree_decl* self);
static inline void tree_set_var_init(tree_decl* self, tree_expr* init);

struct _tree_member_decl
{
        struct _tree_value_decl _base;
        tree_expr* _bits;
};

extern tree_decl* tree_new_member_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* bits);

static inline struct _tree_member_decl* _tree_get_member(tree_decl* self);
static inline const struct _tree_member_decl* _tree_get_cmember(const tree_decl* self);

static inline tree_expr* tree_get_member_bits(const tree_decl* self);
static inline void tree_set_member_bits(tree_decl* self, tree_expr* bits);

struct _tree_enumerator_decl
{
        struct _tree_typed_decl _base;
        tree_expr* _value;
};

extern tree_decl* tree_new_enumerator_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* value);

static inline struct _tree_enumerator_decl* _tree_get_enumerator(tree_decl* self);
static inline const struct _tree_enumerator_decl* _tree_get_cenumerator(const tree_decl* self);

static inline tree_expr* tree_get_enumerator_value(const tree_decl* self);
static inline void tree_set_enumerator_value(tree_decl* self, tree_expr* value);

struct _tree_label_decl
{
        struct _tree_named_decl _base;
        tree_stmt* _stmt;
};

extern tree_decl* tree_new_label_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_stmt* stmt);

static inline struct _tree_label_decl* _tree_get_label_decl(tree_decl* self);
static inline const struct _tree_label_decl* _tree_get_label_cdecl(const tree_decl* self);

static inline tree_stmt* tree_get_label_decl_stmt(const tree_decl* self);
static inline void tree_set_label_decl_stmt(tree_decl* self, tree_stmt* stmt);

struct _tree_decl_group
{
        struct _tree_decl_base _base;
        dseq _group;
};

extern tree_decl* tree_new_decl_group(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc);

extern serrcode tree_decl_group_add(tree_decl* self, tree_decl* d);

static inline struct _tree_decl_group* _tree_get_decl_group(tree_decl* self);
static inline const struct _tree_decl_group* _tree_get_decl_cgroup(const tree_decl* self);

static inline tree_decl** tree_get_decl_group_begin(const tree_decl* self);
static inline tree_decl** tree_get_decl_group_end(const tree_decl* self);

#define TREE_FOREACH_DECL_IN_GROUP(PGROUP, ITNAME) \
        for (tree_decl** ITNAME = tree_get_decl_group_begin(PGROUP); \
                ITNAME != tree_get_decl_group_end(PGROUP); ITNAME++)

typedef struct _tree_decl
{
        union
        {
                struct _tree_typedef_decl _typedef;
                struct _tree_record_decl _record;
                struct _tree_function_decl _func;
                struct _tree_member_decl _member;
                struct _tree_var_decl _var;
                struct _tree_enum_decl _enum;
                struct _tree_enumerator_decl _enumerator;
                struct _tree_label_decl _label;
        };
} tree_decl;

extern bool tree_decls_have_same_name(const tree_decl* a, const tree_decl* b);
extern bool tree_decls_have_same_linkage(const tree_decl* a, const tree_decl* b);
extern bool tree_decls_are_same(const tree_decl* a, const tree_decl* b);

// accessors

static inline tree_decl_scope* tree_get_decl_scope_parent(const tree_decl_scope* self)
{
        return self->_parent;
}

static inline tree_decl* tree_get_decl_scope_begin(const tree_decl_scope* self)
{
        return (tree_decl*)list_begin(&self->_decls);
}

static inline tree_decl* tree_get_decl_scope_end(tree_decl_scope* self)
{
        return (tree_decl*)list_end(&self->_decls);
}

static inline const tree_decl* tree_get_decl_scope_cend(const tree_decl_scope* self)
{
        return (const tree_decl*)list_cend(&self->_decls);
}

static inline hiter tree_get_decl_scope_lookup_begin(const tree_decl_scope* self)
{
        return htab_begin(&self->_lookup);
}

static inline bool tree_decl_scope_is_empty(const tree_decl_scope* self)
{
        return list_empty(&self->_decls);
}

#define TREE_ASSERT_DECL(D) S_ASSERT(D)

static inline struct _tree_decl_base* _tree_get_decl(tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (struct _tree_decl_base*)self;
}

static inline const struct _tree_decl_base* _tree_get_cdecl(const tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (const struct _tree_decl_base*)self;
}

static inline tree_decl* tree_get_next_decl(const tree_decl* self)
{
        return (tree_decl*)list_node_next(&_tree_get_cdecl(self)->_node);
}

static inline tree_decl* tree_get_prev_decl(const tree_decl* self)
{
        return (tree_decl*)list_node_prev(&_tree_get_cdecl(self)->_node);
}

static inline tree_decl_kind tree_get_decl_kind(const tree_decl* self)
{
        return _tree_get_cdecl(self)->_kind;
}

static inline bool tree_decl_is(const tree_decl* self, tree_decl_kind k)
{
        return tree_get_decl_kind(self) == k;
}

static inline tree_decl_scope* tree_get_decl_scope(const tree_decl* self)
{
        return _tree_get_cdecl(self)->_scope;
}

static inline tree_xlocation tree_get_decl_loc(const tree_decl* self)
{
        return _tree_get_cdecl(self)->_loc;
}

static inline tree_location tree_get_decl_loc_begin(const tree_decl* self)
{
        return tree_get_xloc_begin(tree_get_decl_loc(self));
}

static inline tree_location tree_get_decl_loc_end(const tree_decl* self)
{
        return tree_get_xloc_end(tree_get_decl_loc(self));
}

static inline bool tree_decl_is_implicit(const tree_decl* self)
{
        return _tree_get_cdecl(self)->_is_implicit;
}

static inline void tree_set_decl_scope(tree_decl* self, tree_decl_scope* scope)
{
        _tree_get_decl(self)->_scope = scope;
}

static inline void tree_set_decl_kind(tree_decl* self, tree_decl_kind k)
{
        _tree_get_decl(self)->_kind = k;
}

static inline void tree_set_decl_loc(tree_decl* self, tree_xlocation l)
{
        _tree_get_decl(self)->_loc = l;
}

static inline void tree_set_decl_begin_loc(tree_decl* self, tree_location l)
{
        _tree_get_decl(self)->_loc = tree_set_xloc_begin(tree_get_decl_loc(self), l);
}

static inline void tree_set_decl_end_loc(tree_decl* self, tree_location l)
{
        _tree_get_decl(self)->_loc = tree_set_xloc_end(tree_get_decl_loc(self), l);
}

static inline void tree_set_decl_implicit(tree_decl* self, bool v)
{
        _tree_get_decl(self)->_is_implicit = v;
}

static inline struct _tree_named_decl* _tree_get_named_decl(tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (struct _tree_named_decl*)self;
}

static inline const struct _tree_named_decl* _tree_get_named_cdecl(const tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (const struct _tree_named_decl*)self;
}

static inline tree_id tree_get_decl_name(const tree_decl* self)
{
        return _tree_get_named_cdecl(self)->_name;
}

static inline bool tree_decl_is_unnamed(const tree_decl* self)
{
        return tree_id_is_empty(tree_get_decl_name(self));
}

static inline void tree_set_decl_name(tree_decl* self, tree_id name)
{
        _tree_get_named_decl(self)->_name = name;
}

static inline struct _tree_typed_decl* _tree_get_typed_decl(tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (struct _tree_typed_decl*)self;
}

static inline const struct _tree_typed_decl* _tree_get_typed_cdecl(const tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (const struct _tree_typed_decl*)self;
}

static inline tree_type* tree_get_decl_type(const tree_decl* self)
{
        return _tree_get_typed_cdecl(self)->_type;
}

static inline void tree_set_decl_type(tree_decl* self, tree_type* type)
{
        _tree_get_typed_decl(self)->_type = type;
}

static inline struct _tree_value_decl* _tree_get_value_decl(tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (struct _tree_value_decl*)self;
}

static inline const struct _tree_value_decl* _tree_get_value_cdecl(const tree_decl* self)
{
        TREE_ASSERT_DECL(self);
        return (const struct _tree_value_decl*)self;
}

static inline tree_decl_storage_class tree_get_decl_storage_class(const tree_decl* self)
{
        return _tree_get_value_cdecl(self)->_class;
}

static inline void tree_set_decl_storage_class(tree_decl* self, tree_decl_storage_class class_)
{
        _tree_get_value_decl(self)->_class = class_;
}

#undef TREE_ASSERT_DECL
#define TREE_ASSERT_DECL(D, K) S_ASSERT((D) && tree_get_decl_kind(D) == (K))

static inline struct _tree_typedef_decl* _tree_get_typedef(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_TYPEDEF);
        return (struct _tree_typedef_decl*)self;
}
static inline const struct _tree_typedef_decl* _tree_get_ctypedef(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_TYPEDEF);
        return (const struct _tree_typedef_decl*)self;
}

static inline struct _tree_record_decl* _tree_get_record(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_RECORD);
        return (struct _tree_record_decl*)self;
}

static inline const struct _tree_record_decl* _tree_get_crecord(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_RECORD);
        return (const struct _tree_record_decl*)self;
}

static inline tree_decl_scope* tree_get_record_scope(tree_decl* self)
{
        return &_tree_get_record(self)->_scope;
}

static inline const tree_decl_scope* tree_get_record_cscope(const tree_decl* self)
{
        return &_tree_get_crecord(self)->_scope;
}

static inline bool tree_record_is_union(const tree_decl* self)
{
        return _tree_get_crecord(self)->_is_union;
}

static inline bool tree_record_is_complete(const tree_decl* self)
{
        return _tree_get_crecord(self)->_complete;
}

static inline void tree_set_record_union(tree_decl* self, bool val)
{
        _tree_get_record(self)->_is_union = val;
}

static inline void tree_set_record_complete(tree_decl* self, bool complete)
{
        _tree_get_record(self)->_complete = complete;
}

static inline struct _tree_enum_decl* _tree_get_enum(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_ENUM);
        return (struct _tree_enum_decl*)self;
}

static inline const struct _tree_enum_decl* _tree_get_cenum(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_ENUM);
        return (const struct _tree_enum_decl*)self;
}

static inline tree_decl_scope* tree_get_enum_scope(tree_decl* self)
{
        return &_tree_get_enum(self)->_scope;
}

static inline const tree_decl_scope* tree_get_enum_cscope(const tree_decl* self)
{
        return &_tree_get_cenum(self)->_scope;
}

static inline struct _tree_function_decl* _tree_get_function(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_FUNCTION);
        return (struct _tree_function_decl*)self;
}

static inline const struct _tree_function_decl* _tree_get_cfunction(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_FUNCTION);
        return (const struct _tree_function_decl*)self;
}

static inline tree_function_specifier_kind tree_get_function_specifier(const tree_decl* self)
{
        return _tree_get_cfunction(self)->_specs;
}

static inline tree_decl_scope* tree_get_function_params(tree_decl* self)
{
        return &_tree_get_function(self)->_params;
}

static inline const tree_decl_scope* tree_get_function_cparams(const tree_decl* self)
{
        return &_tree_get_cfunction(self)->_params;
}

static inline tree_decl_scope* tree_get_function_labels(tree_decl* self)
{
        return &_tree_get_function(self)->_labels;
}

static inline const tree_decl_scope* tree_get_function_clabels(const tree_decl* self)
{
        return &_tree_get_cfunction(self)->_labels;
}

static inline tree_stmt* tree_get_function_body(const tree_decl* self)
{
        return _tree_get_cfunction(self)->_body;
}

static inline void tree_set_function_specifier(tree_decl* self, tree_function_specifier_kind specs)
{
        _tree_get_function(self)->_specs = specs;
}

static inline void tree_set_function_body(tree_decl* self, tree_stmt* body)
{
        _tree_get_function(self)->_body = body;
}

static inline struct _tree_var_decl* _tree_get_var(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_VAR);
        return (struct _tree_var_decl*)self;
}

static inline const struct _tree_var_decl* _tree_get_cvar(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_VAR);
        return (const struct _tree_var_decl*)self;
}

static inline tree_expr* tree_get_var_init(const tree_decl* self)
{
        return _tree_get_cvar(self)->_init;
}

static inline void tree_set_var_init(tree_decl* self, tree_expr* init)
{
        _tree_get_var(self)->_init = init;
}

static inline struct _tree_member_decl* _tree_get_member(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_MEMBER);
        return (struct _tree_member_decl*)self;
}

static inline const struct _tree_member_decl* _tree_get_cmember(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_MEMBER);
        return (const struct _tree_member_decl*)self;
}

static inline tree_expr* tree_get_member_bits(const tree_decl* self)
{
        return _tree_get_cmember(self)->_bits;
}

static inline void tree_set_member_bits(tree_decl* self, tree_expr* bits)
{
        _tree_get_member(self)->_bits = bits;
}

static inline struct _tree_enumerator_decl* _tree_get_enumerator(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_ENUMERATOR);
        return (struct _tree_enumerator_decl*)self;
}

static inline const struct _tree_enumerator_decl* _tree_get_cenumerator(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_ENUMERATOR);
        return (const struct _tree_enumerator_decl*)self;
}

static inline tree_expr* tree_get_enumerator_value(const tree_decl* self)
{
        return _tree_get_cenumerator(self)->_value;
}

static inline void tree_set_enumerator_value(tree_decl* self, tree_expr* value)
{
        _tree_get_enumerator(self)->_value = value;
}

static inline struct _tree_label_decl* _tree_get_label_decl(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_LABEL);
        return (struct _tree_label_decl*)self;
}

static inline const struct _tree_label_decl* _tree_get_label_cdecl(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_LABEL);
        return (const struct _tree_label_decl*)self;
}

static inline tree_stmt* tree_get_label_decl_stmt(const tree_decl* self)
{
        return _tree_get_label_cdecl(self)->_stmt;
}

static inline void tree_set_label_decl_stmt(tree_decl* self, tree_stmt* stmt)
{
        _tree_get_label_decl(self)->_stmt = stmt;
}

static inline struct _tree_decl_group* _tree_get_decl_group(tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_GROUP);
        return (struct _tree_decl_group*)self;
}

static inline const struct _tree_decl_group* _tree_get_decl_cgroup(const tree_decl* self)
{
        TREE_ASSERT_DECL(self, TDK_GROUP);
        return (const struct _tree_decl_group*)self;
}

static inline tree_decl** tree_get_decl_group_begin(const tree_decl* self)
{
        return (tree_decl**)dseq_begin_ptr(&_tree_get_decl_cgroup(self)->_group);
}

static inline tree_decl** tree_get_decl_group_end(const tree_decl* self)
{
        return (tree_decl**)dseq_end_ptr(&_tree_get_decl_cgroup(self)->_group);
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_DECL_H
