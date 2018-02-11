#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-expr.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-eval.h"

extern tree_decl* csema_local_lookup(const csema* self, tree_id name, tree_lookup_kind lk)
{
        return tree_decl_scope_lookup(self->locals, lk, name, true);
}

extern tree_decl* csema_global_lookup(const csema* self, tree_id name, tree_lookup_kind lk)
{
        return tree_decl_scope_lookup(self->globals, lk, name, false);
}

extern tree_decl* csema_label_lookup(const csema* self, tree_id name)
{
        return tree_decl_scope_lookup(self->labels, TLK_DECL, name, false);
}

static tree_decl* csema_require_decl(
        const csema* self,
        const tree_decl_scope* scope,
        tree_location name_loc,
        tree_id name,
        bool parent_lookup)
{
        tree_decl* d = tree_decl_scope_lookup(scope, TLK_ANY, name, parent_lookup);
        if (!d)
        {
                cerror_undeclared_identifier(self->logger, name_loc, name);
                return NULL;
        }
        return d;
}

extern tree_decl* csema_require_local_decl(const csema* self, tree_location name_loc, tree_id name)
{
        return csema_require_decl(self, self->locals, name_loc, name, true);
}

extern tree_decl* csema_require_global_decl(const csema* self, tree_location name_loc, tree_id name)
{
        return csema_require_decl(self, self->globals, name_loc, name, false);
}

extern tree_decl* csema_require_label_decl(const csema* self, tree_location name_loc, tree_id name)
{
        return csema_require_decl(self, self->labels, name_loc, name, false);
}

extern tree_decl* csema_require_field_decl(
        const csema* self, const tree_decl* rec, tree_location name_loc, tree_id name)
{
        return csema_require_decl(self, tree_get_record_cfields(rec), name_loc, name, false);
}

extern void ctype_chain_init(ctype_chain* self)
{
        self->head = NULL;
        self->tail = NULL;
}

extern void cdeclarator_init(cdeclarator* self, csema* sema, cdeclarator_kind k)
{
        self->kind = k;
        self->name = TREE_EMPTY_ID;
        self->params_initialized = false;
        self->loc.val = TREE_INVALID_XLOC;
        self->name_loc = TREE_INVALID_LOC;
        self->context = sema->ccontext;

        ctype_chain_init(&self->type);
        dseq_init_ex_ptr(&self->params, cget_alloc(sema->ccontext));
}

extern void cdeclarator_dispose(cdeclarator* self)
{
        for (void** p = dseq_begin_ptr(&self->params);
                p != dseq_end_ptr(&self->params); p++)
        {
                // todo: delete(*p)
        }
}

extern void cdeclarator_set_name(cdeclarator* self, tree_location name_loc, tree_id name)
{
        self->name = name;
        self->name_loc = name_loc;
}

extern void cdeclarator_set_initialized(cdeclarator* self)
{
        self->params_initialized = true;
}

extern void cdeclarator_set_loc_begin(cdeclarator* self, tree_location begin)
{
        self->loc.begin = begin;
}

extern void cdeclarator_set_loc_end(cdeclarator* self, tree_location end)
{
        self->loc.end = end;
}

extern tree_type* cdeclarator_get_type(const cdeclarator* self)
{
        return self->type.head;
}

extern tree_location cdeclarator_get_name_loc_or_begin(const cdeclarator* self)
{
        return self->name_loc == TREE_INVALID_LOC
                ? self->loc.begin
                : self->name_loc;
}

// returns declarator type
static tree_type* csema_add_declarator_type(csema* self, cdeclarator* d, tree_type* t)
{
        S_ASSERT(t);
        if (!d->type.head)
        {
                d->type.head = t;
                d->type.tail = t;
                return t;
        }

        tree_type* tail = d->type.tail;
        tree_type_kind k = tree_get_type_kind(tail);

        if (k == TTK_POINTER)
                tree_set_pointer_target(tail, t);
        else if (k == TTK_FUNCTION)
                tree_set_function_type_result(tail, t);
        else if (k == TTK_ARRAY)
                tree_set_array_eltype(tail, t);
        else if (k == TTK_PAREN)
                tree_set_paren_type(tail, t);
        else
                S_UNREACHABLE();

        d->type.tail = t;
        return d->type.head;
}

