#ifndef TREE_DECL_H
#define TREE_DECL_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-common.h"
#include "scc/core/value.h"

typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_context tree_context;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;
typedef struct _tree_expr tree_expr;
typedef struct _tree_stmt tree_stmt;

typedef enum
{
        TLK_DECL,
        TLK_TAG,
        TLK_ANY,
} tree_lookup_kind;

typedef struct _tree_decl_scope
{
        struct _tree_decl_scope* parent;
        list_head decls;
        strmap* lookup[2];
} tree_decl_scope;

extern void tree_init_decl_scope(
        tree_decl_scope* self, tree_context* context, tree_decl_scope* parent);

extern tree_decl_scope* tree_new_decl_scope(tree_context* context, tree_decl_scope* parent);

extern tree_decl* tree_decl_scope_lookup(
        const tree_decl_scope* self,
        tree_lookup_kind lookup_kind,
        tree_id id,
        bool parent_lookup);

extern errcode tree_decl_scope_update_lookup(tree_decl_scope* self, tree_context* context, tree_decl* decl);
extern errcode tree_decl_scope_add_decl(tree_decl_scope* self, tree_context* context, tree_decl* decl);
extern void tree_decl_scope_add_hidden_decl(tree_decl_scope* self, tree_decl* decl);

static TREE_INLINE tree_decl_scope* tree_get_decl_scope_parent(const tree_decl_scope* self)
{
        return self->parent;
}

static TREE_INLINE void tree_set_decl_scope_parent(tree_decl_scope* self, tree_decl_scope* parent)
{
        self->parent = parent;
}

static TREE_INLINE tree_decl* tree_get_decl_scope_decls_begin(const tree_decl_scope* self)
{
        return (tree_decl*)self->decls.head;
}

static TREE_INLINE tree_decl* tree_get_decl_scope_decls_end(tree_decl_scope* self)
{
        return (tree_decl*)list_end(&self->decls);
}

static TREE_INLINE const tree_decl* tree_get_decl_scope_decls_cend(const tree_decl_scope* self)
{
        return (const tree_decl*)list_end_c(&self->decls);
}

static TREE_INLINE bool tree_decl_scope_is_empty(const tree_decl_scope* self)
{
        return list_empty(&self->decls);
}

#define TREE_FOREACH_DECL_IN_SCOPE(PSCOPE, ITNAME) \
        for (tree_decl* ITNAME = tree_get_decl_scope_decls_begin(PSCOPE); \
                ITNAME != tree_get_decl_scope_decls_cend(PSCOPE); \
                ITNAME = tree_get_next_decl(ITNAME))

typedef enum
{
        TSC_NONE,
        TSC_EXTERN,
        TSC_IMPL_EXTERN,
        TSC_STATIC,
        TSC_AUTO,
        TSC_REGISTER,
} tree_storage_class;

typedef enum
{
        TDSC_NONE,
        TDSC_IMPORT,
} tree_dll_storage_class;

typedef enum
{
        TSD_AUTOMATIC,
        TSD_THREAD,
        TSD_STATIC,
} tree_storage_duration;

typedef enum
{
        TDK_UNKNOWN,
        TDK_TYPEDEF,
        TDK_RECORD,
        TDK_ENUM,
        TDK_FUNCTION,
        TDK_FIELD,
        TDK_INDIRECT_FIELD,
        TDK_VAR,
        TDK_PARAM,
        TDK_ENUMERATOR,
        TDK_LABEL,

        TDK_GROUP,

        TDK_SIZE,
} tree_decl_kind;

#define TREE_ASSERT_DECL_KIND(K) assert((K) > TDK_UNKNOWN && (K) < TDK_SIZE)

struct _tree_decl_base
{
        list_node node;
        tree_decl_kind kind;
        tree_decl_scope* scope;
        tree_xlocation loc;
        bool is_implicit;
};

struct _tree_named_decl
{
        struct _tree_decl_base base;
        tree_id name;
};

struct _tree_typed_decl
{
        struct _tree_named_decl base;
        tree_type* type;
};

struct _tree_value_decl
{
        struct _tree_typed_decl base;
        tree_storage_class storage_class;
        tree_storage_duration storage_duration;
        tree_dll_storage_class dll_storage_class;
};

struct _tree_typedef_decl
{
        struct _tree_typed_decl base;
};

struct _tree_tag_decl
{
        struct _tree_named_decl base;
        bool complete;
};

struct _tree_record_decl
{
        struct _tree_tag_decl base;
        tree_decl_scope fields;
        bool is_union;
};

struct _tree_enum_decl
{
        struct _tree_tag_decl base;
        tree_decl_scope values;
};

