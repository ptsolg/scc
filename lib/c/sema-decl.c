#include "scc/c/sema.h"
#include "errors.h"
#include "scc/tree/context.h"
#include "scc/tree/eval.h"
#include "scc/tree/target.h"

extern tree_decl* c_sema_local_lookup(const c_sema* self, tree_id name, tree_lookup_kind lk)
{
        return tree_decl_scope_lookup(self->locals, lk, name, true);
}

extern tree_decl* c_sema_global_lookup(const c_sema* self, tree_id name, tree_lookup_kind lk)
{
        return tree_decl_scope_lookup(self->globals, lk, name, false);
}

extern tree_decl* c_sema_label_lookup(const c_sema* self, tree_id name)
{
        return tree_decl_scope_lookup(self->labels, TLK_DECL, name, false);
}

static tree_decl* c_sema_require_decl(
        const c_sema* self,
        const tree_decl_scope* scope,
        tree_location name_loc,
        tree_id name,
        bool parent_lookup)
{
        tree_decl* d = tree_decl_scope_lookup(scope, TLK_ANY, name, parent_lookup);
        if (!d)
        {
                c_error_undeclared_identifier(self->ccontext, name_loc, name);
                return NULL;
        }
        return d;
}

extern tree_decl* c_sema_require_local_decl(const c_sema* self, tree_location name_loc, tree_id name)
{
        return c_sema_require_decl(self, self->locals, name_loc, name, true);
}

extern tree_decl* c_sema_require_global_decl(const c_sema* self, tree_location name_loc, tree_id name)
{
        return c_sema_require_decl(self, self->globals, name_loc, name, false);
}

extern tree_decl* c_sema_require_label_decl(const c_sema* self, tree_location name_loc, tree_id name)
{
        return c_sema_require_decl(self, self->labels, name_loc, name, false);
}

extern tree_decl* c_sema_require_field_decl(
        const c_sema* self, const tree_decl* rec, tree_location name_loc, tree_id name)
{
        return c_sema_require_decl(self, tree_get_record_cfields(rec), name_loc, name, false);
}

extern void c_type_chain_init(c_type_chain* self)
{
        self->head = NULL;
        self->tail = NULL;
}

extern void c_declarator_init(c_declarator* self, c_sema* sema, c_declarator_kind k)
{
        self->kind = k;
        self->name = TREE_EMPTY_ID;
        self->params_initialized = false;
        self->loc.val = TREE_INVALID_XLOC;
        self->name_loc = TREE_INVALID_LOC;
        self->context = sema->ccontext;

        c_type_chain_init(&self->type);
        ptrvec_init_ex(&self->params, c_context_get_allocator(sema->ccontext));
}

extern void c_declarator_dispose(c_declarator* self)
{
        for (void** p = ptrvec_begin(&self->params);
                p != ptrvec_end(&self->params); p++)
        {
                // todo: delete(*p)
        }
}

extern void c_declarator_set_name(c_declarator* self, tree_location name_loc, tree_id name)
{
        self->name = name;
        self->name_loc = name_loc;
}

extern void c_declarator_set_initialized(c_declarator* self)
{
        self->params_initialized = true;
}

extern void c_declarator_set_loc_begin(c_declarator* self, tree_location begin)
{
        self->loc.begin = begin;
}

extern void c_declarator_set_loc_end(c_declarator* self, tree_location end)
{
        self->loc.end = end;
}

extern tree_type* c_declarator_get_type(const c_declarator* self)
{
        return self->type.head;
}

extern tree_location c_declarator_get_name_loc_or_begin(const c_declarator* self)
{
        return self->name_loc == TREE_INVALID_LOC
                ? self->loc.begin
                : self->name_loc;
}

// returns declarator type
static tree_type* c_sema_add_declarator_type(c_sema* self, c_declarator* d, tree_type* t)
{
        assert(t);
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
                tree_set_func_type_result(tail, t);
        else if (k == TTK_ARRAY)
                tree_set_array_eltype(tail, t);
        else if (k == TTK_PAREN)
                tree_set_paren_type(tail, t);
        else
                UNREACHABLE();

        d->type.tail = t;
        return d->type.head;
}