extern bool csema_add_direct_declarator_function_suffix(csema* self, cdeclarator* d)
{
        return csema_add_declarator_type(self, d,
                csema_new_function_type(self, NULL)) != NULL;
}

extern bool csema_add_direct_declarator_array_suffix(
        csema* self, cdeclarator* d, tree_type_quals q, tree_expr* size_expr)
{
        return csema_add_declarator_type(self, d, 
                csema_new_array_type(self, q, NULL, size_expr)) != NULL;
}

extern bool csema_add_direct_declarator_parens(csema* self, cdeclarator* d)
{
        return csema_add_declarator_type(self, d, csema_new_paren_type(self, NULL)) != NULL;
}

extern void csema_add_declarator_param(csema* self, cdeclarator* d, cparam* p)
{
        S_ASSERT(p);
        tree_type* t = d->type.tail;
        S_ASSERT(t && tree_get_type_kind(t) == TTK_FUNCTION);

        tree_add_function_type_param(t, cparam_get_type(p));
        if (!d->params_initialized)
                dseq_append_ptr(&d->params, p);
        else
                ;// todo delete(p)
}

extern bool csema_set_declarator_has_vararg(csema* self, cdeclarator* d, tree_location ellipsis_loc)
{
        tree_type* t = d->type.tail;
        S_ASSERT(t && tree_get_type_kind(t) == TTK_FUNCTION);

        if (dseq_size(&d->params) == 0)
        {
                cerror_named_argument_before_ellipsis_required(self->logger, ellipsis_loc);
                return false;
        }

        tree_set_function_type_vararg(t, true);
        return true;
}

extern bool csema_finish_declarator(csema* self, cdeclarator* d, ctype_chain* pointers)
{
        if (!pointers->head)
                return true;

        if (d->type.tail)
        {
                bool res = csema_add_declarator_type(self, d, pointers->head) != NULL;
                d->type.tail = pointers->tail;
                return res;
        }

        d->type = *pointers;
        return true;
}

extern void cdecl_specs_init(cdecl_specs* self)
{
        self->class_ = TDSC_NONE;
        self->typespec = NULL;
        self->funcspec = TFSK_NONE;
        self->is_typedef = false;
        self->loc.val = TREE_INVALID_XLOC;
}

extern void cdecl_specs_set_loc_begin(cdecl_specs* self, tree_location begin)
{
        self->loc.begin = begin;
}

extern void cdecl_specs_set_loc_end(cdecl_specs* self, tree_location end)
{
        self->loc.end = end;
}

extern tree_location cdecl_specs_get_loc_begin(const cdecl_specs* self)
{
        return self->loc.begin;
}

extern tree_location cdecl_specs_get_loc_end(const cdecl_specs* self)
{
        return self->loc.end;
}

extern bool csema_set_typespec(csema* self, cdecl_specs* ds, tree_type* ts)
{
        if (ds->typespec)
                return false; // typespec redefinition

        ds->typespec = ts;
        return true;
}

extern bool csema_set_typedef_spec(csema* self, cdecl_specs* ds)
{
        if (ds->class_ != TDSC_NONE || ds->is_typedef)
        {
                cerror_multiple_storage_classes(self->logger, ds);
                return false;
        }

        ds->is_typedef = true;
        return true;

}

extern bool csema_set_inline_spec(csema* self, cdecl_specs* ds)
{
        ds->funcspec = TFSK_INLINE;
        return true;
}