typedef enum
{
        TFBK_ORDINARY,
        TFBK_ATOMIC_CMPXCHG_32_WEAK_SEQ_CST,
        TFBK_ATOMIC_ADD_FETCH_32_SEQ_CST,
        TFBK_ATOMIC_XCHG_32_SEQ_CST,
        TFBK_ATOMIC_FENCE_ST_SEQ_CST,
        TFBK_ATOMIC_FENCE_ST_ACQ,
        TFBK_ATOMIC_FENCE_ST_REL,
        TFBK_ATOMIC_FENCE_ST_ACQ_REL,
} tree_function_builtin_kind;

struct _tree_function_decl
{
        struct _tree_value_decl base;
        tree_decl_scope params;
        tree_decl_scope labels;
        tree_stmt* body;
        tree_function_builtin_kind builtin_kind;
        bool inlined;
};

struct _tree_var_decl
{
        struct _tree_value_decl base;
        tree_expr* init;
};

struct _tree_field_decl
{
        struct _tree_value_decl base;
        tree_expr* bit_width;
        uint index;
};

struct _tree_indirect_field_decl
{
        struct _tree_value_decl base;
        tree_decl* anon_record;
};

struct _tree_enumerator_decl
{
        struct _tree_typed_decl base;
        tree_expr* expr;
        int_value value;
};

struct _tree_label_decl
{
        struct _tree_named_decl base;
        tree_stmt* stmt;
};

struct _tree_decl_group
{
        struct _tree_decl_base base;
        tree_array decls;
};

typedef struct _tree_decl
{
        union
        {
                struct _tree_decl_base base;
                struct _tree_named_decl named;
                struct _tree_typed_decl typed;
                struct _tree_value_decl value;
                struct _tree_tag_decl tag;
                struct _tree_typedef_decl typedef_decl;
                struct _tree_record_decl record;
                struct _tree_enum_decl enum_decl;
                struct _tree_function_decl func;
                struct _tree_var_decl var;
                struct _tree_field_decl field;
                struct _tree_indirect_field_decl indirect_field;
                struct _tree_enumerator_decl enumerator;
                struct _tree_label_decl label;
                struct _tree_decl_group group;
        };
} tree_decl;

extern bool tree_decls_have_same_name(const tree_decl* a, const tree_decl* b);
extern bool tree_decls_have_same_linkage(const tree_decl* a, const tree_decl* b);

extern tree_decl* tree_new_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        size_t size);

static TREE_INLINE tree_decl* tree_get_next_decl(const tree_decl* self)
{
        return (tree_decl*)self->base.node.next;
}

static TREE_INLINE tree_decl* tree_get_prev_decl(const tree_decl* self)
{
        return (tree_decl*)self->base.node.prev;
}

static TREE_INLINE tree_decl_kind tree_get_decl_kind(const tree_decl* self)
{
        return self->base.kind;
}

static TREE_INLINE bool tree_decl_is(const tree_decl* self, tree_decl_kind k)
{
        return tree_get_decl_kind(self) == k;
}

static TREE_INLINE tree_decl* tree_get_first_decl_in_scope(const tree_decl_scope* self, tree_decl_kind k)
{
        TREE_FOREACH_DECL_IN_SCOPE(self, it)
                if (tree_decl_is(it, k))
                        return it;
        return NULL;
}

static TREE_INLINE tree_decl_scope* tree_get_decl_scope(const tree_decl* self)
{
        return self->base.scope;
}

static TREE_INLINE bool tree_decl_is_global(const tree_decl* self)
{
        return tree_get_decl_scope_parent(tree_get_decl_scope(self)) == NULL;
}

static TREE_INLINE tree_xlocation tree_get_decl_loc(const tree_decl* self)
{
        return self->base.loc;
}

static TREE_INLINE tree_location tree_get_decl_loc_begin(const tree_decl* self)
{
        return tree_get_decl_loc(self).begin;
}

static TREE_INLINE tree_location tree_get_decl_loc_end(const tree_decl* self)
{
        return tree_get_decl_loc(self).end;
}

static TREE_INLINE bool tree_decl_is_implicit(const tree_decl* self)
{
        return self->base.is_implicit;
}

static TREE_INLINE void tree_set_decl_scope(tree_decl* self, tree_decl_scope* scope)
{
        self->base.scope = scope;
}

static TREE_INLINE void tree_set_decl_kind(tree_decl* self, tree_decl_kind k)
{
        self->base.kind = k;
}

static TREE_INLINE void tree_set_decl_loc(tree_decl* self, tree_xlocation l)
{
        self->base.loc = l;
}

static TREE_INLINE void tree_set_decl_loc_begin(tree_decl* self, tree_location l)
{
        self->base.loc.begin = l;
}

static TREE_INLINE void tree_set_decl_loc_end(tree_decl* self, tree_location l)
{
        self->base.loc.end = l;
}

