#include "c-prog-decl.h"
#include "c-prog-type.h"
#include "c-prog-conversions.h"

extern void cprog_init_declarator(cprog* self, cdeclarator* declarator, cdeclarator_kind k)
{
        cdeclarator_init(declarator, self->ccontext, k);
}

// returns declarator type
static tree_type* cprog_set_declarator_type(cprog* self, cdeclarator* d, tree_type* t)
{
        if (!d->type.head)
        {
                d->type.head = t;
                d->type.tail = t;
                return t;
        }

        tree_type*     tail = d->type.tail;
        tree_type_kind k    = tree_get_type_kind(tail);
        tree_location  loc  = cdeclarator_get_id_loc_or_begin(d);

        if (k == TTK_POINTER)
        {
                if (!cprog_set_pointer_target(self, tail, t))
                        return NULL;
        }
        else if (k == TTK_FUNCTION)
        {
                if (!cprog_set_function_restype(self, loc, tail, t))
                        return NULL;
        }
        else if (k == TTK_ARRAY)
        {
                if (!cprog_set_array_eltype(self, loc, tail, t))
                        return NULL;
        }
        else if (k == TTK_PAREN)
        {
                if (!cprog_set_paren_type(self, tail, t))
                        return NULL;
        }
        else
                return NULL; // unkown type kind

        d->type.tail = t;
        return d->type.head;
}

extern bool cprog_build_direct_declarator_suffix(
        cprog* self, cdeclarator* declarator, tree_type* suffix)
{
        return (bool)cprog_set_declarator_type(self, declarator, suffix);
}

extern bool cprog_build_direct_declarator_function_suffix(
        cprog* self, cdeclarator* declarator)
{
        tree_type* suffix = cprog_build_function_type(self,
                cdeclarator_get_id_loc_or_begin(declarator), NULL);

        return cprog_build_direct_declarator_suffix(self, declarator, suffix);
}

extern bool cprog_build_direct_declarator_array_suffix(
        cprog* self, cdeclarator* declarator, tree_type_quals quals, tree_exp* size)
{
        tree_type* suffix = cprog_build_array_type(self,
                cdeclarator_get_id_loc_or_begin(declarator), quals, NULL, size);

        return cprog_build_direct_declarator_suffix(self, declarator, suffix);
}

extern bool cprog_add_direct_declarator_parens(cprog* self, cdeclarator* declarator)
{
        return (bool)cprog_set_declarator_type(self, declarator,
                cprog_build_paren_type(self, NULL));
}

extern bool cprog_set_declarator_name(
        cprog* self, tree_location loc, cdeclarator* declarator, tree_id name)
{
        if (declarator->id != tree_get_empty_id())
                return false;

        cdeclarator_set_id(declarator, loc, name);
        return true;
}

extern bool cprog_finish_declarator(
        cprog* self, cdeclarator* declarator, ctype_chain* pointer_chain)
{
        if (!pointer_chain->head)
                return true;

        if (declarator->type.tail)
        {
                bool res = (bool)cprog_set_declarator_type(self, declarator, pointer_chain->head);
                declarator->type.tail = pointer_chain->tail;
                return res;
        }

        declarator->type = *pointer_chain;
        return true;
}

extern tree_decl* cprog_export_decl(cprog* self, tree_decl_scope* scope, tree_decl* decl)
{
        tree_symtab* symtab = tree_get_decl_scope_symtab(scope);
        if (tree_symtab_get(symtab, tree_get_decl_name(decl), false))
        {
                cerror(self->error_manager, CES_ERROR,
                        tree_get_xloc_begin(tree_get_decl_loc(decl)),
                        "redefinition of '%s'",
                        cprog_get_id(self, tree_get_decl_name(decl)));
                return NULL;
        }
        tree_symtab_insert(symtab, decl);
        return decl;
}

static bool cprog_export_decl_scope(cprog* self, tree_decl_scope* to, tree_decl_scope* from)
{
        tree_symtab* tab = tree_get_decl_scope_symtab(from);
        TREE_SYMTAB_FOREACH(tab, it)
                if (!cprog_export_decl(self, to, hiter_get_val(it)))
                        return false;
        return true;
}