extern bool csema_set_decl_storage_class(csema* self, cdecl_specs* ds, tree_decl_storage_class sc)
{
        if (ds->class_ != TDSC_NONE || ds->is_typedef)
        {
                cerror_multiple_storage_classes(self->logger, ds);
                return false;
        }

        ds->class_ = sc;
        return true;
}

static bool csema_check_decl_redefinition(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (tree_decl_scope_lookup(scope,
                tree_decl_is_tag(decl) ? TLK_TAG : TLK_DECL,
                tree_get_decl_name(decl), false))
        {
                cerror_decl_redefinition(self->logger, decl);
                return false;
        }
        return true;
}

static tree_decl* csema_add_decl_to_scope(csema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (!csema_check_decl_redefinition(self, scope, decl))
                return NULL;

        tree_decl_scope_add_decl(scope, self->context, decl);
        return decl;
}

static tree_decl* csema_new_external_decl(csema*, cdecl_specs*, cdeclarator*);

extern tree_decl* csema_handle_unused_decl_specs(csema* self, cdecl_specs* ds)
{
        cdeclarator d;
        cdeclarator_init(&d, self, CDK_UNKNOWN);
        d.loc = ds->loc;

        // todo: warning
        tree_decl* decl = csema_new_external_decl(self, ds, &d);
        if (!decl)
                return NULL;

        return csema_add_decl_to_scope(self, self->locals, decl);
}

extern tree_type* cparam_get_type(const cparam* self)
{
        return cdeclarator_get_type(&self->declarator);
}

extern tree_xlocation cparam_get_loc(const cparam* self)
{
        return self->specs.loc;
}

extern cparam* csema_new_param(csema* self)
{
        cparam* p = callocate(self->ccontext, sizeof(cparam));
        cdecl_specs_init(&p->specs);
        cdeclarator_init(&p->declarator, self, CDK_PARAM);
        return p;
}

static tree_type* csema_finish_decl_type(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator)
{
        tree_type* dt = csema_add_declarator_type(self, declarator, decl_specs->typespec);
        if (!dt)
                return NULL;

        return csema_check_type(self, dt,
                cdeclarator_get_name_loc_or_begin(declarator)) ? dt : NULL;
}

static bool csema_check_specifiers(
        const csema* self, const cdecl_specs* specs, const cdeclarator* d, bool function)
{
        if (specs->funcspec == TFSK_INLINE && !function)
        {
                tree_location loc = cdeclarator_get_name_loc_or_begin(d);
                if (loc == TREE_INVALID_LOC)
                        loc = cdecl_specs_get_loc_begin(specs);

                cerror_inline_allowed_on_functions_only(self->logger, loc);
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
                cerror_invalid_parameter_storage_class(self->logger, d);
                return NULL;
        }

        return p;
}

extern tree_type* csema_new_type_name(csema* self, cdeclarator* d, tree_type* typespec)
{
        return csema_add_declarator_type(self, d, typespec);
}

extern tree_decl* csema_add_init_declarator(csema* self, tree_decl* list, tree_decl* decl)
{
        if (!list)
                return decl;

        if (!tree_decl_is(list, TDK_GROUP))
        {
                tree_decl* first = list;
                tree_set_decl_implicit(first, true);
                list = tree_new_decl_group(self->context,
                        self->locals, tree_get_decl_loc(first));

                tree_add_decl_in_group(list, self->context, first);
                tree_decl_scope_add_hidden_decl(self->locals, list);
        }

        tree_add_decl_in_group(list, self->context, decl);
        tree_set_decl_implicit(decl, true);
        return list;
}

