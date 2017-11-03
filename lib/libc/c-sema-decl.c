#include "c-sema-decl.h"
#include "c-sema-type.h"
#include "c-sema-conv.h"
#include "c-sema-expr.h"
#include <libtree/tree-eval.h>

extern void csema_init_declarator(csema* self, cdeclarator* declarator, cdeclarator_kind k)
{
        cdeclarator_init(declarator, self->ccontext, k);
}

// returns declarator type
static tree_type* csema_set_declarator_type(csema* self, cdeclarator* d, tree_type* t)
{
        if (!d->type.head)
        {
                d->type.head = t;
                d->type.tail = t;
                return t;
        }

        tree_type* tail = d->type.tail;
        tree_type_kind k = tree_get_type_kind(tail);
        tree_location loc = cdeclarator_get_id_loc_or_begin(d);

        if (k == TTK_POINTER)
        {
                if (!csema_set_pointer_target(self, tail, t))
                        return NULL;
        }
        else if (k == TTK_FUNCTION)
        {
                if (!csema_set_function_restype(self, loc, tail, t))
                        return NULL;
        }
        else if (k == TTK_ARRAY)
        {
                if (!csema_set_array_eltype(self, loc, tail, t))
                        return NULL;
        }
        else if (k == TTK_PAREN)
        {
                if (!csema_set_paren_type(self, tail, t))
                        return NULL;
        }
        else
                return NULL; // unkown type kind

        d->type.tail = t;
        return d->type.head;
}

extern bool csema_new_direct_declarator_suffix(
        csema* self, cdeclarator* declarator, tree_type* suffix)
{
        return (bool)csema_set_declarator_type(self, declarator, suffix);
}

extern bool csema_new_direct_declarator_function_suffix(
        csema* self, cdeclarator* declarator)
{
        tree_type* suffix = csema_new_function_type(self,
                cdeclarator_get_id_loc_or_begin(declarator), NULL);

        return csema_new_direct_declarator_suffix(self, declarator, suffix);
}

extern bool csema_new_direct_declarator_array_suffix(
        csema* self, cdeclarator* declarator, tree_type_quals quals, tree_expr* size)
{
        tree_type* suffix = csema_new_array_type(self,
                cdeclarator_get_id_loc_or_begin(declarator), quals, NULL, size);

        return csema_new_direct_declarator_suffix(self, declarator, suffix);
}

extern bool csema_add_direct_declarator_parens(csema* self, cdeclarator* declarator)
{
        return (bool)csema_set_declarator_type(self, declarator,
                csema_new_paren_type(self, NULL));
}

extern bool csema_set_declarator_name(
        csema* self, tree_location loc, cdeclarator* declarator, tree_id name)
{
        if (declarator->id != tree_get_empty_id())
                return false;

        cdeclarator_set_id(declarator, loc, name);
        return true;
}

extern bool csema_finish_declarator(
        csema* self, cdeclarator* declarator, ctype_chain* pointer_chain)
{
        if (!pointer_chain->head)
                return true;

        if (declarator->type.tail)
        {
                bool res = (bool)csema_set_declarator_type(self, declarator, pointer_chain->head);
                declarator->type.tail = pointer_chain->tail;
                return res;
        }

        declarator->type = *pointer_chain;
        return true;
}

static void csema_redefinition(const csema* self, tree_location loc, tree_id id)
{
        cerror(self->error_manager, CES_ERROR, loc, 
                "redefinition of '%s'", csema_get_id(self, id));
}

static void csema_decl_redifinition(const csema* self, const tree_decl* decl)
{
        csema_redefinition(self, tree_get_decl_loc_begin(decl), csema_get_decl_name(self, decl));
}

extern tree_decl* csema_export_decl(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (tree_id_is_empty(csema_get_decl_name(self, decl)))
                return decl;

        tree_symtab* symtab = tree_get_decl_scope_symtab(scope);
        if (tree_symtab_get(symtab, tree_get_decl_name(decl), false))
        {
                csema_decl_redifinition(self, decl);
                return NULL;
        }
        tree_symtab_insert(symtab, decl);
        return decl;
}

static bool csema_export_decl_scope(csema* self, tree_decl_scope* to, tree_decl_scope* from)
{
        tree_symtab* tab = tree_get_decl_scope_symtab(from);
        TREE_SYMTAB_FOREACH(tab, it)
                if (!csema_export_decl(self, to, hiter_get_val(it)))
                        return false;
        return true;
}