extern bool c_sema_add_direct_declarator_function_suffix(c_sema* self, c_declarator* d)
{
        return c_sema_add_declarator_type(self, 
                d, c_sema_new_function_type(self, NULL)) != NULL;
}

extern bool c_sema_add_direct_declarator_array_suffix(
        c_sema* self, c_declarator* d, tree_type_quals q, tree_expr* size_expr)
{
        return c_sema_add_declarator_type(self,
                d, c_sema_new_array_type(self, q, NULL, size_expr)) != NULL;
}

extern void c_sema_add_direct_declarator_transaction_safe_attribute(c_sema* self, c_declarator* d)
{
        assert(d->type.tail && tree_type_is(d->type.tail, TTK_FUNCTION));
        tree_set_func_type_transaction_safe(d->type.tail, true);
}

extern void c_sema_add_direct_declarator_stdcall_attribute(c_sema* self, c_declarator* d)
{
        assert(d->type.tail && tree_type_is(d->type.tail, TTK_FUNCTION));
        tree_set_func_type_cc(d->type.tail, TCC_STDCALL);
}

extern bool c_sema_add_direct_declarator_parens(c_sema* self, c_declarator* d)
{
        return c_sema_add_declarator_type(self, d, c_sema_new_paren_type(self, NULL)) != NULL;
}

extern void c_sema_add_declarator_param(c_sema* self, c_declarator* d, c_param* p)
{
        assert(p);
        tree_type* t = d->type.tail;
        assert(t && tree_get_type_kind(t) == TTK_FUNCTION);

        tree_add_func_type_param(t, self->context, c_param_get_type(p));
        if (!d->params_initialized)
                ptrvec_push(&d->params, p);
        else
                ;// todo delete(p)
}

extern bool c_sema_set_declarator_has_vararg(c_sema* self, c_declarator* d, tree_location ellipsis_loc)
{
        tree_type* t = d->type.tail;
        assert(t && tree_get_type_kind(t) == TTK_FUNCTION);

        if (d->params.size == 0)
        {
                c_error_named_argument_before_ellipsis_required(self->ccontext, ellipsis_loc);
                return false;
        }

        tree_set_func_type_vararg(t, true);
        return true;
}

extern bool c_sema_finish_declarator(c_sema* self, c_declarator* declarator, c_type_chain* pointers)
{
        if (!pointers->head)
                return true;

        if (declarator->type.tail)
        {
                bool res = c_sema_add_declarator_type(self, declarator, pointers->head) != NULL;
                declarator->type.tail = pointers->tail;
                return res;
        }

        declarator->type = *pointers;
        return true;
}

extern void c_decl_specs_init(c_decl_specs* self)
{
        self->storage_class = TSC_NONE;
        self->storage_duration = TSD_AUTOMATIC;
        self->dll_storage_class = TDSC_NONE;
        self->typespec = NULL;
        self->loc.val = TREE_INVALID_XLOC;
        self->has_inline = false;
        self->has_typedef = false;
}

extern void c_decl_specs_set_loc_begin(c_decl_specs* self, tree_location begin)
{
        self->loc.begin = begin;
}

extern void c_decl_specs_set_loc_end(c_decl_specs* self, tree_location end)
{
        self->loc.end = end;
}

extern tree_location c_decl_specs_get_loc_begin(const c_decl_specs* self)
{
        return self->loc.begin;
}

extern tree_location c_decl_specs_get_loc_end(const c_decl_specs* self)
{
        return self->loc.end;
}

extern bool c_sema_set_type_specifier(c_sema* self, c_decl_specs* ds, tree_type* ts)
{
        if (ds->typespec)
                return false; // typespec redefinition

        ds->typespec = ts;
        return true;
}

extern bool c_sema_set_typedef_specified(c_sema* self, c_decl_specs* ds)
{
        if (ds->storage_class != TSC_NONE || ds->has_typedef)
        {
                c_error_multiple_storage_classes(self->ccontext, ds);
                return false;
        }

        ds->has_typedef = true;
        return true;
}