static bool csema_compute_enumerator_value(
        csema* self,
        tree_decl* enum_,
        tree_location name_loc,
        tree_id name,
        tree_expr* init,
        int_value* result)
{
        ssize i32_nbits = 8 * tree_get_builtin_type_size(self->target, TBTK_INT32);
        if (init)
        {
                tree_eval_result r;
                if (!tree_eval_expr_as_integer(self->context, init, &r))
                {
                        cerror_enumerator_value_isnt_constant(self->logger, name_loc, name);
                        return false;
                }
                *result = avalue_get_int(&r.value);
                int_resize(result, i32_nbits);
                return true;
        }

        int value = 0;
        tree_decl_scope* s = tree_get_enum_values(enum_);
        if (!tree_decl_scope_is_empty(s))
        {
                tree_decl* last = tree_get_prev_decl(tree_get_decl_scope_decls_end(s));
                value = int_get_i32(tree_get_enumerator_cvalue(last)) + 1;
        }

        int_init(result, i32_nbits, true, value);
        return true;
}

static tree_decl* csema_new_enumerator(
        csema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* init)
{
        int_value value;
        if (!csema_compute_enumerator_value(self, enum_, name_loc, name, init, &value))
                return NULL;

        tree_type* t = tree_new_qual_type(self->context, TTQ_UNQUALIFIED,
                tree_new_decl_type(self->context, enum_, true));

        return tree_new_enumerator_decl(self->context, self->locals,
                tree_create_xloc(name_loc, name_loc), name, t, init, &value);
}

extern tree_decl* csema_define_enumerator_decl(
        csema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* value)
{
        tree_decl* e = csema_new_enumerator(self, enum_, name_loc, name, value);
        if (!e || !csema_add_decl_to_scope(self, self->locals, e))
                return NULL;

        tree_decl* copy = tree_new_enumerator_decl(
                self->context,
                self->globals,
                tree_get_decl_loc(e),
                name,
                tree_get_decl_type(e),
                value,
                tree_get_enumerator_cvalue(e));
        tree_set_decl_implicit(copy, true);

        return csema_add_decl_to_scope(self, self->globals, copy) ? e : NULL;
}

static tree_decl* csema_new_enum_decl(csema* self, tree_location kw_loc, tree_id name)
{
        return tree_new_enum_decl(
                self->context,
                self->locals,
                tree_create_xloc(kw_loc, kw_loc),
                name);
}

static tree_decl* csema_lookup_or_create_enum_decl(
        csema* self, tree_location kw_loc, tree_id name, bool parent_lookup)
{
        tree_decl* e = tree_decl_scope_lookup(self->locals, TLK_TAG, name, parent_lookup);
        if (!e)
        {
                e = csema_new_enum_decl(self, kw_loc, name);
                tree_set_decl_implicit(e, true);
                if (!csema_add_decl_to_scope(self, self->locals, e))
                        return NULL;
        }
        else if (tree_get_decl_kind(e) != TDK_ENUM)
        {
                cerror_wrong_king_of_tag(self->logger, kw_loc, name);
                return NULL;
        }
        return e;
}

extern tree_decl* csema_define_enum_decl(csema* self, tree_location kw_loc, tree_id name)
{
        tree_decl* e = csema_lookup_or_create_enum_decl(self, kw_loc, name, false);
        if (!e)
                return NULL;

        if (tree_tag_decl_is_complete(e))
        {
                cerror_redefinition(self->logger, kw_loc, name);
                return NULL;
        }
        return e;
}

extern tree_decl* csema_declare_enum_decl(csema* self, tree_location kw_loc, tree_id name)
{
        return csema_lookup_or_create_enum_decl(self, kw_loc, name, true);
}

extern tree_decl* csema_complete_enum_decl(csema* self, tree_decl* enum_, tree_location end)
{
        tree_set_tag_decl_complete(enum_, true);
        tree_set_decl_loc_end(enum_, end);
        return enum_;
}

static tree_decl* csema_new_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        return tree_new_record_decl(
                self->context,
                self->locals,
                tree_create_xloc(kw_loc, kw_loc),
                name,
                is_union);
}