static tree_decl* csema_finish_member_decl(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        tree_type* dt = tree_desugar_type(tree_get_decl_type(decl));
        if (tree_type_is(dt, TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, tree_get_decl_loc_begin(decl),
                       "field '%s' declared as function",
                       csema_get_id(self, tree_get_decl_name(decl)));
                return NULL;
        }
        if (!tree_declared_type_is(dt, TDK_RECORD))
                return decl;

        tree_decl* record = tree_get_decl_type_entity(dt);
        tree_decl_scope* members = tree_get_record_scope(record);

        // if decl is unnamed struct member then add its members to local scope
        if (tree_decl_is_unnamed(decl))
        {
                if (!csema_export_decl_scope(self, self->locals, members))
                        return NULL;
        }
        return decl;
}

static bool csema_check_decl_with_no_linkage(const csema* self, const tree_decl* decl)
{
        tree_location loc = tree_get_xloc_begin(tree_get_decl_loc(decl));
        tree_type* dt = tree_get_decl_type(decl);

        if (tree_get_decl_kind(decl) != TDK_FUNCTION)
                return csema_require_complete_type(self, loc, dt);

        const tree_decl_scope* params = tree_get_function_cparams(decl);
        TREE_DECL_SCOPE_FOREACH(params, p)
        {
                tree_location ploc = tree_get_xloc_begin(tree_get_decl_loc(p));
                tree_type* ptype = tree_get_decl_type(p);
                if (!csema_require_complete_type(self, ploc, ptype))
                        return false;
        }

        dt = tree_get_function_restype(dt);
        return tree_type_is_void(dt)
                ? true : csema_require_complete_type(self, loc, dt);
}

static bool csema_check_decl_with_linkage(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        tree_decl_kind dk = tree_get_decl_kind(decl);
        S_ASSERT(dk != TDK_MEMBER);

        tree_location loc = tree_get_xloc_begin(tree_get_decl_loc(decl));
        tree_id name = tree_get_decl_name(decl);


        if (dk == TDK_FUNCTION && csema_at_block_scope(self))
        {
                tree_decl_storage_class sc = tree_get_decl_storage_class(decl);
                if (sc != TDSC_EXTERN && sc != TDSC_IMPL_EXTERN)
                {
                        cerror(self->error_manager, CES_ERROR, loc,
                               "invalid storage class for '%s'",
                               csema_get_id(self, name));
                        return false;
                }
        }

        tree_decl* orig = tree_decl_scope_find(scope, tree_get_decl_name(decl), false);
        if (!orig)
                return (bool)csema_export_decl(self, scope, decl);

        if (!tree_decls_have_same_linkage(decl, orig))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                       "redefinition of '%s' with different storage class",
                       csema_get_id(self, name));
                return false;
        }

        if (!tree_types_are_same(tree_get_decl_type(decl), tree_get_decl_type(orig)))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                       "conflicting types for '%s'", csema_get_id(self, name));
                return false;
        }
        return true;
}

static tree_decl* csema_finish_object_or_function_decl(
        csema* self, tree_decl_scope* scope, tree_decl* decl, bool allow_incomplete)
{
        // c99 6.7
        // If an identifier has no linkage, there shall be no more than one
        // declaration of the identifier (in a declarator or type specifier)
        // with the same scope and in the same name space
        // If an identifier for an object is declared with no linkage,
        // the type for the object shall be complete by the end of its declarator

        tree_decl_storage_class sc = tree_get_decl_storage_class(decl);
        if (sc == TDSC_NONE || sc == TDSC_IMPL_EXTERN)
                if (!allow_incomplete && !csema_check_decl_with_no_linkage(self, decl))
                        return NULL;

        if (sc == TDSC_NONE)
        {
                if (!csema_export_decl(self, scope, decl))
                        return NULL;
                if (tree_decl_is(decl, TDK_MEMBER))
                        if (!csema_finish_member_decl(self, scope, decl))
                                return NULL;
                return decl;
        }

        if (!csema_check_decl_with_linkage(self, scope, decl))
                return NULL;

        return decl;
}

static tree_decl* _csema_finish_decl(
        csema* self, tree_decl_scope* scope, tree_decl* decl, bool allow_incomplete)
{
        tree_decl_kind dk = tree_get_decl_kind(decl);
        if (dk == TDK_VAR || dk == TDK_FUNCTION || dk == TDK_MEMBER)
        {
                if (!csema_finish_object_or_function_decl(self, scope, decl, allow_incomplete))
                        return NULL;
        }
        else if (dk == TDK_RECORD)
                tree_set_record_complete(decl, true);
        else if (dk == TDK_ENUMERATOR)
                if (!csema_export_decl(self, self->globals, decl))
                        return NULL;

        tree_decl_scope_insert(scope, decl);
        return decl;
}

