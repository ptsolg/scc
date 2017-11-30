#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-expr.h"
#include "scc/tree/tree-eval.h"

extern tree_decl* csema_lookup_for_duplicates(
        const csema* self, const tree_decl_scope* scope, const tree_decl* decl)
{
        return csema_get_decl(self, scope, tree_get_decl_name(decl),
                tree_decl_is(decl, TDK_ENUM) || tree_decl_is(decl, TDK_RECORD), false);
}

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

        if (k == TTK_POINTER)
                csema_set_pointer_target(self, tail, t);
        else if (k == TTK_FUNCTION)
                csema_set_function_restype(self, tail, t);
        else if (k == TTK_ARRAY)
                csema_set_array_eltype(self, tail, t);
        else if (k == TTK_PAREN)
                csema_set_paren_type(self, tail, t);
        else
                S_UNREACHABLE();

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
        tree_type* suffix = csema_new_function_type(self, NULL);
        return csema_new_direct_declarator_suffix(self, declarator, suffix);
}

extern bool csema_new_direct_declarator_array_suffix(
        csema* self, cdeclarator* declarator, tree_type_quals quals, tree_expr* size)
{
        tree_type* suffix = csema_new_array_type(self, quals, NULL, size);

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

static tree_decl* csema_finish_decl(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (!csema_export_decl(self, scope, decl))
                return NULL;

        tree_decl_scope_add(scope, cget_decl_key(self->ccontext, decl), decl);
        return decl;
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

        return csema_finish_decl(self, self->locals, decl);
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
                dseq_append_ptr(&d->params, p);
        else
                csema_handle_unused_param(self, p);

        return p;
}

static tree_type* csema_finish_decl_type(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        tree_type* dt = csema_set_declarator_type(self, declarator, decl_specs->typespec);
        if (!dt)
                return NULL;

        return csema_check_type(self, dt,
                cdeclarator_get_id_loc_or_begin(declarator)) ? dt : NULL;
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

extern cparam* csema_finish_param(csema* self, cparam* p)
{
        cdeclarator* d = &p->declarator;
        if (!csema_check_specifiers(self, &p->specs, d, false))
                return NULL;
        if (!csema_finish_decl_type(self, &p->specs, d))
                return NULL;

        if (p->specs.class_ != TDSC_REGISTER && p->specs.class_ != TDSC_NONE)
        {
                cerror(self->error_manager, CES_ERROR, d->id_loc,
                        "invalid storage class for parameter '%s'", csema_get_id_cstr(self, d->id));
                return NULL;
        }

        return p;
}

extern bool csema_set_declarator_has_vararg(
        csema* self, cdeclarator* declarator, tree_location ellipsis_loc)
{
        tree_type* t = declarator->type.tail;
        S_ASSERT(t && tree_get_type_kind(t) == TTK_FUNCTION);

        if (dseq_size(&declarator->params) == 0)
        {
                cerror(self->error_manager, CES_ERROR, ellipsis_loc,
                        "ISO C requires a named argument before '...'");
        }
        
        tree_set_function_type_vararg(t, true);
        return true;
}

static void csema_redefinition(const csema* self, tree_location loc, tree_id id)
{
        cerror(self->error_manager, CES_ERROR, loc, 
                "redefinition of '%s'", csema_get_id_cstr(self, id));
}

static void csema_decl_redifinition(const csema* self, const tree_decl* decl)
{
        csema_redefinition(self, tree_get_decl_loc_begin(decl), tree_get_decl_name(decl));
}

extern tree_decl* csema_export_decl(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (tree_id_is_empty(tree_get_decl_name(decl)))
                return decl;
        
        hval key = cget_decl_key(self->ccontext, decl);
        if (tree_decl_scope_lookup(scope, key, false))
        {
                csema_decl_redifinition(self, decl);
                return NULL;
        }
        tree_decl_scope_add_lookup(scope, key, decl);
        return decl;
}

extern tree_decl* csema_set_decl_end_loc(const csema* self, tree_decl* decl, tree_location end)
{
        tree_set_decl_loc_end(decl, end);
        return decl;
}

static bool csema_export_decl_scope(csema* self, tree_decl_scope* to, tree_decl_scope* from)
{
        TREE_FOREACH_DECL_IN_LOOKUP(from, it)
                if (!csema_export_decl(self, to, hiter_get_ptr(&it)))
                        return false;
        return true;
}

extern tree_type* csema_new_type_name(
        csema* self, cdeclarator* declarator, tree_type* typespec)
{
        return csema_set_declarator_type(self, declarator, typespec);
}

static void csema_compute_enumerator_value(
        csema* self, tree_decl* enum_, tree_decl* enumerator)
{
        int v = 0;
        tree_decl_scope* s = tree_get_enum_scope(enum_);

        if (!tree_decl_scope_is_empty(s))
        {
                tree_decl* last = tree_get_prev_decl(tree_get_decl_scope_decls_end(s));
                int_value last_val;
                tree_eval_info i;
                bool r = tree_eval_as_integer(&i, tree_get_enumerator_value(last), &last_val);
                S_ASSERT(r);
                v = int_get_i32(&last_val) + 1;
        }

        tree_expr* val = csema_new_integer_literal(self,
                tree_get_decl_loc_begin(enumerator), v, true, false);
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

        const char* name = csema_get_id_cstr(self, tree_get_decl_name(e));

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

extern tree_decl* csema_define_enumerator(
        csema* self, tree_decl* enum_, tree_id id, tree_id id_loc, tree_expr* value)
{
        tree_decl* e = csema_new_enumerator(self, enum_, id, id_loc, value);
        if (!e)
                return NULL;

        if (!csema_finish_decl(self, self->locals, e))
                return NULL;

        return csema_export_decl(self, self->globals, e);
}

static void csema_wrong_kind_of_tag(const csema* self, tree_location loc, tree_id name)
{
        cerror(self->error_manager, CES_ERROR, loc,
                "'%s' defined as wrong kind of tag", csema_get_id_cstr(self, name));
}

extern tree_decl* csema_new_enum_decl(csema* self, tree_location kw_loc, tree_id name)
{
        return tree_new_enum_decl(
                self->context,
                self->locals,
                tree_init_xloc(kw_loc, kw_loc),
                name);
}

static tree_decl* _csema_declare_enum_decl(
        csema* self, tree_location kw_loc, tree_id name, bool parent_lookup)
{
        tree_decl* e = csema_get_decl(self, self->locals, name, true, parent_lookup);
        if (!e)
        {
                e = csema_new_enum_decl(self, kw_loc, name);
                if (!csema_export_decl(self, self->locals, e))
                        return NULL;
        }
        else if (tree_get_decl_kind(e) != TDK_ENUM)
        {
                csema_wrong_kind_of_tag(self, kw_loc, name);
                return NULL;
        }
        return e;
}

extern tree_decl* csema_declare_enum_decl(csema* self, tree_location kw_loc, tree_id name)
{
        return _csema_declare_enum_decl(self, kw_loc, name, true);
}

extern tree_decl* csema_define_enum_decl(csema* self, tree_location kw_loc, tree_id name)
{
        tree_decl* e = _csema_declare_enum_decl(self, kw_loc, name, false);
        if (!e)
                return NULL;

        if (!tree_decl_scope_is_empty(tree_get_enum_scope(e)))
        {
                csema_redefinition(self, kw_loc, name);
                return NULL;
        }
        return e;
}

extern tree_decl* csema_new_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        return tree_new_record_decl(
                self->context,
                self->locals,
                tree_init_xloc(kw_loc, kw_loc),
                name,
                is_union);
}

extern tree_decl* _csema_declare_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union, bool parent_lookup)
{
        tree_decl* d = csema_get_decl(self, self->locals, name, true, parent_lookup);
        if (!d)
        {
                d = csema_new_record_decl(self, kw_loc, name, is_union);
                if (!csema_export_decl(self, self->locals, d))
                        return NULL;
        }
        else if (!tree_decl_is(d, TDK_RECORD) || tree_record_is_union(d) != is_union)
        {
                csema_wrong_kind_of_tag(self, kw_loc, name);
                return NULL;
        }

        return d;
}

extern tree_decl* csema_declare_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        return _csema_declare_record_decl(self, kw_loc, name, is_union, true);
}