static tree_decl* csema_lookup_or_create_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union, bool parent_lookup)
{
        tree_decl* d = tree_decl_scope_lookup(self->locals, TLK_TAG, name, parent_lookup);
        if (!d)
        {
                d = csema_new_record_decl(self, kw_loc, name, is_union);
                tree_set_decl_implicit(d, true);
                if (!csema_add_decl_to_scope(self, self->locals, d))
                        return NULL;
        }
        else if (!tree_decl_is(d, TDK_RECORD) || tree_record_is_union(d) != is_union)
        {
                cerror_wrong_king_of_tag(self->logger, kw_loc, name);
                return NULL;
        }
        return d;
}

extern tree_decl* csema_define_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        tree_decl* r = csema_lookup_or_create_record_decl(self, kw_loc, name, is_union, false);
        if (!r)
                return NULL;

        if (tree_tag_decl_is_complete(r))
        {
                cerror_redefinition(self->logger, kw_loc, name);
                return NULL;
        }
        return r;
}

extern tree_decl* csema_declare_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        return csema_lookup_or_create_record_decl(self, kw_loc, name, is_union, true);
}

extern tree_decl* csema_complete_record_decl(csema* self, tree_decl* rec, tree_location end)
{
        tree_set_tag_decl_complete(rec, true);
        tree_set_decl_loc_end(rec, end);
        return rec;
}


static bool csema_check_field_decl(const csema* self, const tree_decl* field)
{
        tree_type* mt = tree_desugar_type(tree_get_decl_type(field));
        if (tree_type_is(mt, TTK_FUNCTION))
        {
                cerror_field_function(self->logger, field);
                return false;
        }
        if (!csema_require_complete_type(self, tree_get_decl_loc_begin(field), mt))
                return false;

        tree_expr* bit_width = tree_get_field_bit_width(field);
        if (!bit_width)
                return true;

        if (!tree_type_is_integer(mt))
        {
                cerror_invalid_bitfield_type(self->logger, field);
                return false;
        }

        tree_eval_result r;
        if (!tree_eval_expr_as_integer(self->context, bit_width, &r))
        {
                cerror_bitfield_width_isnt_constant(self->logger, field);
                return false;
        }

        if (avalue_is_zero(&r.value))
        {
                cerror_bitfield_width_is_zero(self->logger, field);
                return false;
        }

        if (avalue_is_signed(&r.value) && avalue_get_i64(&r.value) < 0)
        {
                cerror_negative_bitfield_width(self->logger, field);
                return false;
        }

        if (avalue_get_u64(&r.value) > 8 * tree_get_sizeof(self->target, mt))
        {
                cerror_bitfield_width_exceeds_type(self->logger, field);
                return false;
        }
        return field;
}

static tree_decl* csema_new_field_decl(
        csema* self, cdecl_specs* ds, cdeclarator* d, tree_expr* bit_width)
{
        if (!csema_check_specifiers(self, ds, d, false))
                return NULL;

        tree_type* t = csema_finish_decl_type(self, ds, d);
        if (!t)
                return NULL;

        tree_decl* field = tree_new_field_decl(
                self->context, self->locals, ds->loc, d->name, t, bit_width);

        if (!csema_check_field_decl(self, field))
                return NULL;

        return field;
}

static bool csema_inject_anonymous_record_members(
        csema* self, tree_decl_scope* to, tree_decl* anon)
{
        tree_decl* rec = tree_get_decl_type_entity(tree_get_decl_type(anon));
        tree_decl_scope* fields = tree_get_record_fields(rec);

        TREE_FOREACH_DECL_IN_SCOPE(fields, it)
        {
                tree_decl_kind dk = tree_get_decl_kind(it);
                if (dk != TDK_FIELD && dk != TDK_INDIRECT_FIELD)
                        continue;

                tree_decl* indirect_field = tree_new_indirect_field_decl(
                        self->context, to,
                        tree_get_decl_loc(it),
                        tree_get_decl_name(it),
                        tree_get_decl_type(it),
                        anon);

                if (!csema_add_decl_to_scope(self, to, indirect_field))
                        return false;
        }
        return true;
}

