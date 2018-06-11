#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-type.h"
#include "scc/tree/tree-context.h"

extern void tree_init_decl_scope(
        tree_decl_scope* self, tree_context* context, tree_decl_scope* parent)
{
        tree_set_decl_scope_parent(self, parent);
        list_init(&self->decls);
        self->lookup[TLK_DECL] = NULL;
        self->lookup[TLK_TAG] = NULL;
}

extern tree_decl_scope* tree_new_decl_scope(tree_context* context, tree_decl_scope* parent)
{
        tree_decl_scope* ds = tree_allocate_node(context, sizeof(tree_decl_scope));
        if (!ds)
                return NULL;

        tree_init_decl_scope(ds, context, parent);
        return ds;
}

static TREE_INLINE tree_decl* _tree_decl_scope_lookup(
        const tree_decl_scope* self,
        tree_lookup_kind lookup_kind,
        tree_id id)
{
        strmap_entry* e;
        if (lookup_kind == TLK_DECL || lookup_kind == TLK_TAG)
                return self->lookup[lookup_kind] && (e = strmap_lookup(self->lookup[lookup_kind], id))
                        ? e->value : NULL;

        assert(lookup_kind == TLK_ANY);

        if (self->lookup[TLK_DECL] && (e = strmap_lookup(self->lookup[TLK_DECL], id)))
                return e->value;

        if (self->lookup[TLK_TAG] && (e = strmap_lookup(self->lookup[TLK_TAG], id)))
                return e->value;

        return NULL;
}

extern tree_decl* tree_decl_scope_lookup(
        const tree_decl_scope* self,
        tree_lookup_kind lookup_kind,
        tree_id id,
        bool parent_lookup)
{
        const tree_decl_scope* it = self;
        while (it)
        {
                tree_decl* d = _tree_decl_scope_lookup(it, lookup_kind, id);
                if (d || !parent_lookup)
                        return d;

                it = tree_get_decl_scope_parent(it);
        }
        return NULL;
}

extern errcode tree_decl_scope_update_lookup(tree_decl_scope* self, tree_context* context, tree_decl* decl)
{
        assert(decl && tree_get_decl_scope(decl) == self);

        strmap** p = tree_decl_is_tag(decl)
                ? self->lookup + TLK_TAG
                : self->lookup + TLK_DECL;
        if (!*p)
        {
                if (!(*p = tree_allocate_node(context, sizeof(strmap))))
                        return EC_ERROR;
                strmap_init_ex(*p, context->alloc);
        }

        return strmap_insert(*p, tree_get_decl_name(decl), decl);
}

extern errcode tree_decl_scope_add_decl(tree_decl_scope* self, tree_context* context, tree_decl* decl)
{
        if (!tree_decl_is_anon(decl) && EC_FAILED(tree_decl_scope_update_lookup(self, context, decl)))
                return EC_ERROR;
        tree_decl_scope_add_hidden_decl(self, decl);
        return EC_NO_ERROR;
}

extern void tree_decl_scope_add_hidden_decl(tree_decl_scope* self, tree_decl* decl)
{
        assert(decl && tree_get_decl_scope(decl) == self);

        list_push_back(&self->decls, &decl->base.node);
}

extern tree_decl* tree_new_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        size_t size)
{
        tree_decl* d = tree_allocate_node(context, size);
        if (!d)
                return NULL;

        list_node_init(&d->base.node);
        tree_set_decl_implicit(d, false);
        tree_set_decl_kind(d, kind);
        tree_set_decl_loc(d, loc);
        tree_set_decl_scope(d, scope);
        return d;
}

extern tree_decl* tree_new_named_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        size_t size)
{
        tree_decl* d = tree_new_decl(context, kind, scope, loc, size);
        if (!d)
                return NULL;

        tree_set_decl_name(d, name);
        return d;
}

extern tree_decl* tree_new_typed_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        size_t size)
{
        tree_decl* d = tree_new_named_decl(context, kind, scope, loc, name, size);
        if (!d)
                return NULL;

        tree_set_decl_type(d, type);
        return d;
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
        size_t size)
{
        tree_decl* d = tree_new_typed_decl(context, kind, scope, loc, name, type, size);
        if (!d)
                return NULL;

        tree_set_decl_storage_class(d, sc);
        tree_set_decl_storage_duration(d, sd);
        tree_set_decl_dll_storage_class(d, dll_sc);
        return d;
}

extern tree_decl* tree_new_typedef_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type)
{
        return tree_new_typed_decl(context,
                TDK_TYPEDEF, scope, loc, name, type, sizeof(struct _tree_typedef_decl));
}

extern tree_decl* tree_new_tag_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        size_t size)
{
        tree_decl* d = tree_new_named_decl(context, kind, scope, loc, name, size);
        if (!d)
                return NULL;

        tree_set_tag_decl_complete(d, false);
        return d;
}

