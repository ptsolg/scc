#include "scc/core/num.h"
#include "scc/semantics/sema.h"
#include "scc/tree/eval.h"
#include "scc/tree/target.h"
#include "scc/tree/context.h"
#include "scc/tree/expr.h"
#include "errors.h"

extern bool c_sema_types_are_compatible(
        const c_sema* self, const tree_type* a, const tree_type* b, bool unqualify)
{
        if (unqualify)
        {
                a = tree_get_modified_type_c(a);
                b = tree_get_modified_type_c(b);
        }
        return tree_compare_types(a, b) == TTEK_EQ;
}

extern bool c_sema_require_complete_type(const c_sema* self, tree_location loc, const tree_type* type)
{
        if (tree_type_is_incomplete(type))
        {
                c_error_incomplete_type(self->ccontext, loc);
                return false;
        }
        return true;
}

extern tree_type* c_sema_get_int_type(c_sema* self, bool signed_, bool extended)
{
        tree_builtin_type_kind k = signed_
                ? extended ? TBTK_INT64 : TBTK_INT32
                : extended ? TBTK_UINT64 : TBTK_UINT32;

        return tree_new_builtin_type(self->context, k);
}

extern tree_type* c_sema_get_size_t_type(c_sema* self)
{
        return c_sema_get_int_type(self, false, tree_target_is(self->target, TTAK_X86_64));
}

extern tree_type* c_sema_get_float_type(c_sema* self)
{
        return tree_new_builtin_type(self->context, TBTK_FLOAT);
}

extern tree_type* c_sema_get_double_type(c_sema* self)
{
        return tree_new_builtin_type(self->context, TBTK_DOUBLE);
}

extern tree_type* c_sema_get_char_type(c_sema* self)
{
        return tree_new_builtin_type(self->context, TBTK_INT8);
}

extern tree_type* c_sema_get_logical_operation_type(c_sema* self)
{
        return tree_new_builtin_type(self->context, TBTK_INT32);
}

extern tree_type* c_sema_get_type_for_string_literal(c_sema* self, tree_id id)
{
        struct strentry* entry = tree_get_id_strentry(self->context, id);
        assert(entry);

        struct num size;
        init_int(&size, entry->size, 32);

        return tree_new_constant_array_type(self->context, c_sema_get_char_type(self), NULL, &size);
}

extern tree_type* c_sema_get_builtin_type(
        c_sema* self, tree_type_quals q, tree_builtin_type_kind k)
{
        if (k == TBTK_INVALID)
                return NULL;

        return tree_new_qualified_type(self->context, tree_new_builtin_type(self->context, k), q);
}

extern tree_type* c_sema_new_decl_type(c_sema* self, tree_decl* d, bool referenced)
{
        if (!d)
                return NULL;
        //todo?

        // if decl is declared within type specifier it needs to be set to implicit
        if (!referenced)
                tree_set_decl_implicit(d, true);

        return tree_new_qualified_type(self->context, 
                tree_new_decl_type(self->context, d, referenced), TTQ_UNQUALIFIED);
}

extern tree_type* c_sema_new_typedef_name(c_sema* self, tree_location name_loc, tree_id name)
{
        tree_decl* d = c_sema_require_local_decl(self, name_loc, name);
        if (!d || !tree_decl_is(d, TDK_TYPEDEF))
                return NULL;

        return c_sema_new_decl_type(self, d, true);
}

extern tree_type* c_sema_new_pointer_type(c_sema* self, tree_type_quals quals, tree_type* target)
{
        return tree_new_qualified_type(self->context, tree_new_pointer_type(self->context, target), quals); 
}

extern tree_type* c_sema_new_function_type(c_sema* self, tree_type* restype)
{
        return tree_new_modified_type(self->context, tree_new_func_type(self->context, restype));
}

extern tree_type* c_sema_new_paren_type(c_sema* self, tree_type* next)
{
        return tree_new_paren_type(self->context, next);
}

extern tree_type* c_sema_new_array_type(
        c_sema* self,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size)
{
        if (!size)
                return tree_new_incomplete_array_type(self->context, eltype);

        tree_eval_result result;
        // we'll check size-value later
        tree_eval_expr(self->context, size, &result);
        
        struct num size_value;
        init_int(&size_value, 0, 32);
        if (num_is_integral(&result.value))
                size_value = result.value;

        return tree_new_constant_array_type(self->context, eltype, size, &size_value);
}

