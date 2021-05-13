#include "scc/core/num.h"
#include "scc/semantics/sema.h"
#include "scc/tree/eval.h"
#include "scc/tree/context.h"
#include "scc/tree/target.h"
#include "errors.h"
#include "compound-object.h"

extern tree_expr* c_sema_get_default_initializer(c_sema* self, tree_type* obj, tree_storage_class sc)
{
        if (sc == TSC_EXTERN)
                return NULL;

        // todo: avoid allocating integer literals
        obj = tree_desugar_type(obj);
        tree_type_kind k = tree_get_type_kind(obj);
        if (k == TTK_BUILTIN || k == TTK_POINTER || tree_type_is_enumerated(obj))
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
                tree_type* et = tree_get_array_eltype(obj);
                tree_expr* init = tree_new_init_list_expr(self->context, TREE_INVALID_LOC);
                tree_set_expr_type(init, obj);
                if (!tree_array_is(obj, TAK_CONSTANT))
                        return init;

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
                tree_set_expr_type(list, obj);

                if (tree_record_is_union(rec))
                {
                        tree_decl* first_field = tree_get_first_union_field(rec);
                        tree_designator* fd = tree_new_field_designator(
                                self->context, TREE_INVALID_LOC, first_field);
                        tree_expr* des = tree_new_designation(self->context, TREE_INVALID_LOC, 
                                c_sema_get_default_initializer(self, tree_get_decl_type(first_field), sc));
                        tree_add_designation_designator(des, self->context, fd);
                        tree_add_init_list_expr(list, self->context, des);
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

static bool walk_array_designator(
        c_sema* self,
        struct compound_object* co,
        tree_designator* designator)
{
        if (!(co->object->kind & OBJECT_ARRAY))
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
        if (co->object->kind == OBJECT_CONST_ARRAY)
        {
                if (index_val >= tree_get_array_size(co->object->type))
                {
                        c_error_array_index_in_initializer_exceeds_array_bounds(self->ccontext, index);
                        return false;
                }
        }

        set_compound_object_index(co, index_val);
        return true;
}

static bool walk_field_designator(
        c_sema* self,
        struct compound_object* co,
        tree_designator* designator)
{
        if (!(co->object->kind & OBJECT_RECORD))
        {
                c_error_field_name_not_in_record_initializer(self->ccontext, designator);
                return false;
        }

        tree_decl* rec = co->object->record.decl;
        tree_id name = tree_get_designator_field_name(designator);
        tree_id loc = tree_get_designator_loc(designator);
        tree_decl* field = c_sema_require_field_decl(self, rec, loc, name);
        if (!field)
                return false;

        while (tree_decl_is(field, TDK_INDIRECT_FIELD))
        {
                tree_decl* anon_field = tree_get_indirect_field_anon_record(field);
                set_compound_object_field(co, anon_field);
                enter_subobject(co);

                rec = tree_get_decl_type_entity(tree_get_decl_type(anon_field));
                field = tree_decl_scope_lookup(tree_get_record_fields(rec), TLK_DECL, name, false);
        }

        set_compound_object_field(co, field);
        return true;
}

static bool walk_designators(c_sema* self, struct compound_object* co, tree_expr* designation)
{
        while (exit_subobject(co))
                ;
        bool is_first = true;
        TREE_FOREACH_DESIGNATION_DESIGNATOR(designation, it, end)
        {
                bool is_field = tree_designator_is(*it, TREE_DESIGNATOR_FIELD_NAME);
                if (!is_first && !can_enter_subobject(co))
                {
                        if (is_field)
                                c_error_field_name_not_in_record_initializer(self->ccontext, *it);
                        else
                                c_error_array_index_in_non_array_intializer(self->ccontext, *it);
                        return false;
                }
                else if (!is_first)
                        enter_subobject(co);
                bool ok = is_field
                        ? walk_field_designator(self, co, *it)
                        : walk_array_designator(self, co, *it);
                if (!ok)
                        return false;
                is_first = false;
        }
        return true;
}

static void assignment_conversion_error(
        c_sema* self, c_assignment_conversion_result* acr, tree_expr* e)
{
        if (acr->kind == CACRK_RHS_NOT_AN_ARITHMETIC)
                c_error_expr_must_have_arithmetic_type(self->ccontext, tree_get_expr_loc(e));
        else if (acr->kind == CACRK_QUAL_DISCARTION)
                c_error_initialization_discards_qualifer(self->ccontext, e, acr->discarded_quals);
        else if (acr->kind == CACRK_INCOMPATIBLE_POINTERS || acr->kind == CACRK_RHS_TRANSACTION_UNSAFE)
                c_error_initialization_from_incompatible_pointer_types(self->ccontext, e);
        else
                c_error_invalid_initializer(self->ccontext, e);
}

static tree_expr* get_string_literal_semantic_initalizer(
        c_sema* self, tree_type* object, tree_expr** str);

static tree_expr* find_compatible_subobject(c_sema* self, struct compound_object* co, tree_expr** e)
{
        tree_type* et = tree_get_expr_type(*e);
        while (1)
        {
                tree_type* subtype = get_subobject_type(co);
                c_assignment_conversion_result acr;
                bool array_with_string_lit = tree_type_is_char_array(subtype)
                        && tree_expr_is(tree_desugar_expr(*e), TEK_STRING_LITERAL);
                if (c_sema_assignment_conversion(self, subtype, e, &acr)
                        || array_with_string_lit)
                {
                        return array_with_string_lit
                                ? get_string_literal_semantic_initalizer(self, subtype, e)
                                : *e;
                }
                if (!can_enter_subobject(co))
                {
                        assignment_conversion_error(self, &acr, *e);
                        return NULL;
                }
                enter_subobject(co);
        }
}

static tree_expr* _get_semantic_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr** init,
        bool is_top_level);

static tree_expr* handle_braced_scalar_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr* init,
        bool is_top_level)
{
        size_t size = tree_get_init_list_exprs_size(init);
        tree_expr** inner = tree_get_init_list_exprs_begin(init);
        if (size > 1)
        {
                c_error_too_many_initializer_values(self->ccontext,
                        tree_get_expr_loc_begin(tree_get_init_list_expr(init, 1)));
                return NULL;
        }
        if (!is_top_level)
        {
                c_error_braces_around_scalar_initializer(self->ccontext, tree_get_expr_loc_begin(init));
                return NULL;
        }
        if (tree_expr_is(*inner, TEK_DESIGNATION))
        {
                tree_designator* d = tree_get_designation_designators_begin(*inner)[0];
                if (tree_designator_is(d, TREE_DESIGNATOR_FIELD_NAME))
                        c_error_field_name_not_in_record_initializer(self->ccontext, d);
                else
                        c_error_array_index_in_non_array_intializer(self->ccontext, d);
                return NULL;

        }
        return _get_semantic_initializer(self, object, object_sc, inner, false);
}

