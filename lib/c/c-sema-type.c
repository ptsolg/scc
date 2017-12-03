#include "scc/c/c-sema-type.h"
#include "scc/tree/tree-eval.h"

extern tree_type* csema_get_int_type(csema* self, bool signed_, bool extended)
{
        tree_builtin_type_kind k = signed_
                ? extended ? TBTK_INT64 : TBTK_INT32
                : extended ? TBTK_UINT64 : TBTK_UINT32;

        return tree_new_builtin_type(self->context, k);
}

extern tree_type* csema_get_size_t_type(csema* self)
{
        return csema_get_int_type(self, false, tree_target_is(self->target, TTARGET_X64));
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
        tree_decl* d = csema_require_local_decl(self, name_loc, TDK_TYPEDEF, name);
        if (!d)
                return NULL;

        return csema_new_decl_type(self, d, true);
}

extern bool csema_typedef_name_exists(csema* self, tree_id name)
{
        tree_decl* d = csema_get_decl(self, self->locals, name, false, true);
        if (!d)
                return false;

        return tree_get_decl_kind(d) == TDK_TYPEDEF;
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

        int_value size_value;
        tree_eval_info info;
        tree_init_eval_info(&info, self->target);
        // we'll check size-value later
        tree_eval_as_integer(&info, size, &size_value);
        return tree_new_constant_array_type(self->context, eltype, size, &size_value);
}

extern tree_type* csema_new_constant_array_type(
        csema* self, tree_type_quals quals, tree_type* eltype, uint size)
{
        int_value value;
        int_init(&value, 32, false, size);
        
        return tree_new_constant_array_type(self->context, eltype, NULL, &value);
}

extern tree_type* csema_set_type_quals(csema* self, tree_type* type, tree_type_quals quals)
{
        if (!type)
                return NULL;

        tree_set_type_quals(type, quals);
        return type;
}

extern bool csema_check_array_type(const csema* self, const tree_type* t, tree_location l)
{
        S_ASSERT(tree_type_is(t, TTK_ARRAY));

        tree_type* el = tree_get_array_eltype(t);
        if (!el)
                return false;

        if (tree_type_is(tree_desugar_type(el), TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "array of functions is not allowed");
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
                cerror(self->error_manager, CES_ERROR, l,
                        "size of array has non-integer type");
                return false;
        }

        const int_value* size_value = tree_get_constant_array_size_cvalue(t);
        if (int_is_zero(size_value)
                || (int_is_signed(size_value) && int_get_i64(size_value) < 0))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "size of array must be greater than zero");
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
                cerror(self->error_manager, CES_ERROR, l,
                        "function returning array is not allowed");
                return false;
        }
        else if (tree_type_is(r, TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "function returning function is not allowed");
                return false;
        }
        return true;
}

extern bool csema_check_type_quals(const csema* self, const tree_type* t, tree_location l)
{
        tree_type_quals q = tree_get_type_quals(t);
        if ((q & TTQ_RESTRICT) && !tree_type_is_pointer(t))
        {
                cerror(self->error_manager, CES_ERROR, l, "invalid use of 'restrict'");
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