static tree_decl* cprog_finish_member_decl(cprog* self, tree_decl_scope* scope, tree_decl* decl)
{
        tree_type* dt = tree_desugar_type(tree_get_decl_type(decl));
        if (!tree_declared_type_is(dt, TDK_RECORD))
                return decl;

        tree_decl* record        = tree_get_decl_type_entity(dt);
        tree_decl_scope* members = tree_get_record_scope(record);

        // if decl is unnamed struct member then add its members to local scope
        if (tree_decl_is_unnamed(decl))
        {
                if (!cprog_export_decl_scope(self, self->locals, members))
                        return NULL;
        }
        return decl;
}

static bool cprog_check_decl_with_no_linkage(const cprog* self, const tree_decl* decl)
{
        tree_location loc = tree_get_xloc_begin(tree_get_decl_loc(decl));
        tree_type*    dt  = tree_get_decl_type(decl);

        if (tree_get_decl_kind(decl) != TDK_FUNCTION)
                return cprog_require_complete_type(self, loc, dt);

        const tree_decl_scope* params = tree_get_function_cparams(decl);
        TREE_DECL_SCOPE_FOREACH(params, p)
        {
                tree_location ploc = tree_get_xloc_begin(tree_get_decl_loc(p));
                tree_type* ptype = tree_get_decl_type(p);
                if (!cprog_require_complete_type(self, ploc, ptype))
                        return false;
        }

        dt = tree_get_function_restype(dt);
        return tree_type_is_void(dt)
                ? true : cprog_require_complete_type(self, loc, dt);
}

static bool cprog_check_decl_with_linkage(cprog* self, tree_decl_scope* scope, tree_decl* decl)
{
        tree_decl_kind dk = tree_get_decl_kind(decl);
        S_ASSERT(dk != TDK_MEMBER);

        tree_location loc  = tree_get_xloc_begin(tree_get_decl_loc(decl));
        tree_id       name = tree_get_decl_name(decl);


        if (dk == TDK_FUNCTION && cprog_at_block_scope(self))
        {
                tree_decl_storage_class sc = tree_get_decl_storage_class(decl);
                if (sc != TDSC_EXTERN && sc != TDSC_IMPL_EXTERN)
                {
                        cerror(self->error_manager, CES_ERROR, loc,
                               "invalid storage class for '%s'",
                               cprog_get_id(self, name));
                        return false;
                }
        }

        tree_decl* orig = tree_decl_scope_find(scope, tree_get_decl_name(decl), false);
        if (!orig)
                return (bool)cprog_export_decl(self, scope, decl);

        if (!tree_decls_have_same_linkage(decl, orig))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                       "redefinition of '%s' with different storage class",
                       cprog_get_id(self, name));
                return false;
        }

        if (!tree_types_are_same(tree_get_decl_type(decl), tree_get_decl_type(orig)))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                       "conflicting types for '%s'", cprog_get_id(self, name));
                return false;
        }
        return true;
}

static tree_decl* cprog_finish_object_or_function_decl(
        cprog* self, tree_decl_scope* scope, tree_decl* decl, bool allow_incomplete)
{
        // c99 6.7
        // If an identifier has no linkage, there shall be no more than one
        //    declaration of the identifier (in a declarator or type specifier)
        //    with the same scope and in the same name space
        // If an identifier for an object is declared with no linkage,
        //    the type for the object shall be complete by the end of its declarator

        tree_decl_storage_class sc = tree_get_decl_storage_class(decl);
        if (sc == TDSC_NONE || sc == TDSC_IMPL_EXTERN)
                if (!allow_incomplete && !cprog_check_decl_with_no_linkage(self, decl))
                        return NULL;

        if (sc == TDSC_NONE)
        {
                if (!cprog_export_decl(self, scope, decl))
                        return NULL;
                if (tree_decl_is(decl, TDK_MEMBER))
                        if (!cprog_finish_member_decl(self, scope, decl))
                                return NULL;
                return decl;
        }

        if (!cprog_check_decl_with_linkage(self, scope, decl))
                return NULL;

        return decl;
}