extern tree_type* c_sema_new_constant_array_type(
        c_sema* self, tree_type_quals quals, tree_type* eltype, uint size)
{
        struct num value;
        init_int(&value, size, 32);
        
        return tree_new_constant_array_type(self->context, eltype, NULL, &value);
}

extern void c_sema_set_incomplete_array_size(c_sema* sema, tree_type* arr, uint size)
{
        assert(tree_array_is(arr, TAK_INCOMPLETE));

        struct num size_value;
        init_int(&size_value, size,
                8 * tree_get_sizeof(sema->context->target, c_sema_get_size_t_type(sema)));
        tree_init_constant_array_type(arr, tree_get_array_eltype(arr), NULL, &size_value);
}

extern bool c_sema_typedef_name_exists(c_sema* self, tree_id name)
{
        tree_decl* d = c_sema_local_lookup(self, name, TLK_DECL);
        return d && tree_decl_is(d, TDK_TYPEDEF);
}

extern bool c_sema_check_array_type(const c_sema* self, const tree_type* t, tree_location l)
{
        assert(tree_type_is(t, TTK_ARRAY));

        tree_type* el = tree_get_array_eltype(t);
        if (!el)
                return false;

        if (tree_type_is(tree_desugar_type(el), TTK_FUNCTION))
        {
                c_error_array_of_functions(self->ccontext, l);
                return false;
        }

        if (!c_sema_require_complete_type(self, l, el))
                return false;

        tree_expr* size_expr = tree_array_is(t, TAK_CONSTANT) ? tree_get_array_size_expr(t) : NULL;
        if (!size_expr)
                return true;

        if (!tree_type_is_integer(tree_get_expr_type(size_expr)))
        {
                c_error_array_size_isnt_integer(self->ccontext, l);
                return false;
        }

        const struct num* size_value = tree_get_array_size_value_c(t);
        if (num_is_zero(size_value)
                || (size_value->kind == NUM_INT && num_i64(size_value) < 0))
        {
                c_error_array_must_be_greater_than_zero(self->ccontext, l);
                return false;
        }

        return true;
}

extern bool c_sema_check_function_type(const c_sema* self, const tree_type* t, tree_location l)
{
        assert(tree_type_is(t, TTK_FUNCTION));

        tree_type* r = tree_desugar_type(tree_get_func_type_result(t));
        if (tree_type_is(r, TTK_ARRAY))
        {
                c_error_function_returning_array(self->ccontext, l);
                return false;
        }
        else if (tree_type_is(r, TTK_FUNCTION))
        {
                c_error_function_returning_function(self->ccontext, l);
                return false;
        }
        return true;
}

extern bool c_sema_check_type_quals(const c_sema* self, const tree_type* t, tree_location l)
{
        tree_type_quals q = tree_get_type_quals(t);
        if ((q & TTQ_RESTRICT) && !tree_type_is_pointer(t))
        {
                c_error_invalid_use_of_restrict(self->ccontext, l);
                return false;
        }
        return true;
}

extern bool c_sema_check_pointer_type(const c_sema* self, const tree_type* t, tree_location l)
{
        assert(tree_type_is(t, TTK_POINTER));
        return c_sema_check_type_quals(self, t, l);
}

extern bool c_sema_check_type(const c_sema* self, const tree_type* t, tree_location l)
{
        while (1)
        {
                t = tree_desugar_type_c(t);
                tree_type_kind k = tree_get_type_kind(t);

                if (k == TTK_FUNCTION)
                {
                        if (!c_sema_check_function_type(self, t, l))
                                return false;
                        t = tree_get_func_type_result(t);
                }
                else if (k == TTK_ARRAY)
                {
                        if (!c_sema_check_array_type(self, t, l))
                                return false;
                        t = tree_get_array_eltype(t);
                }
                else if (k == TTK_POINTER)
                {
                        if (!c_sema_check_pointer_type(self, t, l))
                                return false;
                        t = tree_get_pointer_target(t);
                }
                else if (k == TTK_BUILTIN || k == TTK_DECL)
                        return c_sema_check_type_quals(self, t, l);
                else
                        return false;
        }
}