extern tree_decl* csema_define_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        tree_decl* r = _csema_declare_record_decl(self, kw_loc, name, is_union, false);
        if (!r)
                return NULL;

        if (tree_record_is_complete(r))
        {
                csema_redefinition(self, kw_loc, name);
                return NULL;
        }
        return r;
}

extern tree_decl* csema_complete_record_decl(csema* self, tree_decl* decl, tree_location end_loc)
{
        tree_set_record_complete(decl, true);
        tree_set_decl_loc_end(decl, end_loc);
        return decl;
}

static bool csema_check_member_decl(const csema* self, const tree_decl* m)
{
        tree_type* mt = tree_desugar_type(tree_get_decl_type(m));
        tree_location loc = tree_get_decl_loc_begin(m);
        const char* name = csema_get_id_cstr(self, tree_get_decl_name(m));

        if (tree_type_is(mt, TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "field '%s' declared as function", name);
                return false;
        }
        if (!csema_require_complete_type(self, loc, mt))
                return false;

        tree_expr* bits = tree_get_member_bits(m);
        if (!bits)
                return true;

        if (!tree_type_is_integer(mt))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "bit-field '%s' has invalid type", name);
                return false;
        }

        int_value val;
        tree_eval_info i;
        tree_init_eval_info(&i, self->target);
        if (!tree_eval_as_integer(&i, bits, &val))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "bit-field '%s' width not an integer constant", name);
                return false;
        }

        if (int_is_zero(&val))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "zero width for bit-field '%s'", name);
                return false;
        }

        if (int_is_signed(&val) && int_get_i64(&val) < 0)
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "negative width in bit-field '%s'", name);
                return false;
        }

        if (int_get_u64(&val) > 8 * tree_get_sizeof(self->target, mt))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "width of '%s' exceeds its type", name);
                return false;
        }
        return m;
}