static tree_decl* _cprog_finish_decl(
        cprog* self, tree_decl_scope* scope, tree_decl* decl, bool allow_incomplete)
{
        tree_decl_kind dk = tree_get_decl_kind(decl);
        if (dk == TDK_VAR || dk == TDK_FUNCTION || dk == TDK_MEMBER)
        {
                if (!cprog_finish_object_or_function_decl(self, scope, decl, allow_incomplete))
                        return NULL;
        }
        else if (dk == TDK_RECORD)
                tree_set_record_complete(decl, true);
        else if (dk == TDK_ENUMERATOR)
                if (!cprog_export_decl(self, self->globals, decl))
                        return NULL;

        tree_decl_scope_insert(scope, decl);
        return decl;
}

extern tree_type* cprog_build_type_name(
        cprog* self, cdeclarator* declarator, tree_type* typespec)
{
        return cprog_set_declarator_type(self, declarator, typespec);
}

extern tree_decl* cprog_finish_decl(cprog* self, tree_decl* decl)
{
        return _cprog_finish_decl(self, self->locals, decl, false);
}

extern tree_decl* cprog_finish_decl_ex(cprog* self, tree_location end_loc, tree_decl* decl)
{
        tree_decl* d = cprog_finish_decl(self, decl);
        if (!d)
                return NULL;

        tree_set_decl_end_loc(d, end_loc);
        return d;
}

extern tree_decl* cprog_build_enumerator(
        cprog* self, tree_decl* enum_, tree_id name, tree_exp* value)
{
        tree_type* t = tree_new_qual_type(self->context, TTQ_UNQUALIFIED,
                tree_new_decl_type(self->context, enum_, true));
        return tree_new_enumerator_decl(self->context, self->locals, 0, name, t, value);
}

extern tree_decl* cprog_build_enum_decl(cprog* self, tree_location kw_loc, tree_id name)
{
        tree_decl* enum_ = cprog_get_local_decl(self, name);
        if (!enum_)
        {
                enum_ = tree_new_enum_decl(self->context,
                        self->locals, tree_init_xloc(kw_loc, kw_loc), name);
                if (!cprog_export_decl(self, self->globals, enum_))
                        return NULL;
        }
        return enum_;
}

extern tree_decl* cprog_build_member_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* struct_declarator, tree_exp* bits)
{
        tree_type* t = cprog_set_declarator_type(self, struct_declarator, decl_specs->typespec);
        if (!t)
                return NULL;

        return tree_new_member_decl(
                self->context,
                self->locals,
                decl_specs->loc,
                struct_declarator->id,
                t,
                bits);
}

extern tree_decl* cprog_build_record_decl(
        cprog* self, tree_location kw_loc, tree_id name, bool is_union)
{
        tree_decl* record = cprog_get_local_decl(self, name);
        if (!record)
        {
                
                record = tree_new_record_decl(self->context,
                        self->locals, tree_init_xloc(kw_loc, kw_loc), name, is_union);

                if (!cprog_export_decl(self, self->globals, record))
                        return NULL;
        }
        return record;
}

static bool cprog_add_function_param(cprog* self, tree_decl* function, cparam* param)
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
        return _cprog_finish_decl(self, self->locals, d,
                sc == TDSC_EXTERN || sc == TDSC_STATIC);
}

static tree_decl* cprog_build_function_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* declarator)
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
      
        cprog_enter_decl_scope(self, tree_get_function_params(func));
        OBJGROUP_FOREACH(&declarator->params, cparam**, it)
                if (!cprog_add_function_param(self, func, *it))
                {
                        func = NULL;
                        break;
                }
        cprog_exit_decl_scope(self);

        return func;
}

static tree_decl* cprog_build_typedef_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        return tree_new_typedef_decl(
                self->context,
                self->locals,
                decl_specs->loc,
                declarator->id,
                declarator->type.head);
}

static tree_decl* cprog_build_var_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        // c99 6.2.2.5
        // If the declaration of an identifier for an object has file scope
        // and no storage - class specifier, its linkage is external.
        tree_decl_storage_class sc = decl_specs->class_;
        if (sc == TDSC_NONE && cprog_at_file_scope(self))
                sc = TDSC_IMPL_EXTERN;

        return tree_new_var_decl(
                self->context,
                self->locals,
                decl_specs->loc,
                declarator->id,
                sc,
                declarator->type.head,
                NULL);
}

