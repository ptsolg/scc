#include "tree-decl.h"
#include "tree-context.h"

extern void tree_symtab_init(tree_symtab* self, tree_context* context, tree_symtab* parent)
{
        htab_init_ex(&self->_symbols, tree_get_context_allocator(context));
        self->_parent = parent;
}

extern void tree_symtab_dispose(tree_symtab* self)
{
        htab_dispose(&self->_symbols);
}

extern serrcode tree_symtab_insert(tree_symtab* self, tree_decl* symbol)
{
        return tree_id_is_empty(tree_get_decl_name(symbol))
                ? S_NO_ERROR
                : htab_insert(&self->_symbols, tree_get_decl_name(symbol), symbol);
}

extern tree_decl* tree_symtab_get(const tree_symtab* self, tree_id name, bool parent_lookup)
{
        const tree_symtab* it = self;
        while (it)
        {
                tree_decl* d = htab_find(&it->_symbols, name);
                if (d || !parent_lookup)
                        return d;

                it = tree_get_symtab_parent(it);
        }
        return NULL;
}

extern void tree_init_decl_scope(
        tree_decl_scope* self, tree_context* context, tree_decl_scope* parent)
{
        tree_symtab_init(&self->_symtab, context, parent ? &parent->_symtab : NULL);
        list_init(&self->_decls);
        self->_parent = parent;
        self->_ndecls = 0;
}

extern tree_decl_scope* tree_new_decl_scope(tree_context* context, tree_decl_scope* parent)
{
        tree_decl_scope* ds = tree_context_fast_allocate(context, sizeof(*ds));
        if (!ds)
                return NULL;

        tree_init_decl_scope(ds, context, parent);
        return ds;
}

extern void tree_dispose_decl_scope(tree_decl_scope* self)
{
        tree_symtab_dispose(&self->_symtab);
        //...
}

extern serrcode tree_decl_scope_insert(tree_decl_scope* self, tree_decl* decl)
{
        S_ASSERT(decl);

        if (S_FAILED(tree_symtab_insert(&self->_symtab, decl)))
                return S_ERROR;

        list_push_back(&self->_decls, &_tree_get_decl(decl)->_node);
        self->_ndecls++;
        return S_NO_ERROR;
}

extern tree_decl* tree_new_decl(
        tree_context*    context,
        tree_decl_kind   kind,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        ssize            size)
{
        tree_decl* d = tree_context_fast_allocate(context, size);
        if (!d)
                return NULL;

        list_node_init(&_tree_get_decl(d)->_node);
        tree_set_decl_implicit(d, false);
        tree_set_decl_kind(d, kind);
        tree_set_decl_loc(d, loc);
        tree_set_decl_scope(d, scope);
        return d;
}

extern tree_decl* tree_new_named_decl(
        tree_context*    context,
        tree_decl_kind   kind,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        ssize            size)
{
        tree_decl* d = tree_new_decl(context, kind, scope, loc, size);
        if (!d)
                return NULL;

        tree_set_decl_name(d, name);
        return d;
}

extern tree_decl* tree_new_typed_decl(
        tree_context*    context,
        tree_decl_kind   kind,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        tree_type*       type,
        ssize            size)
{
        tree_decl* d = tree_new_named_decl(context, kind, scope, loc, name, size);
        if (!d)
                return NULL;

        tree_set_decl_type(d, type);
        return d;
}

extern tree_decl* tree_new_value_decl(
        tree_context*           context,
        tree_decl_kind          kind,
        tree_decl_scope*        scope,
        tree_xlocation          loc,
        tree_id                 name,
        tree_decl_storage_class class_,
        tree_type*              type,
        ssize                   size)
{
        tree_decl* d = tree_new_typed_decl(context, kind, scope, loc, name, type, size);
        if (!d)
                return NULL;

        tree_set_decl_storage_class(d, class_);
        return d;
}

extern tree_decl* tree_new_typedef_decl(
        tree_context*    context,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        tree_type*       type)
{
        return tree_new_typed_decl(context,
                TDK_TYPEDEF, scope, loc, name, type, sizeof(struct _tree_typedef_decl));
}

extern tree_decl* tree_new_record_decl(
        tree_context*    context,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        bool             is_union)
{
        tree_decl* d = tree_new_named_decl(context,
                TDK_RECORD, scope, loc, name, sizeof(struct _tree_record_decl));
        if (!d)
                return NULL;

        tree_set_record_union(d, is_union);
        tree_set_record_complete(d, false);
        tree_init_decl_scope(tree_get_record_scope(d), context, scope);
        return d;
}