extern bool c_sema_set_inline_specified(c_sema* self, c_decl_specs* ds)
{
        ds->has_inline = true;
        return true;
}

extern bool c_sema_set_storage_class(c_sema* self, c_decl_specs* ds, tree_storage_class sc)
{
        if (ds->storage_class != TSC_NONE || ds->has_typedef)
        {
                c_error_multiple_storage_classes(self->ccontext, ds);
                return false;
        }

        ds->storage_class = sc;

        if ((ds->storage_class == TSC_EXTERN || ds->storage_class == TSC_STATIC) && ds->storage_duration == TSD_AUTOMATIC)
                ds->storage_duration = TSD_STATIC;

        return true;
}

extern bool c_sema_set_thread_storage_duration(c_sema* self, c_decl_specs* ds)
{
        ds->storage_duration = TSD_THREAD;
        return true;
}

extern bool c_sema_set_dll_storage_class(c_sema* self, c_decl_specs* ds, tree_dll_storage_class sc)
{
        if (ds->dll_storage_class != TDSC_NONE)
        {
                c_error_multiple_storage_classes(self->ccontext, ds);
                return false;
        }

        ds->dll_storage_class = sc;
        return true;
}

static bool c_sema_check_decl_redefinition(c_sema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (tree_decl_scope_lookup(scope,
                tree_decl_is_tag(decl) ? TLK_TAG : TLK_DECL,
                tree_get_decl_name(decl), false))
        {
                c_error_decl_redefinition(self->ccontext, decl);
                return false;
        }
        return true;
}

static tree_decl* csema_add_decl_to_scope(c_sema* self, tree_decl_scope* scope, tree_decl* decl)
{
        if (!c_sema_check_decl_redefinition(self, scope, decl))
                return NULL;

        tree_decl_scope_add_decl(scope, self->context, decl);
        return decl;
}

static tree_decl* c_sema_new_external_decl(c_sema*, c_decl_specs*, c_declarator*, bool);

extern tree_decl* c_sema_handle_unused_decl_specs(c_sema* self, c_decl_specs* ds)
{
        c_declarator d;
        c_declarator_init(&d, self, CDK_UNKNOWN);
        d.loc = ds->loc;

        // todo: warning
        tree_decl* decl = c_sema_new_external_decl(self, ds, &d, false);
        if (!decl)
                return NULL;

        return csema_add_decl_to_scope(self, self->locals, decl);
}

extern tree_type* c_param_get_type(const c_param* self)
{
        return c_declarator_get_type(&self->declarator);
}

extern tree_xlocation c_param_get_loc(const c_param* self)
{
        return self->specs.loc;
}

extern c_param* c_sema_start_param(c_sema* self)
{
        c_param* p = c_context_allocate_node(self->ccontext, sizeof(c_param));
        c_decl_specs_init(&p->specs);
        c_declarator_init(&p->declarator, self, CDK_PARAM);
        return p;
}

static tree_type* c_sema_finish_decl_type(
        c_sema* self, c_decl_specs* decl_specs, c_declarator* declarator)
{
        tree_type* dt = c_sema_add_declarator_type(self, declarator, decl_specs->typespec);
        if (!dt)
                return NULL;

        return c_sema_check_type(self, dt,
                c_declarator_get_name_loc_or_begin(declarator)) ? dt : NULL;
}

