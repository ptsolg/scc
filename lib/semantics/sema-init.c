#include "scc/core/num.h"
#include "scc/semantics/sema.h"
#include "scc/tree/eval.h"
#include "scc/tree/context.h"
#include "scc/tree/target.h"
#include "errors.h"

extern tree_expr* c_sema_get_default_initializer(c_sema* self, tree_type* obj, tree_storage_class sc)
{
        // todo: avoid allocating integer literals
        obj = tree_desugar_type(obj);
        tree_type_kind k = tree_get_type_kind(obj);
        if (k == TTK_BUILTIN || k == TTK_POINTER)
        {
                if (sc == TSC_NONE)
                        return tree_new_unknown_expr(self->context);

                tree_expr* init = c_sema_new_integer_literal(self, TREE_INVALID_LOC, 0, true, false);
                c_assignment_conversion_result acr;
                c_sema_assignment_conversion(self, obj, &init, &acr);
                assert(acr.kind == CACRK_COMPATIBLE);
                return init;
        }
        else if (k == TTK_ARRAY)
        {
                assert(tree_array_is(obj, TAK_CONSTANT));
                tree_type* et = tree_get_array_eltype(obj);
                tree_expr* init = tree_new_init_list_expr(self->context, TREE_INVALID_LOC);
                uint size = num_as_u64(tree_get_array_size_value_c(obj));
                assert(size);
                for (uint i = 0; i < size; i++)
                        tree_add_init_list_expr(init, self->context,
                                c_sema_get_default_initializer(self, et, sc));
                return init;

        }
        else if (k == TTK_DECL)
        {
                assert(tree_declared_type_is(obj, TDK_RECORD));
                tree_decl* rec = tree_get_decl_type_entity(obj);
                tree_expr* list = tree_new_init_list_expr(self->context, TREE_INVALID_LOC);

                if (tree_record_is_union(rec))
                {
                        tree_decl* first_field = tree_skip_non_field_decls(tree_get_record_fields_begin(rec));
                        tree_add_init_list_expr(list, self->context,
                                c_sema_get_default_initializer(self, tree_get_decl_type(first_field), sc));
                        return list;
                }

                TREE_FOREACH_DECL_IN_SCOPE(tree_get_record_fields(rec), it)
                {
                        if (!tree_decl_is(it, TDK_FIELD))
                                continue;
                        tree_add_init_list_expr(list, self->context,
                                c_sema_get_default_initializer(self, tree_get_decl_type(it), sc));
                }
                return list;
        }
        UNREACHABLE();
        return NULL;
}

typedef enum
{
        COK_SCALAR,
        COK_ARRAY,
        COK_STRUCT,
        COK_UNION,
} c_object_kind;

// represents an object that is being initialized
typedef struct _c_object
{
        c_object_kind kind;
        tree_type* type;
        tree_expr* semantic_initializer;
        union
        {
                struct
                {
                        uint index;
                        uint size;
                } array;

                struct
                {
                        tree_decl* decl;
                        tree_decl* field;
                } record;
        };
} c_object;

static void _c_initialize_subobject_with(c_object* self, tree_expr* e)
{
        if (self->kind == COK_SCALAR)
                self->semantic_initializer = e;
        else
        {
                uint index = self->kind == COK_ARRAY
                        ? self->array.index
                        : tree_get_field_index(self->record.field);
                tree_set_init_list_expr(self->semantic_initializer, index, e);
        }
}

#define VEC   c_object_vec
#define VEC_T c_object
#include "scc/core/vec.inc"

typedef struct
{
        c_object* object;
        tree_storage_class object_sc;
        bool build_semantic_initializer;
        struct c_object_vec objects;
        c_sema* sema;
} c_initialization_context;