extern tree_type* csema_new_type_name(
        csema* self, cdeclarator* declarator, tree_type* typespec)
{
        return csema_set_declarator_type(self, declarator, typespec);
}

extern tree_decl* csema_finish_decl(csema* self, tree_decl* decl)
{
        return _csema_finish_decl(self, self->locals, decl, false);
}

extern tree_decl* csema_finish_decl_ex(csema* self, tree_location end_loc, tree_decl* decl)
{
        tree_decl* d = csema_finish_decl(self, decl);
        if (!d)
                return NULL;

        tree_set_decl_end_loc(d, end_loc);
        return d;
}

static void csema_compute_enumerator_value(
        csema* self, tree_decl* enum_, tree_decl* enumerator)
{
        int v = 0;
        tree_decl_scope* s = tree_get_enum_scope(enum_);

        if (tree_get_decl_scope_size(s))
        {
                tree_decl* last = tree_get_prev_decl(tree_get_decl_scope_end(s));
                int_value last_val;
                tree_eval_info i;
                bool r = tree_eval_as_integer(&i, tree_get_enumerator_value(last), &last_val);
                S_ASSERT(r);
                v = int_get_i32(&last_val) + 1;
        }

        tree_expr* val = csema_new_integer_literal(self, tree_get_decl_loc_begin(enumerator), v, true, false);
        tree_set_enumerator_value(enumerator, tree_new_impl_init_expr(self->context, val));
}

extern tree_decl* csema_new_enumerator(
        csema* self, tree_decl* enum_, tree_id id, tree_id id_loc, tree_expr* value)
{
        tree_type* t = tree_new_qual_type(self->context, TTQ_UNQUALIFIED,
                tree_new_decl_type(self->context, enum_, true));
        tree_decl* e = tree_new_enumerator_decl(
                self->context, self->locals, tree_init_xloc(id_loc, id_loc), id, t, value);

        if (!value)
        {
                csema_compute_enumerator_value(self, enum_, e);
                return e;
        }

        const char* name = csema_get_id(self, tree_get_decl_name(e));

        int_value val;
        tree_eval_info i;
        tree_init_eval_info(&i, self->target);
        if (!tree_eval_as_integer(&i, value, &val))
        {
                cerror(self->error_manager, CES_ERROR, id_loc,
                        "enumerator value for '%s' is not an integer constant", name);
                return NULL;
        }

        return e;
}

static void csema_wrong_kind_of_tag(const csema* self, tree_location loc, tree_id name)
{
        cerror(self->error_manager, CES_ERROR, loc,
                "'%s' defined as wrong kind of tag", csema_get_id(self, name));
}

extern tree_decl* csema_new_enum_decl(
        csema* self, tree_location kw_loc, tree_id name, bool has_body)
{
        tree_decl* e = csema_get_local_tag_decl(self, name, !has_body);
        if (!e)
        {
                e = tree_new_enum_decl(self->context,
                        self->locals, tree_init_xloc(kw_loc, kw_loc),
                        cident_info_to_tag(self->id_info, name));

                if (!csema_export_decl(self, self->globals, e))
                        return NULL;
        }
        else if (tree_get_decl_kind(e) != TDK_ENUM)
        {
                csema_wrong_kind_of_tag(self, kw_loc, name);
                return NULL;
        }
        
        if (has_body && tree_get_decl_scope_size(tree_get_enum_scope(e)))
        {
                csema_redefinition(self, kw_loc, name);
                return NULL;
        }

        return e;
}

static bool csema_check_specifiers(
        const csema* self, const cdecl_specs* specs, const cdeclarator* d, bool function)
{
        if (specs->funcspec == TFSK_INLINE && !function)
        {
                tree_location loc = cdeclarator_get_id_loc_or_begin(d);
                if (loc == TREE_INVALID_LOC)
                        loc = cdecl_specs_get_start_loc(specs);

                cerror(self->error_manager, CES_ERROR, loc,
                        "'inline' specifier allowed on function declarations only");
                return false;
        }
        return true;
}