static bool c_sema_check_decl_specs(const c_sema* self, 
        const c_decl_specs* specs, const c_declarator* d, tree_decl_kind dk, bool has_body_or_init)
{
        tree_location name_loc = c_declarator_get_name_loc_or_begin(d);
        if (name_loc == TREE_INVALID_LOC)
                name_loc = c_decl_specs_get_loc_begin(specs);

        tree_storage_class sc = specs->storage_class;
        tree_storage_duration sd = specs->storage_duration;
        if (dk == TDK_FUNCTION)
        {
                if (c_sema_at_block_scope(self) && sc != TSC_EXTERN && sc != TSC_IMPL_EXTERN)
                {
                        c_error_invalid_storage_class(self->ccontext, specs->loc.begin, d->name);
                        return false;
                }
        }
        else if (specs->has_inline)
        {
                c_error_invalid_specifier(self->ccontext, name_loc, CTK_INLINE, TDK_FUNCTION);
                return false;
        }

        if (dk == TDK_FIELD && (sc != TSC_NONE || sd != TSD_AUTOMATIC))
        {
                c_error_field_declared_with_storage_specifier(self->ccontext, d);
                return false;
        }

        if (sd == TSD_THREAD)
        {
                if (dk != TDK_VAR)
                {
                        c_error_invalid_specifier(self->ccontext, name_loc, CTK_THREAD_LOCAL, TDK_VAR);
                        return false;
                }
                if (sc == TSC_NONE && !c_sema_at_file_scope(self))
                {
                        c_error_variable_declared_thread_local_at_function_scope(self->ccontext, d);
                        return false;
                }
        }

        if (sc == TSC_REGISTER)
        {
                if (dk != TDK_VAR)
                {
                        c_error_invalid_specifier(self->ccontext, name_loc, CTK_REGISTER, TDK_VAR);
                        return false;
                }
                if (c_sema_at_file_scope(self))
                {
                        c_error_variable_declared_register_at_file_scope(self->ccontext, d);
                        return false;
                }
        }

        if ((sc == TSC_EXTERN || sc == TSC_STATIC) && dk == TDK_PARAM)
        {
                c_error_invalid_parameter_storage_class(self->ccontext, d);
                return false;

        }

        if (specs->dll_storage_class == TDSC_IMPORT)
        {
                if (dk != TDK_FUNCTION && dk != TDK_VAR)
                {
                        c_error_dllimport_applied_to_wrong_decl(self->ccontext, specs->loc.begin);
                        return false;
                }
                if (has_body_or_init)
                {
                        c_error_dllimport_cannot_be_applied_to_definition(self->ccontext, specs, dk);
                        return false;
                }
        }

        return true;
}

extern c_param* c_sema_finish_param(c_sema* self, c_param* p)
{
        c_declarator* d = &p->declarator;
        if (!c_sema_check_decl_specs(self, &p->specs, d, TDK_PARAM, false))
                return NULL;
        if (!c_sema_finish_decl_type(self, &p->specs, d))
                return NULL;
        return p;
}

extern tree_type* c_sema_new_type_name(c_sema* self, c_declarator* d, tree_type* typespec)
{
        return c_sema_add_declarator_type(self, d, typespec);
}

extern tree_decl* c_sema_add_decl_to_group(c_sema* self, tree_decl* group, tree_decl* decl)
{
        if (!group)
                return decl;

        if (!tree_decl_is(group, TDK_GROUP))
        {
                tree_decl* first = group;
                tree_set_decl_implicit(first, true);
                group = tree_new_decl_group(self->context,
                        self->locals, tree_get_decl_loc(first));

                tree_add_decl_in_group(group, self->context, first);
                tree_decl_scope_add_hidden_decl(self->locals, group);
        }

        tree_add_decl_in_group(group, self->context, decl);
        tree_set_decl_implicit(decl, true);
        return group;
}