extern tree_decl* csema_new_member_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* struct_declarator, tree_expr* bits)
{
        if (!csema_check_specifiers(self, decl_specs, struct_declarator, false))
                return NULL;

        tree_type* t = csema_finish_decl_type(self, decl_specs, struct_declarator);
        if (!t)
                return NULL;

        tree_decl* m = tree_new_member_decl(
                self->context, self->locals, decl_specs->loc, struct_declarator->id, t, bits);

        if (!csema_check_member_decl(self, m))
                return NULL;

        return m;
}

extern tree_decl* csema_define_member_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* struct_declarator, tree_expr* bits)
{
        tree_decl* m = csema_new_member_decl(self, decl_specs, struct_declarator, bits);
        if (!m || !csema_finish_decl(self, self->locals, m))
                return NULL;

        tree_type* mt = tree_get_decl_type(m);
        // if decl is unnamed struct member then if it is record, add its members to local scope
        if (!tree_decl_is_unnamed(m) || !tree_declared_type_is(mt, TDK_RECORD))
                return m;

        tree_decl* record = tree_get_decl_type_entity(mt);
        tree_decl_scope* members = tree_get_record_scope(record);

        if (!csema_export_decl_scope(self, self->locals, members))
                return NULL;

        return m;
}

extern tree_decl* csema_new_label_decl(
        csema* self, tree_location id_loc, tree_id id, tree_location colon_loc)
{
        return tree_new_label_decl(self->context,
                self->labels, tree_init_xloc(id_loc, colon_loc), id, NULL);
}