extern tree_decl* csema_new_member_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* struct_declarator, tree_expr* bits)
{
        if (!csema_check_specifiers(self, decl_specs, struct_declarator, false))
                return NULL;

        tree_type* t = csema_set_declarator_type(self, struct_declarator, decl_specs->typespec);
        if (!t)
                return NULL;

        tree_id id = struct_declarator->id;
        tree_xlocation loc = decl_specs->loc;

        tree_decl* m = tree_new_member_decl(
                self->context, self->locals, loc, id, t, bits);
        if (!bits)
                return m;

        tree_location start_loc = tree_get_xloc_begin(loc);
        const char* name = csema_get_id(self, id);

        if (!tree_type_is_integer(t))
        {
                cerror(self->error_manager, CES_ERROR, start_loc,
                       "bit-field '%s' has invalid type", name);
                return NULL;
        }

        int_value val;
        tree_eval_info i;
        tree_init_eval_info(&i, self->target);
        if (!tree_eval_as_integer(&i, bits, &val))
        {
                cerror(self->error_manager, CES_ERROR, start_loc,
                        "bit-field '%s' width not an integer constant", name);
                return NULL;
        }

        if (int_is_zero(&val))
        {
                cerror(self->error_manager, CES_ERROR, start_loc,
                        "zero width for bit-field '%s'", name);
                return NULL;
        }

        if (int_is_signed(&val) && int_get_i64(&val) < 0)
        {
                cerror(self->error_manager, CES_ERROR, start_loc,
                       "negative width in bit-field '%s'", name);
                return NULL;
        }

        if (int_get_u64(&val) > 8 * tree_get_sizeof(self->target, t))
        {
                cerror(self->error_manager, CES_ERROR, start_loc,
                       "width of '%s' exceeds its type", name);
                return NULL;
        }
        return m;
}

extern tree_decl* csema_new_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union, bool has_body)
{
        tree_decl* d = csema_get_local_tag_decl(self, name, !has_body);
        if (!d)
        {
                d = tree_new_record_decl(self->context,
                        self->locals,
                        tree_init_xloc(kw_loc, kw_loc),
                        cident_info_to_tag(self->id_info, name),
                        is_union);

                if (!csema_export_decl(self, self->locals, d))
                        return NULL;
        }
        else if (!tree_decl_is(d, TDK_RECORD) || tree_record_is_union(d) != is_union)
        {
                csema_wrong_kind_of_tag(self, kw_loc, name);
                return NULL;
        }

        if (has_body && tree_record_is_complete(d))
        {
                csema_redefinition(self, kw_loc, name);
                return NULL;
        }

        return d;
}

static bool csema_add_function_param(csema* self, tree_decl* function, cparam* param)
{
        tree_decl* d = tree_new_var_decl(
                self->context,
                self->locals,
                cparam_get_loc(param),
                param->declarator.id,
                TDSC_NONE,
                param->declarator.type.head,
                NULL);
        
        tree_decl_storage_class sc = tree_get_decl_storage_class(function);
        return _csema_finish_decl(self, self->locals, d,
                sc == TDSC_EXTERN || sc == TDSC_STATIC);
}

static tree_decl* csema_new_function_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        // c99 6.2.2.5
        // If the declaration of an identifier for a function has no storage - class specifier,
        // its linkage is external
        tree_decl_storage_class sc = decl_specs->class_;
        if (sc == TDSC_NONE)
                sc = TDSC_IMPL_EXTERN;

        tree_type* ftype = declarator->type.head;
        tree_decl* func = tree_new_function_decl(
                self->context,
                self->locals,
                decl_specs->loc,
                declarator->id,
                sc,
                ftype,
                decl_specs->funcspec,
                NULL);
      
        csema_enter_decl_scope(self, tree_get_function_params(func));
        OBJGROUP_FOREACH(&declarator->params, cparam**, it)
                if (!csema_add_function_param(self, func, *it))
                {
                        func = NULL;
                        break;
                }
        csema_exit_decl_scope(self);

        return func;
}

static tree_decl* csema_new_typedef_decl(
        csema* self, cdecl_specs* specs, cdeclarator* d)
{
        if (!csema_check_specifiers(self, specs, d, false))
                return NULL;

        return tree_new_typedef_decl(
                self->context,
                self->locals,
                specs->loc,
                d->id,
                d->type.head);
}

static tree_decl* csema_new_var_decl(
        csema* self, cdecl_specs* specs, cdeclarator* d)
{
        if (!csema_check_specifiers(self, specs, d, false))
                return NULL;
        // c99 6.2.2.5
        // If the declaration of an identifier for an object has file scope
        // and no storage - class specifier, its linkage is external.
        tree_decl_storage_class sc = specs->class_;
        if (sc == TDSC_NONE && csema_at_file_scope(self))
                sc = TDSC_IMPL_EXTERN;

        return tree_new_var_decl(
                self->context,
                self->locals,
                specs->loc,
                d->id,
                sc,
                d->type.head,
                NULL);
}