static void c_init_object(c_initialization_context* ic, c_object* o, tree_type* type)
{
        type = tree_desugar_type(type);
        o->type = type;
        o->semantic_initializer = NULL;
        
        bool is_array_of_known_size = false;

        if (tree_type_is_record(type))
        {
                o->record.decl = tree_get_decl_type_entity(type);
                o->kind = tree_record_is_union(o->record.decl) ? COK_UNION : COK_STRUCT;
                o->record.field = tree_skip_non_field_decls(tree_get_record_fields_begin(o->record.decl));
        }
        else if (tree_type_is_array(type))
        {
                is_array_of_known_size = tree_array_is(type, TAK_CONSTANT);
                o->kind = COK_ARRAY;
                o->array.index = 0;
                o->array.size = is_array_of_known_size ? tree_get_array_size(type) : 0;
        }
        else
                o->kind = COK_SCALAR;

        if (!ic->build_semantic_initializer || o->kind == COK_SCALAR)
                return;

        size_t size = 1;
        if (o->kind == COK_STRUCT)
                size = tree_count_record_fields(o->record.decl);
        else if (is_array_of_known_size)
                size = o->array.size;

        o->semantic_initializer = tree_new_init_list_expr(ic->sema->context, TREE_INVALID_LOC);
        for (size_t i = 0; i < size; i++)
                tree_add_init_list_expr(o->semantic_initializer, ic->sema->context, NULL);
}

static void c_init_initialization_context(
        c_initialization_context* self,
        c_sema* sema,
        tree_type* object, 
        tree_storage_class object_sc,
        bool build_semantic_initializer)
{
        self->sema = sema;
        self->object_sc = object_sc;
        self->build_semantic_initializer = build_semantic_initializer;
        c_object_vec_init(&self->objects);

        c_object o;
        c_init_object(self, &o, object);
        c_object_vec_push(&self->objects, o);
        self->object = c_object_vec_last_ptr(&self->objects);
}

static void c_dispose_initialization_context(c_initialization_context* self)
{
        c_object_vec_drop(&self->objects);
}

static void c_set_object_field(c_initialization_context* ic, tree_decl* field)
{
        c_object* o = ic->object;
        assert(o->kind == COK_STRUCT || o->kind == COK_UNION);
        assert(tree_get_field_record(field) == o->record.decl);
        o->record.field = field;
}

static void c_set_object_index(c_initialization_context* ic, uint index)
{
        c_object* o = ic->object;

        assert(o->kind == COK_ARRAY);
        if (tree_array_is(o->type, TAK_CONSTANT))
                assert(index < o->array.size);

        o->array.index = index;
        if (tree_array_is(o->type, TAK_INCOMPLETE) && index >= o->array.size)
                o->array.size = index + 1;

        if (!ic->build_semantic_initializer)
                return;

        while (tree_get_init_list_exprs_size(o->semantic_initializer) < o->array.size)
                tree_add_init_list_expr(o->semantic_initializer, ic->sema->context, NULL);
}

static tree_type* c_get_subobject_type(const c_initialization_context* ic)
{
        tree_type* t = NULL;
        const c_object* o = ic->object;
        if (o->kind == COK_ARRAY)
                t = tree_get_array_eltype(o->type);
        else if (o->kind == COK_STRUCT || o->kind == COK_UNION)
                t = tree_get_decl_type(o->record.field);

        if (t)
                t = tree_desugar_type(t);

        return t;
}

static bool c_can_goto_next_subobject(const c_initialization_context* ic)
{
        c_object* o = ic->object;
        if (o->kind == COK_SCALAR || o->kind == COK_UNION)
                return false;
        else if (o->kind == COK_ARRAY)
        {
                return tree_array_is(o->type, TAK_INCOMPLETE)
                        ? true
                        : o->array.index + 1 < o->array.size;
        }
        else if (o->kind == COK_STRUCT)
        {
                return tree_get_record_fields_end(o->record.decl) 
                        != tree_get_next_field(o->record.field);
        }

        UNREACHABLE();
        return false;
}

