#include "scc/c/c-sema-type.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-eval.h"

extern bool csema_types_are_same(const csema* self, const tree_type* a, const tree_type* b)
{
        return tree_compare_types(a, b) == TTEK_EQ;
}

extern bool csema_types_are_compatible(const csema* self, const tree_type* a, const tree_type* b)
{
        // todo
        return csema_types_are_same(self, a, b);
}

extern bool csema_require_complete_type(const csema* self, tree_location loc, const tree_type* type)
{
        if (tree_type_is_incomplete(type))
        {
                cerror_incomplete_type(self->logger, loc);
                return false;
        }
        return true;
}

extern tree_type* csema_get_int_type(csema* self, bool signed_, bool extended)
{
        tree_builtin_type_kind k = signed_
                ? extended ? TBTK_INT64 : TBTK_INT32
                : extended ? TBTK_UINT64 : TBTK_UINT32;

        return tree_new_builtin_type(self->context, k);
}

extern tree_type* csema_get_size_t_type(csema* self)
{
        return csema_get_int_type(self, false, tree_target_is(self->target, TTAK_X86_64));
}

extern tree_type* csema_get_float_type(csema* self)
{
        return tree_new_builtin_type(self->context, TBTK_FLOAT);
}

extern tree_type* csema_get_double_type(csema* self)
{
        return tree_new_builtin_type(self->context, TBTK_DOUBLE);
}

extern tree_type* csema_get_char_type(csema* self)
{
        return tree_new_builtin_type(self->context, TBTK_INT8);
}

extern tree_type* csema_get_logical_operation_type(csema* self)
{
        return tree_new_builtin_type(self->context, TBTK_INT32);
}

extern tree_type* csema_get_type_for_string_literal(csema* self, tree_id id)
{
        strentry entry;
        bool found = tree_get_id_strentry(self->context, id, &entry);
        S_ASSERT(found);

        int_value size;
        int_init(&size, 32, false, entry.size);

        return tree_new_constant_array_type(self->context,
                csema_get_char_type(self), NULL, &size);
}

extern tree_type* csema_new_builtin_type(
        csema* self, tree_type_quals q, tree_builtin_type_kind k)
{
        if (k == TBTK_INVALID)
                return NULL;

        return tree_new_qual_type(self->context, q, tree_new_builtin_type(self->context, k));
}

extern tree_type* csema_new_decl_type(csema* self, tree_decl* d, bool referenced)
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

extern tree_type* csema_new_typedef_name(csema* self, tree_location name_loc, tree_id name)
{
        tree_decl* d = csema_require_local_decl(self, name_loc, name);
        if (!d || !tree_decl_is(d, TDK_TYPEDEF))
                return NULL;

        return csema_new_decl_type(self, d, true);
}

extern tree_type* csema_new_pointer(csema* self, tree_type_quals quals, tree_type* target)
{
        return tree_new_qual_type(self->context, quals, 
                tree_new_pointer_type(self->context, target)); 
}

extern tree_type* csema_new_function_type(csema* self, tree_type* restype)
{
        return tree_new_function_type(self->context, restype);
}

extern tree_type* csema_new_paren_type(csema* self, tree_type* next)
{
        return tree_new_paren_type(self->context, next);
}

extern tree_type* csema_new_array_type(
        csema* self,
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

extern tree_type* csema_new_constant_array_type(
        csema* self, tree_type_quals quals, tree_type* eltype, uint size)
{
        int_value value;
        int_init(&value, 32, false, size);
        
        return tree_new_constant_array_type(self->context, eltype, NULL, &value);
}

extern bool csema_typedef_name_exists(csema* self, tree_id name)
{
        tree_decl* d = csema_local_lookup(self, name, TLK_DECL);
        return d && tree_decl_is(d, TDK_TYPEDEF);
}

extern bool csema_check_array_type(const csema* self, const tree_type* t, tree_location l)
{
        S_ASSERT(tree_type_is(t, TTK_ARRAY));

        tree_type* el = tree_get_array_eltype(t);
        if (!el)
                return false;

        if (tree_type_is(tree_desugar_type(el), TTK_FUNCTION))
        {
                cerror_array_of_functions(self->logger, l);
                return false;
        }

        if (!csema_require_complete_type(self, l, el))
                return false;

        tree_expr* size_expr = tree_array_is(t, TAK_CONSTANT)
                ? tree_get_constant_array_size_expr(t)
                : NULL;
        if (!size_expr)
                return true;

        if (!tree_type_is_integer(tree_get_expr_type(size_expr)))
        {
                cerror_array_size_isnt_integer(self->logger, l);
                return false;
        }

        const int_value* size_value = tree_get_constant_array_size_cvalue(t);
        if (int_is_zero(size_value)
                || (int_is_signed(size_value) && int_get_i64(size_value) < 0))
        {
                cerror_array_must_be_greater_than_zero(self->logger, l);
                return false;
        }

        return true;
}

extern bool csema_check_function_type(const csema* self, const tree_type* t, tree_location l)
{
        S_ASSERT(tree_type_is(t, TTK_FUNCTION));

        tree_type* r = tree_desugar_type(tree_get_function_type_result(t));
        if (tree_type_is(r, TTK_ARRAY))
        {
                cerror_function_returning_array(self->logger, l);
                return false;
        }
        else if (tree_type_is(r, TTK_FUNCTION))
        {
                cerror_function_returning_function(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_check_type_quals(const csema* self, const tree_type* t, tree_location l)
{
        tree_type_quals q = tree_get_type_quals(t);
        if ((q & TTQ_RESTRICT) && !tree_type_is_pointer(t))
        {
                cerror_invalid_use_of_restrict(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_check_pointer_type(const csema* self, const tree_type* t, tree_location l)
{
        S_ASSERT(tree_type_is(t, TTK_POINTER));
        return csema_check_type_quals(self, t, l);
}

extern bool csema_check_type(const csema* self, const tree_type* t, tree_location l)
{
        while (1)
        {
                t = tree_desugar_ctype(t);
                tree_type_kind k = tree_get_type_kind(t);

                if (k == TTK_FUNCTION)
                {
                        if (!csema_check_function_type(self, t, l))
                                return false;
                        t = tree_get_function_type_result(t);
                }
                else if (k == TTK_ARRAY)
                {
                        if (!csema_check_array_type(self, t, l))
                                return false;
                        t = tree_get_array_eltype(t);
                }
                else if (k == TTK_POINTER)
                {
                        if (!csema_check_pointer_type(self, t, l))
                                return false;
                        t = tree_get_pointer_target(t);
                }
                else if (k == TTK_BUILTIN || k == TTK_DECL)
                        return csema_check_type_quals(self, t, l);
                else
                        return false;
        }
}