#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-type.h"
#include "scc/tree/tree-context.h"

extern void tree_init_decl_scope(
        tree_decl_scope* self, tree_context* context, tree_decl_scope* parent)
{
        self->_parent = parent;
        htab_init_ex_ptr(&self->_lookup, tree_get_allocator(context));
        list_init(&self->_decls);
}

extern tree_decl_scope* tree_new_decl_scope(tree_context* context, tree_decl_scope* parent)
{
        tree_decl_scope* s = tree_allocate(context, sizeof(*s));
        if (!s)
                return NULL;

        tree_init_decl_scope(s, context, parent);
        return s;
}

extern bool tree_decl_scopes_are_same(const tree_decl_scope* a, const tree_decl_scope* b)
{
        if (a == b)
                return true;
        if (tree_get_decl_scope_parent(a) != tree_get_decl_scope_parent(b))
                return false;
 
        const htab* alookup = &a->_lookup;
        const htab* blookup = &b->_lookup;
        if (htab_size(alookup) != htab_size(blookup))
                return false;

        ssize matches = 0;
        HTAB_FOREACH(alookup, ait)
        {
                tree_id akey = hiter_get_key(&ait);
                hiter bit;
                if (!htab_find(blookup, akey, &bit))
                        return false;

                if (!tree_decls_are_same(hiter_get_ptr(&ait), hiter_get_ptr(&bit)))
                        return false;

                matches++;
        }
        return matches == htab_size(alookup);
}

extern void tree_decl_scope_add_lookup(tree_decl_scope* self, hval key, tree_decl* decl)
{
        S_ASSERT(decl);
        htab_insert_ptr(&self->_lookup, key, decl);
}

extern void tree_decl_scope_add_hidden(tree_decl_scope* self, tree_decl* decl)
{
        S_ASSERT(decl);
        list_push_back(&self->_decls, &_tree_get_decl(decl)->_node);
}

extern void tree_decl_scope_add(tree_decl_scope* self, hval key, tree_decl* decl)
{
        tree_decl_scope_add_lookup(self, key, decl);
        tree_decl_scope_add_hidden(self, decl);
}

extern tree_decl* tree_decl_scope_lookup(
        const tree_decl_scope* self, hval key, bool parent_lookup)
{
        const tree_decl_scope* it = self;
        while (it)
        {
                tree_decl* d = NULL;
                hiter res;
                if (htab_find(&it->_lookup, key, &res))
                        d = hiter_get_ptr(&res);

                if (d || !parent_lookup)
                        return d;

                it = tree_get_decl_scope_parent(it);
        }
        return NULL;
}

extern tree_decl* tree_new_decl(
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        ssize size)
{
        tree_decl* d = tree_allocate(context, size);
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
        tree_context* context,
        tree_decl_kind kind,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        ssize size)
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
        ssize size)
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
        tree_decl_storage_class class_,
        tree_type* type,
        ssize size)
{
        tree_decl* d = tree_new_typed_decl(context, kind, scope, loc, name, type, size);
        if (!d)
                return NULL;

        tree_set_decl_storage_class(d, class_);
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

extern tree_decl* tree_new_record_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        bool is_union)
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

extern tree_decl* tree_get_record_members_begin(tree_decl* self)
{
        const tree_decl_scope* scope = tree_get_record_cscope(self);
        TREE_FOREACH_DECL_IN_SCOPE(scope, it)
                if (tree_decl_is(it, TDK_MEMBER))
                        return it;

        return tree_get_record_members_end(self);
}

extern const tree_decl* tree_get_record_members_cbegin(const tree_decl* self)
{
        const tree_decl_scope* scope = tree_get_record_cscope(self);
        TREE_FOREACH_DECL_IN_SCOPE(scope, it)
                if (tree_decl_is(it, TDK_MEMBER))
                        return it;

        return tree_get_record_members_cend(self);
}

extern tree_decl* tree_get_record_members_end(tree_decl* self)
{
        return tree_get_decl_scope_decls_end(tree_get_record_scope(self));
}

extern const tree_decl* tree_get_record_members_cend(const tree_decl* self)
{
        return tree_get_decl_scope_decls_cend(tree_get_record_cscope(self));
}