extern tree_decl* csema_define_field_decl(
        csema* self, cdecl_specs* ds, cdeclarator* d, tree_expr* bit_width)
{
        tree_decl* field = csema_new_field_decl(self, ds, d, bit_width);
        if (!field || !csema_add_decl_to_scope(self, self->locals, field))
                return NULL;

        tree_type* type = tree_get_decl_type(field);
        if (!tree_decl_is_anon(field) || !tree_declared_type_is(type, TDK_RECORD))
                return field;

        if (!csema_inject_anonymous_record_members(self, self->locals, field))
                return NULL;

        return field;
}

static tree_decl* csema_new_label_decl(
        csema* self, tree_location id_loc, tree_id id, tree_location colon_loc)
{
        return tree_new_label_decl(self->context,
                self->labels, tree_create_xloc(id_loc, colon_loc), id, NULL);
}

extern tree_decl* csema_define_label_decl(
        csema* self, tree_location name_loc, tree_id name, tree_location colon_loc, tree_stmt* stmt)
{
        tree_decl* l = csema_declare_label_decl(self, name_loc, name);
        if (!l)
                return NULL;

        if (tree_get_label_decl_stmt(l))
        {
                cerror_redefinition(self->logger, name_loc, name);
                return NULL;
        }

        tree_set_decl_loc(l, tree_create_xloc(name_loc, colon_loc));
        tree_set_label_decl_stmt(l, stmt);
        return l;
}

extern tree_decl* csema_declare_label_decl(csema* self, tree_location name_loc, tree_id name)
{
        tree_decl* l = csema_label_lookup(self, name);
        if (!l)
        {
                l = csema_new_label_decl(self, name_loc, name, name_loc);
                if (!csema_add_decl_to_scope(self, self->labels, l))
                        return NULL;
        }
        return l;
}

extern bool csema_require_variable_decl_kind(const csema* self, const tree_decl* decl)
{
        tree_decl_kind k = tree_get_decl_kind(decl);
        if (k == TDK_VAR)
                return true;

        if (k == TDK_FUNCTION)
                cerror_function_initialized_like_a_variable(self->logger, decl);
        else
                S_UNREACHABLE(); // todo
        
        return false;
}

extern bool csema_require_function_decl_kind(const csema* self, const tree_decl* decl)
{
        if (tree_decl_is(decl, TDK_FUNCTION))
                return true;
        return false;
}

static tree_decl* csema_new_typedef_decl(csema* self, cdecl_specs* ds, cdeclarator* d)
{
        if (!csema_check_specifiers(self, ds, d, false))
                return NULL;

        return tree_new_typedef_decl(
                self->context, self->locals, ds->loc, d->name, d->type.head);
}

static tree_decl* csema_new_param_decl(csema* self, cparam* p)
{
        return tree_new_var_decl(
                self->context,
                self->locals,
                cparam_get_loc(p),
                p->declarator.name,
                TDSC_NONE,
                p->declarator.type.head,
                NULL);
}

static tree_decl* csema_define_param_decl(csema* self, cparam* p)
{
        tree_decl* d = csema_new_param_decl(self, p);
        if (!d)
                return NULL;

        return csema_add_decl_to_scope(self, self->locals, d);
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
                d->name,
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
                self->context, self->locals, specs->loc, d->name, sc, d->type.head, NULL);
        if (!csema_check_var_decl(self, v))
                return NULL;

        return v;
}

static tree_decl* csema_new_external_decl(csema* self, cdecl_specs* ds, cdeclarator* d)
{
        if (!csema_finish_decl_type(self, ds, d))
                return NULL;

        if (ds->is_typedef)
                return csema_new_typedef_decl(self, ds, d);
        else if (tree_type_is(d->type.head, TTK_FUNCTION))
                return csema_new_function_decl(self, ds, d);

        return csema_new_var_decl(self, ds, d);
}