static tree_expr* get_init_list_semantic_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr* init,
        bool is_top_level)
{
        if (!tree_type_is_record(object) && !tree_type_is_array(object))
                return handle_braced_scalar_initializer(self, object, object_sc, init, is_top_level);

        size_t size = tree_get_init_list_exprs_size(init);
        assert(size > 0);
        tree_expr** inner = tree_get_init_list_exprs_begin(init);
        if (is_top_level
                && tree_type_is_char_array(object)
                && tree_expr_is(tree_desugar_expr(*inner), TEK_STRING_LITERAL))
        {
                if (size > 1)
                {
                        c_error_too_many_initializer_values(self->ccontext, 
                                tree_get_expr_loc_begin(tree_get_init_list_expr(init, 1)));
                        return NULL;
                }
                return _get_semantic_initializer(self, object, object_sc, inner, false);
        }

        struct compound_object co;
        init_compound_object(&co, self, object, object_sc);

        for (size_t i = 0; i < size; i++)
        {
                tree_expr** item_ptr = tree_get_init_list_exprs_begin(init) + i;
                tree_expr* item = *item_ptr;
                if (tree_expr_is(item, TEK_DESIGNATION))
                {
                        if (!walk_designators(self, &co, item))
                                return NULL;
                        item_ptr = tree_get_designation_init_ptr(item);
                        item = *item_ptr;
                }

                tree_expr* sem_init = item;
                if (tree_expr_is(item, TEK_INIT_LIST))
                        sem_init = get_init_list_semantic_initializer(
                                self, get_subobject_type(&co), object_sc, item, false);
                else if (!(sem_init = find_compatible_subobject(self, &co, item_ptr)))
                                return NULL;
                
                if (!sem_init)
                        return NULL;
                initialize_subobject_with(&co, sem_init);

                if (i + 1 < size)
                {
                        tree_expr* next = tree_get_init_list_expr(init, i + 1);
                        if (!tree_expr_is(next, TEK_DESIGNATION) && !goto_next_subobject(&co))
                        {
                                c_error_too_many_initializer_values(self->ccontext, tree_get_expr_loc_begin(next));
                                return NULL;
                        }
                }
        }
        while (exit_subobject(&co))
                ;
        finish_current_object(&co);
        return co.object->sem_init;
}