static void c_goto_next_subobject(c_initialization_context* ic)
{
        assert(c_can_goto_next_subobject(ic));

        c_object* o = ic->object;
        if (o->kind == COK_ARRAY)
                c_set_object_index(ic, o->array.index + 1);
        else if (o->kind == COK_STRUCT)
                o->record.field = tree_get_next_field(o->record.field);
}

static void c_initialize_subobject_with(c_initialization_context* ic, tree_expr* e)
{
        if (ic->build_semantic_initializer)
                _c_initialize_subobject_with(ic->object, e);
}

static c_object* c_get_object_parent(const c_initialization_context* ic)
{
        assert(ic->objects.size);
        return ic->objects.size == 1
                ? NULL
                : ic->objects.items + ic->objects.size - 2;
}

static void c_sema_set_incomplete_array_size(c_sema* self, tree_type* arr, uint size)
{
        assert(tree_array_is(arr, TAK_INCOMPLETE));

        struct num size_value;
        init_int(&size_value, size,
                8 * tree_get_sizeof(self->context->target, c_sema_get_size_t_type(self)));
        tree_init_constant_array_type(arr, tree_get_array_eltype(arr), NULL, &size_value);
}

static void c_finish_subobject(c_initialization_context* ic)
{
        c_object* o = ic->object;
        if (o->kind == COK_ARRAY && tree_array_is(o->type, TAK_INCOMPLETE))
                c_sema_set_incomplete_array_size(ic->sema, o->type, o->array.size);

        if (!ic->build_semantic_initializer)
                return;

        if (o->kind == COK_SCALAR)
        {
                if (!o->semantic_initializer)
                {
                        o->semantic_initializer
                                = c_sema_get_default_initializer(ic->sema, o->type, ic->object_sc);
                }
        }
        else if (o->kind == COK_ARRAY)
        {
                tree_type* et = tree_get_array_eltype(o->type);
                TREE_FOREACH_INIT_LIST_EXPR(o->semantic_initializer, it, end)
                        if (!*it)
                                *it = c_sema_get_default_initializer(ic->sema, et, ic->object_sc);
        }
        else if (o->kind == COK_UNION)
        {
                tree_expr** v = tree_get_init_list_exprs_begin(o->semantic_initializer);
                if (!*v)
                        *v = c_sema_get_default_initializer(ic->sema,
                                tree_get_decl_type(o->record.field), ic->object_sc);
        }
        else if (o->kind == COK_STRUCT)
        {
                tree_decl* field = tree_skip_non_field_decls(tree_get_record_fields_begin(o->record.decl));
                TREE_FOREACH_INIT_LIST_EXPR(o->semantic_initializer, it, end)
                {
                        if (!*it)
                                *it = c_sema_get_default_initializer(ic->sema,
                                        tree_get_decl_type(field), ic->object_sc);
                        field = tree_get_next_field(field);
                }
        }

        assert(o->semantic_initializer);
        c_object* parent = c_get_object_parent(ic);
        if (parent)
                _c_initialize_subobject_with(parent, o->semantic_initializer);
}

static void c_initialize_object_with_string(c_initialization_context* ic, const uint8_t* data, uint size)
{
        assert(ic->object->kind == COK_ARRAY && ic->object->array.index == 0);

        if (!ic->build_semantic_initializer)
                return;

        for (uint i = 0; i < size; i++)
        {
                tree_expr* char_literal = c_sema_new_character_literal(
                        ic->sema, TREE_INVALID_LOC, (int)data[i]);
                c_initialize_subobject_with(ic, char_literal);
                if (i + 1 < size)
                        c_goto_next_subobject(ic);
        }

        c_finish_subobject(ic);
        c_set_object_index(ic, ic->object->array.size - 1);
}

static void c_enter_subobject(c_initialization_context* ic)
{
        tree_type* t = c_get_subobject_type(ic);
        assert(t && ic->objects.size > 0);
        c_object o;
        c_init_object(ic, &o, t);
        c_object_vec_push(&ic->objects, o);
        ic->object = c_object_vec_last_ptr(&ic->objects);
}