static bool c_sema_compute_enumerator_value(
        c_sema* self,
        tree_decl* enum_,
        tree_location name_loc,
        tree_id name,
        tree_expr* init,
        int_value* result)
{
        size_t i32_nbits = 8 * tree_get_builtin_type_size(self->target, TBTK_INT32);
        if (init)
        {
                tree_eval_result r;
                if (!tree_eval_expr_as_integer(self->context, init, &r))
                {
                        c_error_enumerator_value_isnt_constant(self->ccontext, name_loc, name);
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

static tree_decl* c_sema_new_enumerator(
        c_sema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* init)
{
        int_value value;
        if (!c_sema_compute_enumerator_value(self, enum_, name_loc, name, init, &value))
                return NULL;

        tree_type* t = tree_new_qualified_type(self->context,
                tree_new_decl_type(self->context, enum_, true), TTQ_UNQUALIFIED);

        return tree_new_enumerator_decl(self->context, self->locals,
                tree_create_xloc(name_loc, name_loc), name, t, init, &value);
}

extern tree_decl* c_sema_define_enumerator_decl(
        c_sema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* value)
{
        tree_decl* e = c_sema_new_enumerator(self, enum_, name_loc, name, value);
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

static tree_decl* c_sema_new_enum_decl(c_sema* self, tree_location kw_loc, tree_id name)
{
        return tree_new_enum_decl(
                self->context,
                self->locals,
                tree_create_xloc(kw_loc, kw_loc),
                name);
}

static tree_decl* c_sema_lookup_or_create_enum_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool parent_lookup)
{
        tree_decl* e = tree_decl_scope_lookup(self->locals, TLK_TAG, name, parent_lookup);
        if (!e)
        {
                e = c_sema_new_enum_decl(self, kw_loc, name);
                tree_set_decl_implicit(e, true);
                if (!csema_add_decl_to_scope(self, self->locals, e))
                        return NULL;
        }
        else if (tree_get_decl_kind(e) != TDK_ENUM)
        {
                c_error_wrong_king_of_tag(self->ccontext, kw_loc, name);
                return NULL;
        }
        return e;
}

extern tree_decl* c_sema_define_enum_decl(c_sema* self, tree_location kw_loc, tree_id name)
{
        tree_decl* e = c_sema_lookup_or_create_enum_decl(self, kw_loc, name, false);
        if (!e)
                return NULL;

        if (tree_tag_decl_is_complete(e))
        {
                c_error_redefinition(self->ccontext, kw_loc, name);
                return NULL;
        }
        return e;
}

extern tree_decl* c_sema_declare_enum_decl(c_sema* self, tree_location kw_loc, tree_id name)
{
        return c_sema_lookup_or_create_enum_decl(self, kw_loc, name, true);
}

extern tree_decl* c_sema_complete_enum_decl(c_sema* self, tree_decl* enum_, tree_location end)
{
        tree_set_tag_decl_complete(enum_, true);
        tree_set_decl_loc_end(enum_, end);
        return enum_;
}

static tree_decl* c_sema_new_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        return tree_new_record_decl(
                self->context,
                self->locals,
                tree_create_xloc(kw_loc, kw_loc),
                name,
                is_union);
}

static tree_decl* c_sema_lookup_or_create_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union, bool parent_lookup)
{
        tree_decl* d = tree_decl_scope_lookup(self->locals, TLK_TAG, name, parent_lookup);
        if (!d)
        {
                d = c_sema_new_record_decl(self, kw_loc, name, is_union);
                tree_set_decl_implicit(d, true);
                if (!csema_add_decl_to_scope(self, self->locals, d))
                        return NULL;
        }
        else if (!tree_decl_is(d, TDK_RECORD) || tree_record_is_union(d) != is_union)
        {
                c_error_wrong_king_of_tag(self->ccontext, kw_loc, name);
                return NULL;
        }
        return d;
}

extern tree_decl* c_sema_define_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        tree_decl* r = c_sema_lookup_or_create_record_decl(self, kw_loc, name, is_union, false);
        if (!r)
                return NULL;

        if (tree_tag_decl_is_complete(r))
        {
                c_error_redefinition(self->ccontext, kw_loc, name);
                return NULL;
        }
        return r;
}

extern tree_decl* c_sema_declare_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union)
{
        return c_sema_lookup_or_create_record_decl(self, kw_loc, name, is_union, true);
}

extern tree_decl* c_sema_complete_record_decl(c_sema* self, tree_decl* rec, tree_location end)
{
        tree_set_tag_decl_complete(rec, true);
        tree_set_decl_loc_end(rec, end);
        return rec;
}