extern tree_decl* tree_new_record_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        bool is_union)
{
        tree_decl* d = tree_new_tag_decl(context,
                TDK_RECORD, scope, loc, name, sizeof(struct _tree_record_decl));
        if (!d)
                return NULL;

        tree_set_record_union(d, is_union);
        tree_init_decl_scope(tree_get_record_fields(d), context, scope);
        return d;
}

extern tree_decl* tree_new_enum_decl(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc, tree_id name)
{
        tree_decl* d = tree_new_tag_decl(context,
                TDK_ENUM, scope, loc, name, sizeof(struct _tree_enum_decl));
        if (!d)
                return NULL;

        tree_init_decl_scope(tree_get_enum_values(d), context, scope);
        return d;
}

extern tree_decl* tree_new_func_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_storage_class sc,
        tree_dll_storage_class dll_sc,
        tree_type* type,
        tree_stmt* body)
{
        tree_decl* d = tree_new_value_decl(context, TDK_FUNCTION, scope, loc, name,
                sc, TSD_AUTOMATIC, dll_sc, type, sizeof(struct _tree_function_decl));
        if (!d)
                return NULL;

        tree_init_decl_scope(tree_get_func_params(d), context, scope);
        tree_init_decl_scope(tree_get_func_labels(d), context, NULL);
        tree_set_func_inlined(d, false);
        tree_set_func_body(d, body);
        tree_set_func_builtin_kind(d, TFBK_ORDINARY);
        return d;
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
        tree_expr* init)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_VAR, scope, loc, name, sc, sd, dll_sc, type, sizeof(struct _tree_var_decl));
        if (!d)
                return NULL;

        tree_set_var_init(d, init);
        return d;
}

extern tree_decl* tree_new_param_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type)
{
        return tree_new_value_decl(context,
                TDK_PARAM, scope, loc, name, TSC_NONE, TSD_AUTOMATIC, TDSC_NONE, type, sizeof(struct _tree_var_decl));
}

extern tree_decl* tree_new_field_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* bit_width)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_FIELD, scope, loc, name, TSC_NONE, TSD_AUTOMATIC, TDSC_NONE, type, sizeof(struct _tree_field_decl));
        if (!d)
                return NULL;

        tree_set_field_bit_width(d, bit_width);
        d->field.index = -1;
        return d;
}

extern uint tree_get_field_index(tree_decl* self)
{
        uint index = self->field.index;
        if (index != -1)
                return index;

        index = 0;
        tree_decl_scope* fields = tree_get_record_fields(tree_get_field_record(self));
        TREE_FOREACH_DECL_IN_SCOPE(fields, it)
        {
                if (!tree_decl_is(it, TDK_FIELD))
                        continue;
                it->field.index = index++;
        }

        return self->field.index;
}

extern tree_decl* tree_new_indirect_field_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_decl* anon_record)
{
        tree_decl* d = tree_new_value_decl(context, TDK_INDIRECT_FIELD, scope, loc, name, 
                TSC_NONE, TSD_AUTOMATIC, TDSC_NONE, type, sizeof(struct _tree_indirect_field_decl));
        if (!d)
                return NULL;

        tree_set_decl_implicit(d, true);
        tree_set_indirect_field_anon_record(d, anon_record);
        return d;
}

extern tree_decl* tree_new_enumerator_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* expr,
        const int_value* val)
{
        tree_decl* d = tree_new_typed_decl(context,
                TDK_ENUMERATOR, scope, loc, name, type, sizeof(struct _tree_enumerator_decl));
        if (!d)
                return NULL;

        tree_set_enumerator_expr(d, expr);
        tree_set_enumerator_value(d, val);
        return d;
}

extern tree_decl* tree_new_label_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_stmt* stmt)
{
        tree_decl* d = tree_new_named_decl(context,
                TDK_LABEL, scope, loc, name, sizeof(struct _tree_label_decl));
        if (!d)
                return NULL;

        tree_set_label_decl_stmt(d, stmt);
        return d;
}

extern tree_decl* tree_new_decl_group(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc)
{
        tree_decl* d = tree_new_decl(context,
                TDK_GROUP, scope, loc, sizeof(struct _tree_decl_group));
        if (!d)
                return NULL;

        tree_init_array(&d->group.decls);
        return d;
}

extern errcode tree_add_decl_in_group(tree_decl* self, tree_context* context, tree_decl* decl)
{
        assert(decl);
        assert(tree_get_decl_kind(decl) != TDK_GROUP
                && "Decl group cannot contain other decl group");

        return tree_array_append_ptr(context, &self->group.decls, decl);
}


extern bool tree_decls_have_same_name(const tree_decl* a, const tree_decl* b)
{
        return tree_get_decl_name(a) == tree_get_decl_name(b);
}

extern bool tree_decls_have_same_linkage(const tree_decl* a, const tree_decl* b)
{
        tree_storage_class asc = tree_get_decl_storage_class(a);
        tree_storage_class bsc = tree_get_decl_storage_class(b);
        if (asc == TSC_EXTERN && bsc == TSC_IMPL_EXTERN)
                return true;
        if (bsc == TSC_EXTERN && asc == TSC_IMPL_EXTERN)
                return true;
        return asc == bsc;
}