static TREE_INLINE void tree_set_decl_implicit(tree_decl* self, bool v)
{
        self->base.is_implicit = v;
}

extern tree_decl* tree_new_named_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        size_t size);

static TREE_INLINE tree_id tree_get_decl_name(const tree_decl* self)
{
        return self->named.name;
}

static TREE_INLINE bool tree_decl_is_anon(const tree_decl* self)
{
        return tree_get_decl_name(self) == TREE_EMPTY_ID;
}

static TREE_INLINE void tree_set_decl_name(tree_decl* self, tree_id name)
{
        self->named.name = name;
}

extern tree_decl* tree_new_typed_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        size_t size);

static TREE_INLINE tree_type* tree_get_decl_type(const tree_decl* self)
{
        return self->typed.type;
}

static TREE_INLINE void tree_set_decl_type(tree_decl* self, tree_type* type)
{
        self->typed.type = type;
}

extern tree_decl* tree_new_value_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_storage_class sc,
        tree_storage_duration sd,
        tree_dll_storage_class dll_sc,
        tree_type* type,
        size_t size);

static TREE_INLINE tree_storage_class tree_get_decl_storage_class(const tree_decl* self)
{
        return self->value.storage_class;
}

static TREE_INLINE tree_dll_storage_class tree_get_decl_dll_storage_class(const tree_decl* self)
{
        return self->value.dll_storage_class;
}

static TREE_INLINE tree_storage_duration tree_get_decl_storage_duration(const tree_decl* self)
{
        return self->value.storage_duration;
}

static TREE_INLINE void tree_set_decl_storage_class(tree_decl* self, tree_storage_class sc)
{
        self->value.storage_class = sc;
}

static TREE_INLINE void tree_set_decl_dll_storage_class(tree_decl* self, tree_dll_storage_class sc)
{
        self->value.dll_storage_class = sc;
}

static TREE_INLINE void tree_set_decl_storage_duration(tree_decl* self, tree_storage_duration sd)
{
        self->value.storage_duration = sd;
}

extern tree_decl* tree_new_typedef_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type);

extern tree_decl* tree_new_tag_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        size_t size);

static TREE_INLINE bool tree_decl_is_tag(const tree_decl* self)
{
        return tree_decl_is(self, TDK_RECORD) || tree_decl_is(self, TDK_ENUM);
}

static TREE_INLINE bool tree_tag_decl_is_complete(const tree_decl* self)
{
        return self->tag.complete;
}

static TREE_INLINE void tree_set_tag_decl_complete(tree_decl* self, bool complete)
{
        self->tag.complete = complete;
}

extern tree_decl* tree_new_record_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        bool is_union);

static TREE_INLINE tree_decl* tree_get_record_fields_begin(const tree_decl* self)
{
        return tree_get_decl_scope_decls_begin(&self->record.fields);
}

static TREE_INLINE tree_decl* tree_get_record_fields_end(tree_decl* self)
{
        return tree_get_decl_scope_decls_end(&self->record.fields);
}

static TREE_INLINE const tree_decl* tree_get_record_fields_cend(const tree_decl* self)
{
        return tree_get_decl_scope_decls_cend(&self->record.fields);
}

static TREE_INLINE tree_decl_scope* tree_get_record_fields(tree_decl* self)
{
        return &self->record.fields;
}

static TREE_INLINE const tree_decl_scope* tree_get_record_cfields(const tree_decl* self)
{
        return &self->record.fields;
}

static TREE_INLINE bool tree_record_is_union(const tree_decl* self)
{
        return self->record.is_union;
}

static TREE_INLINE void tree_set_record_union(tree_decl* self, bool val)
{
        self->record.is_union = val;
}

extern tree_decl* tree_new_enum_decl(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc, tree_id name);

static TREE_INLINE tree_decl_scope* tree_get_enum_values(tree_decl* self)
{
        return &self->enum_decl.values;
}

static TREE_INLINE const tree_decl_scope* tree_get_enum_cvalues(const tree_decl* self)
{
        return &self->enum_decl.values;
}

extern tree_decl* tree_new_func_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_storage_class sc,
        tree_dll_storage_class dll_sc,
        tree_type* type,
        tree_stmt* body);

static TREE_INLINE bool tree_func_is_inlined(const tree_decl* self)
{
        return self->func.inlined;
}

static TREE_INLINE tree_decl_scope* tree_get_func_params(tree_decl* self)
{
        return &self->func.params;
}

static TREE_INLINE const tree_decl_scope* tree_get_func_cparams(const tree_decl* self)
{
        return &self->func.params;
}

static TREE_INLINE tree_decl_scope* tree_get_func_labels(tree_decl* self)
{
        return &self->func.labels;
}