static bool c_sema_check_field_decl(const c_sema* self, const tree_decl* field)
{
        tree_type* mt = tree_desugar_type(tree_get_decl_type(field));
        if (tree_type_is(mt, TTK_FUNCTION))
        {
                c_error_field_function(self->ccontext, field);
                return false;
        }
        if (!c_sema_require_complete_type(self, tree_get_decl_loc_begin(field), mt))
                return false;

        tree_expr* bit_width = tree_get_field_bit_width(field);
        if (!bit_width)
                return true;

        if (!tree_type_is_integer(mt))
        {
                c_error_invalid_bitfield_type(self->ccontext, field);
                return false;
        }

        tree_eval_result r;
        if (!tree_eval_expr_as_integer(self->context, bit_width, &r))
        {
                c_error_bitfield_width_isnt_constant(self->ccontext, field);
                return false;
        }

        if (avalue_is_zero(&r.value))
        {
                c_error_bitfield_width_is_zero(self->ccontext, field);
                return false;
        }

        if (avalue_is_signed(&r.value) && avalue_get_i64(&r.value) < 0)
        {
                c_error_negative_bitfield_width(self->ccontext, field);
                return false;
        }

        if (avalue_get_u64(&r.value) > 8 * tree_get_sizeof(self->target, mt))
        {
                c_error_bitfield_width_exceeds_type(self->ccontext, field);
                return false;
        }
        return field;
}

static tree_decl* c_sema_new_field_decl(
        c_sema* self, c_decl_specs* ds, c_declarator* d, tree_expr* bit_width)
{
        if (!c_sema_check_decl_specs(self, ds, d, TDK_FIELD, false))
                return NULL;

        tree_type* t = c_sema_finish_decl_type(self, ds, d);
        if (!t)
                return NULL;

        tree_decl* field = tree_new_field_decl(
                self->context, self->locals, ds->loc, d->name, t, bit_width);

        if (!c_sema_check_field_decl(self, field))
                return NULL;

        return field;
}

static bool c_sema_inject_anonymous_record_fields(
        c_sema* self, tree_decl_scope* to, tree_decl* anon)
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

extern tree_decl* c_sema_define_field_decl(
        c_sema* self, c_decl_specs* ds, c_declarator* d, tree_expr* bit_width)
{
        tree_decl* field = c_sema_new_field_decl(self, ds, d, bit_width);
        if (!field || !csema_add_decl_to_scope(self, self->locals, field))
                return NULL;

        tree_type* type = tree_get_decl_type(field);
        if (!tree_decl_is_anon(field) || !tree_declared_type_is(type, TDK_RECORD))
                return field;

        if (!c_sema_inject_anonymous_record_fields(self, self->locals, field))
                return NULL;

        return field;
}

static tree_decl* c_sema_new_label_decl(
        c_sema* self, tree_location id_loc, tree_id id, tree_location colon_loc)
{
        return tree_new_label_decl(self->context,
                self->labels, tree_create_xloc(id_loc, colon_loc), id, NULL);
}

extern tree_decl* c_sema_define_label_decl(
        c_sema* self, tree_location name_loc, tree_id name, tree_location colon_loc, tree_stmt* stmt)
{
        tree_decl* l = c_sema_declare_label_decl(self, name_loc, name);
        if (!l)
                return NULL;

        if (tree_get_label_decl_stmt(l))
        {
                c_error_redefinition(self->ccontext, name_loc, name);
                return NULL;
        }

        tree_set_decl_loc(l, tree_create_xloc(name_loc, colon_loc));
        tree_set_label_decl_stmt(l, stmt);
        return l;
}

extern tree_decl* c_sema_declare_label_decl(c_sema* self, tree_location name_loc, tree_id name)
{
        tree_decl* l = c_sema_label_lookup(self, name);
        if (!l)
        {
                l = c_sema_new_label_decl(self, name_loc, name, name_loc);
                if (!csema_add_decl_to_scope(self, self->labels, l))
                        return NULL;
        }
        return l;
}

extern bool csema_require_variable_decl_kind(const c_sema* self, const tree_decl* decl)
{
        tree_decl_kind k = tree_get_decl_kind(decl);
        if (k == TDK_VAR)
                return true;

        if (k == TDK_FUNCTION)
                c_error_function_initialized_like_a_variable(self->ccontext, decl);
        else
                UNREACHABLE(); // todo
        
        return false;
}

extern bool csema_require_function_decl_kind(const c_sema* self, const tree_decl* decl)
{
        if (tree_decl_is(decl, TDK_FUNCTION))
                return true;
        return false;
}