extern tree_decl* csema_declare_label_decl(csema* self, tree_id name, tree_id name_loc)
{
        tree_decl* l = csema_get_label_decl(self, name);
        if (!l)
        {
                l = csema_new_label_decl(self, name_loc, name, name_loc);
                if (!csema_export_decl(self, self->labels, l))
                        return NULL;
        }
        return l;
}

extern tree_decl* csema_define_label_decl(
        csema* self, tree_location id_loc, tree_id id, tree_location colon_loc, tree_stmt* stmt)
{
        tree_decl* l = csema_declare_label_decl(self, id, id_loc);
        if (!l)
                return NULL;

        if (tree_get_label_decl_stmt(l))
        {
                csema_redefinition(self, id_loc, id);
                return NULL;
        }

        tree_set_decl_loc(l, tree_init_xloc(id_loc, colon_loc));
        tree_set_label_decl_stmt(l, stmt);
        return l;
}

static tree_decl* csema_new_typedef_decl(
        csema* self, cdecl_specs* specs, cdeclarator* d)
{
        if (!csema_check_specifiers(self, specs, d, false))
                return NULL;

        return tree_new_typedef_decl(
                self->context, self->locals, specs->loc, d->id, d->type.head);
}

static tree_decl* csema_new_param_decl(csema* self, cparam* p)
{
        return tree_new_var_decl(
                self->context,
                self->locals,
                cparam_get_loc(p),
                p->declarator.id,
                TDSC_NONE,
                p->declarator.type.head,
                NULL);
}

static tree_decl* csema_define_param_decl(csema* self, cparam* p)
{
        tree_decl* d = csema_new_param_decl(self, p);
        if (!d)
                return NULL;

        return csema_finish_decl(self, self->locals, d);
}

static bool csema_set_function_params(csema* self, tree_decl* func, dseq* params)
{
        csema_enter_decl_scope(self, tree_get_function_params(func));

        for (ssize i = 0; i < dseq_size(params); i++)
                if (!csema_define_param_decl(self, dseq_get_ptr(params, i)))
                {
                        csema_exit_decl_scope(self);
                        return false;
                }

        csema_exit_decl_scope(self);
        return true;
}

static tree_decl* csema_new_function_decl(
        csema* self, cdecl_specs* specs, cdeclarator* d)
{
        // c99 6.2.2.5
        // If the declaration of an identifier for a function has no storage - class specifier,
        // its linkage is external
        tree_decl_storage_class sc = specs->class_;
        if (sc == TDSC_NONE)
                sc = TDSC_IMPL_EXTERN;

        tree_decl* func = tree_new_function_decl(
                self->context,
                self->locals,
                specs->loc,
                d->id,
                sc,
                d->type.head,
                specs->funcspec,
                NULL);

        if (!csema_set_function_params(self, func, &d->params))
                return NULL;

        return func;
}

static bool csema_check_var_decl(const csema* self, const tree_decl* var)
{
        tree_decl_storage_class sc = tree_get_decl_storage_class(var);
        if (sc == TDSC_NONE || sc == TDSC_IMPL_EXTERN)
                return csema_require_complete_type(self,
                        tree_get_decl_loc_begin(var), tree_get_decl_type(var));
        return true;
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

        tree_decl* v = tree_new_var_decl(
                self->context, self->locals, specs->loc, d->id, sc, d->type.head, NULL);
        if (!csema_check_var_decl(self, v))
                return NULL;

        return v;
}

extern tree_decl* csema_new_external_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        if (!csema_finish_decl_type(self, decl_specs, declarator))
                return NULL;

        if (decl_specs->is_typedef)
                return csema_new_typedef_decl(self, decl_specs, declarator);
        else if (tree_type_is(declarator->type.head, TTK_FUNCTION))
                return csema_new_function_decl(self, decl_specs, declarator);

        return csema_new_var_decl(self, decl_specs, declarator);
}