extern tree_decl* tree_get_next_member(const tree_decl* member)
{
        tree_decl* end = tree_get_decl_scope_decls_end(tree_get_decl_scope(member));
        if (member == end)
                return end;

        for (tree_decl* it = tree_get_next_decl(member);
                it != end; it = tree_get_next_decl(member))
        {
                if (tree_decl_is(it, TDK_MEMBER))
                        return it;
        }
        return end;
}

extern tree_decl* tree_get_prev_member(const tree_decl* member)
{
        tree_decl* end = tree_get_decl_scope_decls_end(tree_get_decl_scope(member));
        if (member == end)
                return end;

        for (tree_decl* it = tree_get_next_decl(member);
                it != end; it = tree_get_prev_decl(member))
        {
                if (tree_decl_is(it, TDK_MEMBER))
                        return it;
        }
        return end;
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
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_decl_storage_class class_,
        tree_type* type,
        tree_function_specifier_kind spec,
        tree_stmt* body)
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
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_decl_storage_class class_,
        tree_type* type,
        tree_expr* init)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_VAR, scope, loc, name, class_, type, sizeof(struct _tree_var_decl));
        if (!d)
                return NULL;

        tree_set_var_init(d, init);
        return d;
}

extern tree_decl* tree_new_member_decl(
        tree_context* context,
        tree_decl_scope* scope,
        tree_xlocation loc,
        tree_id name,
        tree_type* type,
        tree_expr* bits)
{
        tree_decl* d = tree_new_value_decl(context,
                TDK_MEMBER, scope, loc, name, TDSC_NONE, type, sizeof(struct _tree_member_decl));
        if (!d)
                return NULL;

        tree_set_member_bits(d, bits);
        _tree_get_member(d)->_index = -1;
        return d;
}

extern uint tree_get_member_index(tree_decl* self)
{
        uint index = _tree_get_cmember(self)->_index;
        if (index != -1)
                return index;

        tree_decl* rec = tree_get_member_parent(self);
        tree_decl* end = tree_get_record_members_end(rec);

        index = 0;
        for (tree_decl* it = tree_get_record_members_begin(rec);
                it != end; it = tree_get_next_member(it))
        {
                _tree_get_member(it)->_index = index++;
        }

        return _tree_get_member(self)->_index; 
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

        dseq_init_ex_ptr(&_tree_get_decl_group(d)->_group,
                tree_get_allocator(context));
        return d;
}

extern serrcode tree_decl_group_add(tree_decl* self, tree_decl* d)
{
        S_ASSERT(d);
        S_ASSERT(tree_get_decl_kind(d) != TDK_GROUP && "Decl group cannot contain other decl group");
        return dseq_append_ptr(&_tree_get_decl_group(self)->_group, d);
}

extern bool tree_decls_have_same_name(const tree_decl* a, const tree_decl* b)
{
        return tree_get_decl_name(a) == tree_get_decl_name(b);
}

extern bool tree_decls_have_same_linkage(const tree_decl* a, const tree_decl* b)
{
        tree_decl_storage_class asc = tree_get_decl_storage_class(a);
        tree_decl_storage_class bsc = tree_get_decl_storage_class(b);
        if (asc == TDSC_EXTERN && bsc == TDSC_IMPL_EXTERN)
                return true;
        if (bsc == TDSC_EXTERN && asc == TDSC_IMPL_EXTERN)
                return true;
        return asc == bsc;
}

extern bool tree_decls_are_same(const tree_decl* a, const tree_decl* b)
{
        if (a == b)
                return true;

        if (!a || !b)
                return false;

        if (!tree_decls_have_same_name(a, b))
                return false;

        tree_decl_kind k = tree_get_decl_kind(a);
        if (k != tree_get_decl_kind(b))
                return false;

        if (k == TDK_RECORD)
                return tree_decl_scopes_are_same(
                        tree_get_record_cscope(a), tree_get_record_cscope(b));
        else if (k == TDK_ENUM) // todo: sort enumerators by value
                return tree_decl_scopes_are_same(
                        tree_get_enum_cscope(a), tree_get_enum_cscope(b));
        else if (k == TDK_MEMBER)
                ; // todo: bit size
        else if (k == TDK_ENUMERATOR)
        {
                ; // todo: value
                return true;
        }
        else if (k == TDK_GROUP)
        {
                ; // todo:
                S_UNREACHABLE();
                return true;
        }
        else if (k == TDK_LABEL)
                return true;

        return tree_types_are_same(tree_get_decl_type(a), tree_get_decl_type(b));
}