static tree_decl* c_sema_new_typedef_decl(c_sema* self, c_decl_specs* ds, c_declarator* d)
{
        if (!c_sema_check_decl_specs(self, ds, d, TDK_TYPEDEF, false))
                return NULL;

        return tree_new_typedef_decl(
                self->context, self->locals, ds->loc, d->name, d->type.head);
}

static tree_decl* c_sema_define_param_decl(c_sema* self, c_param* p)
{
        tree_decl* d = tree_new_param_decl(
                self->context,
                self->locals,
                c_param_get_loc(p),
                p->declarator.name,
                p->declarator.type.head);

        return csema_add_decl_to_scope(self, self->locals, d);
}

static bool c_sema_set_function_params(c_sema* self, tree_decl* func, ptrvec* params)
{
        c_sema_enter_decl_scope(self, tree_get_func_params(func));

        for (size_t i = 0; i < params->size; i++)
                if (!c_sema_define_param_decl(self, ptrvec_get(params, i)))
                {
                        c_sema_exit_decl_scope(self);
                        return false;
                }

        c_sema_exit_decl_scope(self);
        return true;
}

static bool c_sema_check_function_decl(
        const c_sema* self, const c_decl_specs* specs, const c_declarator* d, bool has_body)
{
        tree_location func_loc = specs->loc.begin;
        if (self->function && has_body)
        {
                c_error_function_isnt_allowed_here(self->ccontext, func_loc);
                return false;
        }

        if (!c_sema_check_decl_specs(self, specs, d, TDK_FUNCTION, has_body))
                return false;

        if (!has_body)
                return true;

        tree_type* func_type = tree_desugar_type(d->type.head);
        tree_type* restype = tree_get_func_type_result(func_type);
        if (!tree_type_is_void(restype) && !c_sema_require_complete_type(self, func_loc, restype))
                return false;

        const ptrvec* params = &d->params;
        for (size_t i = 0; i < params->size; i++)
        {
                c_param* param = ptrvec_get(params, i);
                tree_location param_loc = param->specs.loc.begin;

                if (param->declarator.name == TREE_EMPTY_ID)
                {
                        c_error_parameter_name_omitted(self->ccontext, param_loc);
                        return false;
                }

                tree_type* param_type = param->declarator.type.head;
                if (!c_sema_require_complete_type(self, param_loc, param_type))
                        return false;
                if (tree_func_type_is_transaction_safe(func_type)
                        && (tree_get_type_quals(param_type) & TTQ_VOLATILE))
                {
                        c_error_volatile_param_is_not_allowed(self->ccontext, param_loc);
                        return false;
                }
        }
        return true;
}

static tree_decl* c_sema_new_function_decl(
        c_sema* self, c_decl_specs* specs, c_declarator* d, bool has_body)
{
        // c99 6.2.2.5
        // If the declaration of an identifier for a function has no storage-class specifier,
        // its linkage is external
        if (specs->storage_class == TSC_NONE)
                specs->storage_class = TSC_IMPL_EXTERN;

        if (!c_sema_check_function_decl(self, specs, d, has_body))
                return NULL;

        tree_decl* func = tree_new_func_decl(self->context, self->locals, specs->loc,
                d->name, specs->storage_class, specs->dll_storage_class, d->type.head, NULL);
        tree_set_func_inlined(func, specs->has_inline);

        return c_sema_set_function_params(self, func, &d->params) ? func : NULL;
}

static bool c_sema_check_var_decl(const c_sema* self,
        const c_decl_specs* specs, const c_declarator* d, bool has_init)
{
        if (!c_sema_check_decl_specs(self, specs, d, TDK_VAR, has_init))
                return false;

        tree_storage_class sc = specs->storage_class;
        if (sc == TSC_NONE || sc == TSC_IMPL_EXTERN)
        {
                tree_type* t = tree_desugar_type(d->type.head);
                if (tree_type_is(t, TTK_ARRAY) && tree_array_is(t, TAK_INCOMPLETE) && has_init)
                        ; // int a[] = { ... }; is ok
                else
                        return c_sema_require_complete_type(self, specs->loc.begin, t);
        }
        else if (sc == TSC_EXTERN && has_init)
        {
                c_error_extern_variable_has_an_initializer(self->ccontext, d);
                return false;
        }
        return true;
}