extern tree_decl* csema_new_external_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        if (!csema_set_declarator_type(self, declarator, decl_specs->typespec))
                return NULL;

        if (decl_specs->is_typedef)
                return csema_new_typedef_decl(self, decl_specs, declarator);
        else if (tree_type_is(declarator->type.head, TTK_FUNCTION))
                return csema_new_function_decl(self, decl_specs, declarator);
        return csema_new_var_decl(self, decl_specs, declarator);
}

extern tree_decl* csema_add_init_declarator(csema* self, tree_decl* list, tree_decl* d)
{
        if (!list)
                return d;

        if (!tree_decl_is(list, TDK_GROUP))
        {
                tree_decl* first = list;
                tree_set_decl_implicit(first, true);
                list = tree_new_decl_group(self->context,
                        self->locals, tree_get_decl_loc(first));

                tree_decl_group_add(list, first);
                tree_decl_scope_insert(self->locals, list);
        }

        tree_decl_group_add(list, d);
        tree_set_decl_implicit(d, true);
        return list;
}

extern bool csema_set_var_initializer(csema* self, tree_decl* decl, tree_expr* init)
{
        if (tree_get_decl_kind(decl) != TDK_VAR)
                return false;

        tree_type* t = tree_get_decl_type(decl);
        if (tree_get_expr_kind(init) != TEK_INIT)
                init = csema_new_impl_cast(self, init, t);

        tree_set_var_init(decl, init);
        return true;
}

extern bool csema_set_function_body(csema* self, tree_decl* decl, tree_stmt* body)
{
        if (tree_get_decl_kind(decl) != TDK_FUNCTION)
                return false;

        tree_set_function_body(decl, body);
        return true;
}

extern cparam* csema_new_param(csema* self)
{
        return cparam_new(self->ccontext);
}

extern void csema_handle_unused_param(csema* self, cparam* p)
{
        cparam_delete(self->ccontext, p);
}

extern cparam* csema_add_declarator_param(csema* self, cdeclarator* d, cparam* p)
{
        S_ASSERT(p);
        tree_type* t = d->type.tail;
        S_ASSERT(t && tree_get_type_kind(t) == TTK_FUNCTION);

        if (!csema_add_function_type_param(self, t, p))
                return NULL;

        if (!d->params_initialized)
                objgroup_push_back(&d->params, p);
        else
                csema_handle_unused_param(self, p);

        return p;
}

extern cparam* csema_finish_param(csema* self, cparam* p)
{
        if (!csema_check_specifiers(self, &p->specs, &p->declarator, false))
                return NULL;
        if (!csema_set_declarator_type(self, &p->declarator, p->specs.typespec))
                return NULL;

        return p;
}

extern tree_decl* csema_handle_unused_decl_specs(csema* self, cdecl_specs* specs)
{
        cdeclarator d;
        csema_init_declarator(self, &d, CDK_UNKNOWN);
        d.loc = specs->loc;

        // todo: warning
        tree_decl* decl = csema_new_external_decl(self, specs, &d);
        if (!decl)
                return NULL;

        return csema_finish_decl(self, decl);
}

extern bool csema_set_typespec(csema* self, cdecl_specs* specs, tree_type* typespec)
{
        if (specs->typespec)
                return false; // typespec redefinition

        specs->typespec = typespec;
        return true;
}

static void csema_multiple_storage_classes(const csema* self, const cdecl_specs* specs)
{
        cerror(self->error_manager, CES_ERROR, cdecl_specs_get_start_loc(specs),
               "multiple storage classes in declaration specifiers");
}

extern bool csema_set_typedef_specifier(csema* self, cdecl_specs* specs)
{
        if (specs->class_ != TDSC_NONE || specs->is_typedef)
        {
                csema_multiple_storage_classes(self, specs);
                return false;
        }

        specs->is_typedef = true;
        return true;
}

extern bool csema_set_inline_specifier(csema* self, cdecl_specs* specs)
{
        specs->funcspec = TFSK_INLINE;
        return true;
}

extern bool csema_set_decl_storage_class(
        csema* self, cdecl_specs* specs, tree_decl_storage_class class_)
{
        if (specs->class_ != TDSC_NONE || specs->is_typedef)
        {
                csema_multiple_storage_classes(self, specs);
                return false;
        }

        specs->class_ = class_;
        return true;
}