static bool c_can_exit_subobject(const c_initialization_context* ic)
{
        return ic->objects.size > 1;
}

static void c_exit_subobject(c_initialization_context* ic)
{
        assert(c_can_exit_subobject(ic));
        c_finish_subobject(ic);
        c_object_vec_pop(&ic->objects);
        ic->object = c_object_vec_last_ptr(&ic->objects);
}

static bool c_sema_check_field_designator(
        c_sema* self, c_initialization_context* ic, tree_designator* designator)
{
        if (ic->object->kind != COK_STRUCT && ic->object->kind != COK_UNION)
        {
                c_error_field_name_not_in_record_initializer(self->ccontext, designator);
                return false;
        }

        tree_decl* rec = ic->object->record.decl;
        tree_id name = tree_get_designator_field(designator);
        tree_id loc = tree_get_designator_loc(designator);
        tree_decl* field = c_sema_require_field_decl(self, rec, loc, name);
        if (!field)
                return false;

        while (tree_decl_is(field, TDK_INDIRECT_FIELD))
        {
                tree_decl* anon_field = tree_get_indirect_field_anon_record(field);
                c_set_object_field(ic, anon_field);
                c_enter_subobject(ic);

                rec = tree_get_decl_type_entity(tree_get_decl_type(anon_field));
                field = tree_decl_scope_lookup(tree_get_record_fields(rec), TLK_DECL, name, false);
        }

        c_set_object_field(ic, field);
        return true;
}

static bool c_sema_check_array_designator(
        c_sema* self, c_initialization_context* ic, tree_designator* designator)
{
        if (ic->object->kind != COK_ARRAY)
        {
                c_error_array_index_in_non_array_intializer(self->ccontext, designator);
                return false;
        }

        tree_expr* index = tree_get_designator_index(designator);
        tree_eval_result eval_result;
        if (!tree_eval_expr_as_integer(self->context, index, &eval_result))
        {
                if (eval_result.kind == TERK_FLOATING)
                        c_error_array_index_in_initializer_not_of_integer_type(self->ccontext, index);
                else
                        c_error_nonconstant_array_index_in_initializer(self->ccontext, index);
                return false;
        }

        uint index_val = num_as_u64(&eval_result.value);
        if (tree_array_is(ic->object->type, TAK_CONSTANT))
        {
                if (index_val >= tree_get_array_size(ic->object->type))
                {
                        c_error_array_index_in_initializer_exceeds_array_bounds(self->ccontext, index);
                        return false;
                }
        }

        c_set_object_index(ic, index_val);
        return true;
}

static bool c_sema_check_designated_initializer(
        c_sema* self, c_initialization_context* ic, tree_expr* designation)
{
        assert(tree_expr_is(designation, TEK_DESIGNATION));

        while (c_can_exit_subobject(ic))
                c_exit_subobject(ic);

        TREE_FOREACH_DESIGNATION_DESIGNATOR(designation, it, end)
        {
                bool succeeded = tree_designator_is_field(*it)
                        ? c_sema_check_field_designator(self, ic, *it)
                        : c_sema_check_array_designator(self, ic, *it);
                if (!succeeded)
                        return false;
                if (it + 1 != end)
                        c_enter_subobject(ic);
        }

        c_initializer_check_result r;
        if (!c_sema_check_initializer(self, c_get_subobject_type(ic), ic->object_sc,
                tree_get_designation_init(designation), &r, !ic->build_semantic_initializer))
        {
                return false;
        }
        c_initialize_subobject_with(ic, r.semantic_initializer);
        return true;
}