static tree_decl* c_sema_new_var_decl(
        c_sema* self, c_decl_specs* specs, c_declarator* d, bool has_init)
{
        // c99 6.2.2.5
        // If the declaration of an identifier for an object has file scope
        // and no storage-class specifier, its linkage is external.
        if (specs->storage_class == TSC_NONE && c_sema_at_file_scope(self))
        {
                specs->storage_class = TSC_IMPL_EXTERN;
                if (specs->storage_duration == TSD_AUTOMATIC)
                        specs->storage_duration = TSD_STATIC;
        }

        if (!c_sema_check_var_decl(self, specs, d, has_init))
                return NULL;

        return tree_new_var_decl(self->context, self->locals, specs->loc, d->name,
                specs->storage_class, specs->storage_duration, specs->dll_storage_class,
                d->type.head, NULL);
}

static tree_decl* c_sema_new_external_decl(
        c_sema* self, c_decl_specs* ds, c_declarator* d, bool has_init_or_body)
{
        if (!c_sema_finish_decl_type(self, ds, d))
                return NULL;

        if (ds->has_typedef)
                return c_sema_new_typedef_decl(self, ds, d);
        else if (tree_type_is(d->type.head, TTK_FUNCTION))
                return c_sema_new_function_decl(self, ds, d, has_init_or_body);

        return c_sema_new_var_decl(self, ds, d, has_init_or_body);
}

static bool c_sema_check_external_decl_redefinition(
        c_sema* self, tree_decl* decl, tree_decl* other, bool has_init_or_body)
{
        if (!other)
                return true;

        tree_decl_kind decl_kind = tree_get_decl_kind(decl);
        tree_decl_kind other_kind = tree_get_decl_kind(other);
        if (decl_kind != other_kind)
        {
                c_error_different_kind_of_symbol(self->ccontext, decl);
                return false;
        }

        if (decl_kind != TDK_TYPEDEF && !tree_decls_have_same_linkage(decl, other))
        {
                c_error_different_storage_class(self->ccontext, decl);
                return false;
        }

        if (!c_sema_types_are_same(self, tree_get_decl_type(decl), tree_get_decl_type(other)))
        {
                c_error_conflicting_types(self->ccontext, decl);
                return false;
        }

        if (has_init_or_body
                && ((other_kind == TDK_VAR && tree_get_var_init(other))
                 || (other_kind == TDK_FUNCTION && tree_get_func_body(other))))
        {
                c_error_decl_redefinition(self->ccontext, decl);
                return false;
        }

        return true;
}

extern tree_decl* c_sema_declare_external_decl(
        c_sema* self, c_decl_specs* ds, c_declarator* d, bool has_init_or_body)
{
        tree_decl* decl = c_sema_new_external_decl(self, ds, d, has_init_or_body);
        if (!decl)
                return NULL;

        tree_decl_kind dk = tree_get_decl_kind(decl);
        if (dk != TDK_TYPEDEF && tree_get_decl_storage_class(decl) == TSC_NONE)
                return csema_add_decl_to_scope(self, self->locals, decl);

        tree_decl* other = c_sema_local_lookup(self, tree_get_decl_name(decl), false);
        if (!other)
                return csema_add_decl_to_scope(self, self->locals, decl);

        if (!c_sema_check_external_decl_redefinition(self, decl, other, has_init_or_body))
                return NULL;

        tree_decl_scope_add_hidden_decl(self->locals, decl);
        if (has_init_or_body
                || (dk == TDK_VAR && tree_get_decl_storage_class(decl) == TSC_IMPL_EXTERN))
        {
                tree_decl_scope_update_lookup(self->locals, self->context, decl);
        }

        return decl;
}

extern void c_sema_set_var_init(c_sema* self, tree_decl* var, tree_expr* init)
{
        tree_set_var_init(var, init);
}

extern void c_sema_set_func_body(c_sema* self, tree_decl* func, tree_stmt* body)
{
        tree_set_func_body(func, body);
}