static TREE_INLINE const tree_decl_scope* tree_get_func_clabels(const tree_decl* self)
{
        return &self->func.labels;
}

static TREE_INLINE tree_stmt* tree_get_func_body(const tree_decl* self)
{
        return self->func.body;
}

static TREE_INLINE tree_function_builtin_kind tree_get_func_builtin_kind(const tree_decl* self)
{
        return self->func.builtin_kind;
}

static TREE_INLINE bool tree_func_is_builtin(const tree_decl* self)
{
        return tree_get_func_builtin_kind(self) != TFBK_ORDINARY;
}

static TREE_INLINE void tree_set_func_inlined(tree_decl* self, bool inlined)
{
        self->func.inlined = inlined;
}

static TREE_INLINE void tree_set_func_body(tree_decl* self, tree_stmt* body)
{
        self->func.body = body;
}

static TREE_INLINE void tree_set_func_builtin_kind(tree_decl* self, tree_function_builtin_kind kind)
{
        self->func.builtin_kind = kind;
}

extern tree_decl* tree_new_var_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name, 
        tree_storage_class sc,
        tree_storage_duration sd,
        tree_dll_storage_class dll_sc,
        tree_type* type,
        tree_expr* init);

extern tree_decl* tree_new_param_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type);

static TREE_INLINE tree_expr* tree_get_var_init(const tree_decl* self)
{
        return self->var.init;
}

static TREE_INLINE void tree_set_var_init(tree_decl* self, tree_expr* init)
{
        self->var.init = init;
}

extern tree_decl* tree_new_field_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* bit_width);

extern uint tree_get_field_index(tree_decl* self);

static TREE_INLINE tree_decl* tree_get_field_record(const tree_decl* self)
{
        return (tree_decl*)((uint8_t*)tree_get_decl_scope(self)
                - offsetof(struct _tree_record_decl, fields));
}

static TREE_INLINE tree_expr* tree_get_field_bit_width(const tree_decl* self)
{
        return self->field.bit_width;
}

static TREE_INLINE void tree_set_field_bit_width(tree_decl* self, tree_expr* bit_width)
{
        self->field.bit_width = bit_width;
}

extern tree_decl* tree_new_indirect_field_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_decl* anon_record);

static TREE_INLINE tree_decl* tree_get_indirect_field_anon_record(const tree_decl* self)
{
        return self->indirect_field.anon_record;
}

static TREE_INLINE void tree_set_indirect_field_anon_record(tree_decl* self, tree_decl* anon)
{
        self->indirect_field.anon_record = anon;
}

extern tree_decl* tree_new_enumerator_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* expr,
        const int_value* val);

static TREE_INLINE tree_expr* tree_get_enumerator_expr(const tree_decl* self)
{
        return self->enumerator.expr;
}

static TREE_INLINE const int_value* tree_get_enumerator_cvalue(const tree_decl* self)
{
        return &self->enumerator.value;
}

static TREE_INLINE void tree_set_enumerator_expr(tree_decl* self, tree_expr* expr)
{
        self->enumerator.expr = expr;
}

static TREE_INLINE void tree_set_enumerator_value(tree_decl* self, const int_value* val)
{
        self->enumerator.value = *val;
}

extern tree_decl* tree_new_label_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_stmt* stmt);

static TREE_INLINE tree_stmt* tree_get_label_decl_stmt(const tree_decl* self)
{
        return self->label.stmt;
}

static TREE_INLINE void tree_set_label_decl_stmt(tree_decl* self, tree_stmt* stmt)
{
        self->label.stmt = stmt;
}

extern tree_decl* tree_new_decl_group(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc);

extern errcode tree_add_decl_in_group(tree_decl* self, tree_context* context, tree_decl* decl);

static TREE_INLINE tree_decl** tree_get_decl_group_begin(const tree_decl* self)
{
        return (tree_decl**)self->group.decls.data;
}

static TREE_INLINE tree_decl** tree_get_decl_group_end(const tree_decl* self)
{
        return tree_get_decl_group_begin(self) + self->group.decls.size;
}

static TREE_INLINE size_t tree_get_decl_group_size(const tree_decl* self)
{
        return self->group.decls.size;
}

static TREE_INLINE tree_decl* tree_get_decl_group_decl(const tree_decl* self, size_t i)
{
        return tree_get_decl_group_begin(self)[i];
}

#define TREE_FOREACH_DECL_IN_GROUP(PGROUP, ITNAME) \
        for (tree_decl** ITNAME = tree_get_decl_group_begin(PGROUP); \
                ITNAME != tree_get_decl_group_end(PGROUP); ITNAME++)

#ifdef __cplusplus
}
#endif

#endif // !TREE_DECL_H