static bool c_sema_check_ordinary_initializer(
        c_sema* self, c_initialization_context* ic, tree_expr** expr)
{
        tree_type* et = tree_get_expr_type(*expr);
        while (ic->object->kind != COK_SCALAR
                && !c_sema_types_are_compatible(self, ic->object->type, et, true))
        {
                c_enter_subobject(ic);
        }

        c_assignment_conversion_result acr;
        if (!c_sema_assignment_conversion(self, ic->object->type, expr, &acr))
        {
                c_error_invalid_scalar_initialization(self->ccontext, *expr, &acr);
                return false;
        }

        tree_eval_result er;
        if (c_sema_at_file_scope(self) && !tree_eval_expr(self->context, *expr, &er))
        {
                c_error_initializer_element_isnt_constant(self->ccontext, *expr);
                return false;
        }

        c_initialize_subobject_with(ic, *expr);
        return true;
}

static void c_fix_array_to_pointer_conversion_opt(const tree_type* object, tree_expr** expr)
{
        // when we are building a string literal we cant tell whether it is used to initialize
        // array or pointer, so we perform array-to-pointer conversion by default
        // so now, if the initialized object is array we need to remove implicit cast

        if (!object || !tree_type_is(object, TTK_ARRAY)
                || !tree_expr_is(*expr, TEK_CAST) || !tree_cast_is_implicit(*expr))
        {
                return;
        }

        tree_expr* operand = tree_get_cast_operand(*expr);
        tree_expr* base = tree_ignore_paren_exprs(operand);
        if (tree_expr_is(base, TEK_STRING_LITERAL))
        {
                assert(tree_type_is_pointer(tree_get_expr_type(*expr)));
                *expr = operand;
        }
}

static bool c_sema_check_string_initializer(c_sema* self, c_initialization_context* ic, tree_expr** expr)
{
        assert(tree_expr_is(tree_desugar_expr(*expr), TEK_STRING_LITERAL));

        tree_type* eltype = NULL;
        while (1)
        {
                if (ic->object->kind == COK_SCALAR)
                        break;
                else if (ic->object->kind == COK_ARRAY)
                {
                        eltype = c_get_subobject_type(ic);
                        if (tree_type_is_scalar(eltype))
                                break;
                }
                c_enter_subobject(ic);
        }

        if (ic->object->kind == COK_SCALAR)
                return c_sema_check_ordinary_initializer(self, ic, expr);

        assert(ic->object->kind == COK_ARRAY);
        c_fix_array_to_pointer_conversion_opt(ic->object->type, expr);
        tree_expr* str = tree_desugar_expr(*expr);
        bool is_constant_array = tree_array_is(ic->object->type, TAK_CONSTANT);

        if (!tree_builtin_type_is(eltype, TBTK_UINT8)
                && !tree_builtin_type_is(eltype, TBTK_INT8))
        {
                c_error_array_cannot_be_initalized_with_string_literal(self->ccontext, str);
                return false;
        }

        struct strentry* entry = tree_get_id_strentry(self->context, tree_get_string_literal(str));
        uint size = (uint)(is_constant_array ? entry->size - 1 : entry->size);
        if (is_constant_array && tree_get_array_size(ic->object->type) < size)
        {
                c_error_initializer_string_is_too_long(self->ccontext, str);
                return false;
        }

        c_initialize_object_with_string(ic, (uint8_t*)entry->data, size);
        return true;
}

static bool c_sema_check_initializer_list(
        c_sema* self, c_initialization_context* ic, tree_expr* list)
{
        assert(tree_expr_is(list, TEK_INIT_LIST));

        tree_type* subobject_type = c_get_subobject_type(ic);
        if (!subobject_type || tree_type_is_scalar(subobject_type))
        {
                c_error_braces_around_scalar_initializer(self->ccontext, tree_get_expr_loc_begin(list));
                return false;
        }

        c_initializer_check_result result;
        if (!c_sema_check_initializer(self,
                subobject_type, ic->object_sc, list, &result, !ic->build_semantic_initializer))
        {
                return false;
        }

        c_initialize_subobject_with(ic, result.semantic_initializer);
        return true;
}