extern tree_decl* tree_get_record_begin(tree_decl* self)
{
        const tree_decl_scope* scope = tree_get_record_cscope(self);
        TREE_DECL_SCOPE_FOREACH(scope, it)
                if (tree_decl_is(it, TDK_MEMBER))
                        return it;

        return tree_get_record_end(self);
}

extern const tree_decl* tree_get_record_cbegin(const tree_decl* self)
{
        const tree_decl_scope* scope = tree_get_record_cscope(self);
        TREE_DECL_SCOPE_FOREACH(scope, it)
                if (tree_decl_is(it, TDK_MEMBER))
                        return it;

        return tree_get_record_cend(self);
}

extern tree_decl* tree_get_record_end(tree_decl* self)
{
        return tree_get_decl_scope_end(tree_get_record_scope(self));
}

extern const tree_decl* tree_get_record_cend(const tree_decl* self)
{
        return tree_get_decl_scope_cend(tree_get_record_cscope(self));
}

extern const tree_decl* tree_get_next_cmember(const tree_decl* member, const tree_decl* record)
{
        const tree_decl* end = tree_get_record_cend(record);
        if (member == end)
                return end;

        while (1)
        {
                member = tree_get_next_decl(member);
                if (member == end)
                        return end;

                if (tree_get_decl_kind(member) == TDK_MEMBER)
                        return member;
        }
}

extern tree_decl* tree_get_next_member(tree_decl* member, tree_decl* record)
{
        tree_decl* end = tree_get_record_end(record);
        if (member == end)
                return end;

        while (1)
        {
                member = tree_get_next_decl(member);
                if (member == end)
                        return end;

                if (tree_get_decl_kind(member) == TDK_MEMBER)
                        return member;
        }
}

extern tree_decl* tree_new_enum_decl(
        tree_context* context, tree_decl_scope* scope, tree_xlocation loc, tree_id name)
{
        tree_decl* d = tree_new_named_decl(context,
                TDK_ENUM, scope, loc, name, sizeof(struct _tree_enum_decl));
        if (!d)
                return NULL;

        tree_init_decl_scope(tree_get_enum_scope(d), context, scope);
        return d;
}

extern tree_decl* tree_new_function_decl(
        tree_context*                context,
        tree_decl_scope*             scope,
        tree_xlocation               loc,
        tree_id                      name,
        tree_decl_storage_class      class_,
        tree_type*                   type,
        tree_function_specifier_kind spec,
        tree_stmt*                   body)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_FUNCTION, scope, loc, name, class_, type, sizeof(struct _tree_function_decl));
        if (!d)
                return NULL;

        tree_init_decl_scope(tree_get_function_params(d), context, scope);
        tree_init_decl_scope(tree_get_function_labels(d), context, NULL);
        tree_set_function_specifier(d, spec);
        tree_set_function_body(d, body);
        return d;
}

extern tree_decl* tree_new_var_decl(
        tree_context*           context,
        tree_decl_scope*        scope,
        tree_xlocation          loc,
        tree_id                 name,
        tree_decl_storage_class class_,
        tree_type*              type,
        tree_exp*               init)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_VAR, scope, loc, name, class_, type, sizeof(struct _tree_var_decl));
        if (!d)
                return NULL;

        tree_set_var_init(d, init);
        return d;
}

extern tree_decl* tree_new_member_decl(
        tree_context*    context,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        tree_type*       type,
        tree_exp*        bits)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_MEMBER, scope, loc, name, TDSC_NONE, type, sizeof(struct _tree_member_decl));
        if (!d)
                return NULL;

        tree_set_member_bits(d, bits);
        return d;
}

extern tree_decl* tree_new_enumerator_decl(
        tree_context*    context,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        tree_type*       type,
        tree_exp*        value)
{
        tree_decl* d = tree_new_typed_decl(context,
                TDK_ENUMERATOR, scope, loc, name, type, sizeof(struct _tree_enumerator_decl));
        if (!d)
                return NULL;

        tree_set_enumerator_value(d, value);
        return d;
}

extern tree_decl* tree_new_label_decl(
        tree_context*    context,
        tree_decl_scope* scope,
        tree_xlocation   loc,
        tree_id          name,
        tree_stmt*       stmt)
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

        objgroup_init_ex(&_tree_get_decl_group(d)->_group, tree_get_context_allocator(context));
        return d;
}

extern serrcode tree_decl_group_add(tree_decl* self, tree_decl* d)
{
        S_ASSERT(d);
        S_ASSERT(tree_get_decl_kind(d) != TDK_GROUP && "Decl group cannot contain other decl group");
        return objgroup_push_back(&_tree_get_decl_group(self)->_group, d);
}