#include "scc/c/c-sema-type.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-eval.h"
#include "scc/tree/tree-target.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-expr.h"

extern bool c_sema_types_are_same(const c_sema* self, const tree_type* a, const tree_type* b)
{
        return tree_compare_types(a, b) == TTEK_EQ;
}

extern bool c_sema_types_are_compatible(const c_sema* self, const tree_type* a, const tree_type* b)
{
        // todo
        return c_sema_types_are_same(self, a, b);
}

extern bool c_sema_require_complete_type(const c_sema* self, tree_location loc, const tree_type* type)
{
        if (tree_type_is_incomplete(type))
        {
                c_error_incomplete_type(self->logger, loc);
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
        strentry entry;
        bool found = tree_get_id_strentry(self->context, id, &entry);
        assert(found);

        int_value size;
        int_init(&size, 32, false, entry.size);

        return tree_new_constant_array_type(self->context,
                c_sema_get_char_type(self), NULL, &size);
}

extern tree_type* c_sema_get_builtin_type(
        c_sema* self, tree_type_quals q, tree_builtin_type_kind k)
{
        if (k == TBTK_INVALID)
                return NULL;

        return tree_new_qual_type(self->context, q, tree_new_builtin_type(self->context, k));
}

extern tree_type* c_sema_new_decl_type(c_sema* self, tree_decl* d, bool referenced)
{
        if (!d)
                return NULL;
        //todo?

        // if decl is declared within type specifier it needs to be set to implicit
        if (!referenced)
                tree_set_decl_implicit(d, true);

        return tree_new_qual_type(self->context, TTQ_UNQUALIFIED,
                tree_new_decl_type(self->context, d, referenced));
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
        return tree_new_qual_type(self->context, quals, 
                tree_new_pointer_type(self->context, target)); 
}

extern tree_type* c_sema_new_function_type(c_sema* self, tree_type* restype)
{
        return tree_new_function_type(self->context, restype);
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
        
        int_value size_value;
        int_init(&size_value, 32, false, 0);
        if (avalue_is_int(&result.value))
                size_value = avalue_get_int(&result.value);

        return tree_new_constant_array_type(self->context, eltype, size, &size_value);
}

extern tree_type* c_sema_new_constant_array_type(
        c_sema* self, tree_type_quals quals, tree_type* eltype, uint size)
{
        int_value value;
        int_init(&value, 32, false, size);
        
        return tree_new_constant_array_type(self->context, eltype, NULL, &value);
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
                c_error_array_of_functions(self->logger, l);
                return false;
        }

        if (!c_sema_require_complete_type(self, l, el))
                return false;

        tree_expr* size_expr = tree_array_is(t, TAK_CONSTANT)
                ? tree_get_constant_array_size_expr(t)
                : NULL;
        if (!size_expr)
                return true;

        if (!tree_type_is_integer(tree_get_expr_type(size_expr)))
        {
                c_error_array_size_isnt_integer(self->logger, l);
                return false;
        }

        const int_value* size_value = tree_get_constant_array_size_cvalue(t);
        if (int_is_zero(size_value)
                || (int_is_signed(size_value) && int_get_i64(size_value) < 0))
        {
                c_error_array_must_be_greater_than_zero(self->logger, l);
                return false;
        }

        return true;
}

extern bool c_sema_check_function_type(const c_sema* self, const tree_type* t, tree_location l)
{
        assert(tree_type_is(t, TTK_FUNCTION));

        tree_type* r = tree_desugar_type(tree_get_function_type_result(t));
        if (tree_type_is(r, TTK_ARRAY))
        {
                c_error_function_returning_array(self->logger, l);
                return false;
        }
        else if (tree_type_is(r, TTK_FUNCTION))
        {
                c_error_function_returning_function(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_check_type_quals(const c_sema* self, const tree_type* t, tree_location l)
{
        tree_type_quals q = tree_get_type_quals(t);
        if ((q & TTQ_RESTRICT) && !tree_type_is_pointer(t))
        {
                c_error_invalid_use_of_restrict(self->logger, l);
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
                t = tree_desugar_ctype(t);
                tree_type_kind k = tree_get_type_kind(t);

                if (k == TTK_FUNCTION)
                {
                        if (!c_sema_check_function_type(self, t, l))
                                return false;
                        t = tree_get_function_type_result(t);
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