static bool c_goto_nearest_uninitialized_subobject(c_initialization_context* ic)
{
        while (1)
        {
                if (c_can_goto_next_subobject(ic))
                {
                        c_goto_next_subobject(ic);
                        return true;
                }

                if (c_can_exit_subobject(ic))
                        c_exit_subobject(ic);
                else
                        return false;
        }
}

static bool c_sema_check_initializer_value(
        c_sema* self, c_initialization_context* ic, tree_expr** pos)
{
        tree_expr* e = *pos;
        tree_expr_kind k = tree_get_expr_kind(e);

        if (k == TEK_INIT_LIST)
                return c_sema_check_initializer_list(self, ic, e);
        else if (k == TEK_DESIGNATION)
                return c_sema_check_designated_initializer(self, ic, e);
        else if (k == TEK_STRING_LITERAL)
                return c_sema_check_string_initializer(self, ic, pos);
        else if (k == TEK_CAST && tree_cast_is_implicit(e))
        {
                tree_expr* op = tree_ignore_paren_exprs(tree_get_cast_operand(e));
                if (tree_expr_is(op, TEK_STRING_LITERAL))
                        return c_sema_check_string_initializer(self, ic, pos);
        }
        else if (k == TEK_PAREN)
        {
                tree_expr* last_paren = e;
                tree_expr* sub = NULL;
                while (tree_expr_is(last_paren, TEK_PAREN))
                {
                        sub = tree_get_paren_expr(last_paren);
                        if (tree_expr_is(sub, TEK_PAREN))
                                last_paren = sub;
                        else
                                break;
                }

                if (!c_sema_check_initializer_value(self, ic, &sub))
                        return false;
                
                tree_set_paren_expr(last_paren, sub);
                return true;
        }

        return c_sema_check_ordinary_initializer(self, ic, pos);
}

static bool c_sema_check_object_initializer(c_sema* self, c_initialization_context* ic, tree_expr* init)
{
        tree_expr_kind k = tree_get_expr_kind(tree_ignore_paren_exprs(init));
        if (ic->object->kind == COK_ARRAY)
                return k == TEK_STRING_LITERAL || k == TEK_INIT_LIST;
        else if (ic->object->kind != COK_SCALAR)
                return k == TEK_INIT_LIST;
        return true;
}

extern bool c_sema_check_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr* init,
        c_initializer_check_result* result,
        bool check_only)
{
        assert(result);
        bool correct = false;
        result->check_only = check_only;
        result->syntactical_initializer = NULL;

        c_initialization_context ic;
        c_init_initialization_context(&ic, self, object, object_sc, !check_only);

        c_fix_array_to_pointer_conversion_opt(ic.object->type, &init);
        if (!c_sema_check_object_initializer(self, &ic, init))
        {
                c_error_invalid_initializer(self->ccontext, init);
                goto cleanup;
        }

        bool is_init_list = tree_expr_is(init, TEK_INIT_LIST);
        tree_expr** it = is_init_list ? tree_get_init_list_exprs_begin(init) : &init;
        tree_expr** end = is_init_list ? tree_get_init_list_exprs_end(init) : &init + 1;

        for (; it != end; it++)
        {
                if (!c_sema_check_initializer_value(self, &ic, it))
                        goto cleanup;

                if (it + 1 != end && !tree_expr_is(it[1], TEK_DESIGNATION)
                        && !c_goto_nearest_uninitialized_subobject(&ic))
                {
                        c_error_too_many_initializer_values(self->ccontext, 
                                tree_get_expr_loc_begin(it[1]));
                        goto cleanup;
                }
        }


        while (c_can_exit_subobject(&ic))
                c_exit_subobject(&ic);

        correct = true;
        c_finish_subobject(&ic); // finish top-level subobject
        result->semantic_initializer = ic.object->semantic_initializer;
        result->syntactical_initializer = init;

cleanup:
        c_dispose_initialization_context(&ic);
        return correct;
}
