#include "c-sema-type.h"
#include <libtree/tree-eval.h>

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
        tree_decl* d = csema_get_local_decl(self, name);
        if (!d)
                return false;

        return tree_get_decl_kind(d) == TDK_TYPEDEF;
}

extern tree_type* csema_new_pointer(csema* self, tree_type_quals quals, tree_type* target)
{
        tree_type* ptr = tree_new_qual_type(self->context, quals,
                tree_new_pointer_type(self->context, target));

        return csema_set_pointer_target(self, ptr, target);
}

extern tree_type* csema_set_pointer_target(csema* self, tree_type* pointer, tree_type* target)
{
        tree_set_pointer_target(pointer, target);
        return pointer;
}

extern tree_type* csema_new_function_type(csema* self, tree_type* restype)
{
        tree_type* f = tree_new_qual_type(self->context, TTQ_UNQUALIFIED,
                tree_new_function_type(self->context, NULL));

        return csema_set_function_restype(self, f, restype);
}

extern tree_type* csema_set_function_restype(
        csema* self, tree_type* func, tree_type* restype)
{
        //if (!restype)
        //        return func;

        //tree_type* dt = tree_desugar_type(restype);
        //if (tree_type_is(dt, TTK_ARRAY))
        //{
        //        cerror(self->error_manager, CES_ERROR, loc,
        //                "function returning array is not allowed");
        //        return NULL;
        //}
        //else if (tree_type_is(dt, TTK_FUNCTION))
        //{
        //        cerror(self->error_manager, CES_ERROR, loc,
        //                "function returning function is not allowed");
        //        return NULL;
        //}

        tree_set_function_restype(func, restype);
        return func;
}

extern tree_type* csema_add_function_type_param(csema* self, tree_type* func, cparam* param)
{
        // todo
        tree_type* param_type = param->declarator.type.head;
        tree_add_function_type_param(func, param_type);
        return func;
}

extern tree_type* csema_new_paren_type(csema* self, tree_type* next)
{
        return csema_set_paren_type(self, tree_new_paren_type(self->context, NULL), next);
}

extern tree_type* csema_set_paren_type(csema* self, tree_type* paren, tree_type* next)
{
        tree_set_paren_type(paren, next);
        return paren;
}

extern tree_type* csema_new_array_type(
        csema* self,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size)
{
        tree_type* arr = tree_new_qual_type(self->context, quals,
                tree_new_array_type(self->context, NULL, size));
        //if (!size)
        //        return arr;

        //if (!tree_type_is_integer(tree_get_expr_type(size)))
        //{
        //        cerror(self->error_manager, CES_ERROR, loc,
        //                "size of array has non-integer type");
        //        return NULL;
        //}

        //tree_eval_info i;
        //int_value v;
        //tree_init_eval_info(&i, self->target);
        //if (!tree_eval_as_integer(&i, size, &v))
        //        return NULL;

        //if (int_is_zero(&v) || (int_is_signed(&v) && int_get_i64(&v) < 0))
        //{
        //        cerror(self->error_manager, CES_ERROR, loc,
        //                "size of array must be greater than zero");
        //        return NULL;
        //}

        return csema_set_array_eltype(self, arr, eltype);
}

extern tree_type* csema_set_array_eltype(
        csema* self, tree_type* array, tree_type* eltype)
{
        //if (!eltype)
        //        return array;

        //if (tree_type_is(tree_desugar_type(eltype), TTK_FUNCTION))
        //{
        //        cerror(self->error_manager, CES_ERROR, loc,
        //                "array of functions is not allowed");
        //        return NULL;
        //}

        //if (!csema_require_complete_type(self, loc, eltype))
        //        return NULL;

        tree_set_array_eltype(array, eltype);
        return array;
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

        tree_expr* size = tree_get_array_size(t);
        if (!size)
                return true;

        if (!tree_type_is_integer(tree_get_expr_type(size)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "size of array has non-integer type");
                return false;
        }

        tree_eval_info i;
        int_value v;
        tree_init_eval_info(&i, self->target);
        if (!tree_eval_as_integer(&i, size, &v))
                return false;

        if (int_is_zero(&v) || (int_is_signed(&v) && int_get_i64(&v) < 0))
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

        tree_type* r = tree_desugar_type(tree_get_function_restype(t));
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
                        t = tree_get_function_restype(t);
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