static void c_fix_array_to_pointer_conversion_opt(const tree_type* object, tree_expr** expr)
{
        // When we are building a string literal we cant tell whether it is used to initialize
        // array or pointer, so we perform array-to-pointer conversion by default
        // so now, if the initialized object is array we need to remove implicit cast

        if (!object 
                || !tree_type_is(object, TTK_ARRAY)
                || !tree_expr_is(*expr, TEK_CAST) 
                || !tree_cast_is_implicit(*expr))
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

static tree_expr* get_string_literal_semantic_initalizer(
        c_sema* self, tree_type* object, tree_expr** str)
{
        object = tree_desugar_type(object);

        if (!tree_type_is_void_pointer(object)
                && !tree_type_is_char_pointer(object)
                && !tree_type_is_char_array(object))
        {
                if (tree_type_is_array(object))
                        c_error_array_cannot_be_initalized_with_string_literal(self->ccontext, *str);
                else
                        c_error_invalid_initializer(self->ccontext, *str);
                return 0;
        }
        
        c_fix_array_to_pointer_conversion_opt(object, str);
        unsigned max_len = -1U;
        if (tree_type_is(object, TTK_ARRAY) && tree_array_is(object, TAK_CONSTANT))
                max_len = tree_get_array_size(object);
        
        struct strentry* entry = tree_get_id_strentry(self->context,
                tree_get_string_literal(tree_desugar_expr(*str)));
        if (max_len < entry->size - 1)
        {
                c_error_initializer_string_is_too_long(self->ccontext, *str);
                return NULL;
        }

        tree_expr* result = *str;
        if (tree_type_is(object, TTK_ARRAY))
        {
                if (tree_array_is(object, TAK_INCOMPLETE))
                        c_sema_set_incomplete_array_size(self, object, entry->size);
                
                result = tree_new_init_list_expr(self->context, TREE_INVALID_LOC);
                for (unsigned i = 0; i < tree_get_array_size(object); i++)
                {
                        tree_expr* char_lit = c_sema_new_character_literal(self, TREE_INVALID_LOC, entry->data[i]);
                        tree_add_init_list_expr(result, self->context, char_lit);
                }
                tree_set_expr_type(result, object);
        }

        return result;
}

static tree_expr* get_ordinary_semantic_initalizer(
        c_sema* self, tree_type* object, tree_expr** init)
{
        c_assignment_conversion_result acr;
        if (!c_sema_assignment_conversion(self, object, init, &acr))
        {
                assignment_conversion_error(self, &acr, *init);
                return false;
        }

        tree_eval_result er;
        if (c_sema_at_file_scope(self) && !tree_eval_expr(self->context, *init, &er))
        {
                c_error_initializer_element_isnt_constant(self->ccontext, *init);
                return false;
        }

        return *init;
}

static tree_expr* _get_semantic_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr** init,
        bool is_top_level)
{
        if (!*init)
                return NULL;

        tree_expr* base = tree_desugar_expr(*init);
        if (tree_expr_is(base, TEK_INIT_LIST))
                return get_init_list_semantic_initializer(self, object, object_sc, base, is_top_level);
        else if (tree_expr_is(base, TEK_STRING_LITERAL))
                return get_string_literal_semantic_initalizer(self, object, init);
        return get_ordinary_semantic_initalizer(self, object, init);
}

extern tree_expr* c_sema_get_semantic_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr** init)
{
        return _get_semantic_initializer(self, object, object_sc, init, true);
}