extern tree_decl* csema_declare_external_decl(csema* self, cdecl_specs* ds, cdeclarator* d)
{
        tree_decl* decl = csema_new_external_decl(self, ds, d);
        if (!decl)
                return NULL;

        tree_decl_kind dk = tree_get_decl_kind(decl);
        tree_decl_storage_class sc = dk == TDK_TYPEDEF
                ? TDSC_NONE : tree_get_decl_storage_class(decl);

        if (dk != TDK_TYPEDEF && sc == TDSC_NONE)
                return csema_add_decl_to_scope(self, self->locals, decl);
        else if (dk == TDK_FUNCTION && csema_at_block_scope(self))
                if (sc != TDSC_EXTERN && sc != TDSC_IMPL_EXTERN)
                {
                        cerror_invalid_storage_class(self->logger, decl);
                        return false;
                }

        tree_decl* orig = csema_local_lookup(self, tree_get_decl_name(decl), false);
        if (!orig)
                return csema_add_decl_to_scope(self, self->locals, decl);

        if (dk != tree_get_decl_kind(orig))
        {
                cerror_different_kind_of_symbol(self->logger, decl);
                return false;
        }

        if (dk != TDK_TYPEDEF && !tree_decls_have_same_linkage(decl, orig))
        {
                cerror_different_storage_class(self->logger, decl);
                return false;
        }

        if (!csema_types_are_same(self, tree_get_decl_type(decl), tree_get_decl_type(orig)))
        {
                cerror_conflicting_types(self->logger, decl);
                return false;
        }

        tree_decl_scope_add_hidden_decl(self->locals, decl);
        return decl;
}

extern tree_decl* csema_define_var_decl(csema* self, tree_decl* var, tree_expr* init)
{
        S_ASSERT(init && tree_get_decl_kind(var) == TDK_VAR);

        if (tree_get_expr_kind(init) != TEK_INIT_LIST)
                init = csema_new_impl_cast(self, init, tree_get_decl_type(var));

        tree_decl* orig = csema_local_lookup(self, tree_get_decl_name(var), false);
        if (orig && tree_get_var_init(orig))
        {
                cerror_decl_redefinition(self->logger, var);
                return false;
        }

        tree_set_var_init(var, init);
        tree_decl_scope_update_lookup(self->locals, self->context, var);
        return var;
}

extern tree_decl* csema_define_func_decl(csema* self, tree_decl* func, tree_stmt* body)
{
        S_ASSERT(body && tree_get_decl_kind(func) == TDK_FUNCTION);

        tree_decl* orig = csema_global_lookup(self, tree_get_decl_name(func), false);
        if (orig && tree_get_function_body(orig))
        {
                cerror_decl_redefinition(self->logger, func);
                return NULL;
        }

        tree_location loc = tree_get_decl_loc_begin(func);
        tree_type* restype = tree_get_function_type_result(tree_get_decl_type(func));
        if (!tree_type_is_void(restype) && !csema_require_complete_type(self, loc, restype))
                return false;

        const tree_decl_scope* params = tree_get_function_cparams(func);
        TREE_FOREACH_DECL_IN_SCOPE(params, p)
        {
                if (tree_decl_is_anon(p))
                {
                        cerror_parameter_name_omitted(self->logger, p);
                        return NULL;
                }

                tree_type* ptype = tree_get_decl_type(p);
                tree_location ploc = tree_get_decl_loc_begin(p);
                if (!csema_require_complete_type(self, ploc, ptype))
                        return NULL;
        }

        tree_set_function_body(func, body);
        tree_decl_scope_update_lookup(self->globals, self->context, func);
        return func;
}

extern bool csema_check_func_def_loc(csema* self, const tree_decl* func)
{
        if (self->function)
        {
                cerror_function_isnt_allowed_here(self->logger, func);
                return false;
        }
        return true;
}