extern tree_decl* cprog_build_external_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        if (!cprog_set_declarator_type(self, declarator, decl_specs->typespec))
                return NULL;

        // todo semantics
        if (decl_specs->is_typedef)
        {
                //...       
                return cprog_build_typedef_decl(self, decl_specs, declarator);
        }
        else if (tree_type_is(declarator->type.head, TTK_FUNCTION))
        {
                //...
                return cprog_build_function_decl(self, decl_specs, declarator);
        }

        //...
        return cprog_build_var_decl(self, decl_specs, declarator);
}

extern tree_decl* cprog_add_init_declarator(cprog* self, tree_decl* list, tree_decl* d)
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

extern bool cprog_set_var_initializer(cprog* self, tree_decl* decl, tree_exp* init)
{
        if (tree_get_decl_kind(decl) != TDK_VAR)
                return false;

        tree_type* t = tree_get_decl_type(decl);
        if (tree_get_exp_kind(init) != TEK_INIT)
                init = cprog_build_impl_cast(self, init, t);

        tree_set_var_init(decl, init);
        return true;
}

extern bool cprog_set_function_body(cprog* self, tree_decl* decl, tree_stmt* body)
{
        if (tree_get_decl_kind(decl) != TDK_FUNCTION)
                return false;

        tree_set_function_body(decl, body);
        return true;
}

extern cparam* cprog_build_param(cprog* self)
{
        return cparam_new(self->ccontext);
}

extern void cprog_handle_unused_param(cprog* self, cparam* p)
{
        cparam_delete(self->ccontext, p);
}

extern cparam* cprog_add_declarator_param(cprog* self, cdeclarator* d, cparam* p)
{
        S_ASSERT(p);
        tree_type* t = d->type.tail;
        S_ASSERT(t && tree_get_type_kind(t) == TTK_FUNCTION);

        if (!cprog_add_function_type_param(self, t, p))
                return NULL;

        if (!d->params_initialized)
                objgroup_push_back(&d->params, p);
        else
                cprog_handle_unused_param(self, p);

        return p;
}

extern cparam* cprog_finish_param(cprog* self, cparam* p)
{
        if (!cprog_set_declarator_type(self, &p->declarator, p->specs.typespec))
                return NULL;

        return p;
}

extern tree_decl* cprog_handle_unused_decl_specs(cprog* self, cdecl_specs* specs)
{
        cdeclarator d;
        cprog_init_declarator(self, &d, CDK_UNKNOWN);
        d.loc = specs->loc;

        // todo: warning
        tree_decl* decl = cprog_build_external_decl(self, specs, &d);
        if (!decl)
                return NULL;

        return cprog_finish_decl(self, decl);
}

extern bool cprog_set_typespec(cprog* self, cdecl_specs* specs, tree_type* typespec)
{
        if (specs->typespec)
                return false; // typespec redefinition

        specs->typespec = typespec;
        return true;
}

static void cprog_multiple_storage_classes(const cprog* self, const cdecl_specs* specs)
{
        cerror(self->error_manager, CES_ERROR, cdecl_specs_get_start_loc(specs),
               "multiple storage classes in declaration specifiers");
}

extern bool cprog_set_typedef_specifier(cprog* self, cdecl_specs* specs)
{
        if (specs->class_ != TDSC_NONE || specs->is_typedef)
        {
                cprog_multiple_storage_classes(self, specs);
                return false;
        }

        specs->is_typedef = true;
        return true;
}

extern bool cprog_set_inline_specifier(cprog* self, cdecl_specs* specs)
{
        specs->funcspec = TFSK_INLINE;
        return true;
}

extern bool cprog_set_decl_storage_class(
        cprog* self, cdecl_specs* specs, tree_decl_storage_class class_)
{
        if (specs->class_ != TDSC_NONE || specs->is_typedef)
        {
                cprog_multiple_storage_classes(self, specs);
                return false;
        }

        specs->class_ = class_;
        return true;
}