extern tree_decl* csema_declare_external_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        tree_decl* d = csema_new_external_decl(self, decl_specs, declarator);
        if (!d)
                return NULL;
        
        tree_location loc = tree_get_decl_loc_begin(d);
        const char* name = csema_get_id_cstr(self, tree_get_decl_name(d));
        tree_decl_kind dk = tree_get_decl_kind(d);
        tree_decl_storage_class sc = dk == TDK_TYPEDEF
                ? TDSC_NONE : tree_get_decl_storage_class(d);

        if (dk != TDK_TYPEDEF && sc == TDSC_NONE)
                return csema_finish_decl(self, self->locals, d);
        else if (dk == TDK_FUNCTION && csema_at_block_scope(self))
        {
                if (sc != TDSC_EXTERN && sc != TDSC_IMPL_EXTERN)
                {
                        cerror(self->error_manager, CES_ERROR, loc,
                                "invalid storage class for '%s'", name);
                        return NULL;
                }
        }

        tree_decl* orig = csema_lookup_for_duplicates(self, self->locals, d);
        if (!orig)
                return csema_finish_decl(self, self->locals, d);

        if (tree_get_decl_kind(d) != tree_get_decl_kind(orig))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "'%s' redeclared as different kind of symbol", name);
                return NULL;
        }

        if (dk != TDK_TYPEDEF && !tree_decls_have_same_linkage(d, orig))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "redefinition of '%s' with different storage class", name);
                return NULL;
        }

        if (!tree_types_are_same(tree_get_decl_type(d), tree_get_decl_type(orig)))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "conflicting types for '%s'", name);
                return NULL;
        }

        tree_decl_scope_add(self->locals, cget_decl_key(self->ccontext, d), d);
        return d;
}

extern tree_decl* csema_define_var_decl(csema* self, tree_decl* var, tree_expr* init)
{
        if (tree_get_decl_kind(var) != TDK_VAR)
                return false;

        tree_type* t = tree_get_decl_type(var);
        if (tree_get_expr_kind(init) != TEK_INIT)
                init = csema_new_impl_cast(self, init, t);

        tree_decl* orig = csema_get_local_decl(self, tree_get_decl_name(var), false);
        if (tree_get_var_init(orig))
        {
                csema_decl_redifinition(self, var);
                return false;
        }

        tree_set_var_init(var, init);
        return var;
}

extern tree_decl* csema_define_function_decl(csema* self, tree_decl* func, tree_stmt* body)
{
        S_ASSERT(tree_decl_is(func, TDK_FUNCTION));
        tree_decl* orig = csema_get_global_decl(self, tree_get_decl_name(func), false);
        if (!orig)
                orig = func;

        if (tree_get_function_body(orig))
        {
                csema_decl_redifinition(self, func);
                return NULL;
        }

        if (!body)
                return func;

        tree_location loc = tree_get_decl_loc_begin(func);
        tree_type* restype = tree_get_function_type_result(tree_get_decl_type(func));
        if (!tree_type_is_void(restype) && !csema_require_complete_type(self, loc, restype))
                return false;

        const tree_decl_scope* params = tree_get_function_cparams(func);
        TREE_FOREACH_DECL_IN_SCOPE(params, p)
        {
                tree_location ploc = tree_get_decl_loc_begin(p);
                if (tree_get_decl_name(p) == tree_get_empty_id())
                {
                        cerror(self->error_manager, CES_ERROR, ploc, "parameter name omitted");
                        return NULL;
                }

                tree_type* ptype = tree_get_decl_type(p);
                if (!csema_require_complete_type(self, ploc, ptype))
                        return NULL;
        }

        tree_set_function_body(func, body);
        return func;
}

extern bool csema_check_function_def_loc(csema* self, tree_decl* func)
{
        if (self->function)
        {
                cerror(self->error_manager, CES_ERROR, tree_get_decl_loc_begin(func),
                        "function definition is not allowed here");
                return false;
        }
        return true;
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
                tree_decl_scope_add_hidden(self->locals, list);
        }

        tree_decl_group_add(list, d);
        tree_set_decl_implicit(d, true);
        return list;
}
