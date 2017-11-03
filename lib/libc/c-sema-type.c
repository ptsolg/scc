#include "c-sema-type.h"

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

extern tree_type* csema_new_function_type(
        csema* self, tree_location loc, tree_type* restype)
{
        tree_type* f = tree_new_qual_type(self->context, TTQ_UNQUALIFIED,
                tree_new_function_type(self->context, NULL));

        return csema_set_function_restype(self, loc, f, restype);
}

extern tree_type* csema_set_function_restype(
        csema* self, tree_location loc, tree_type* func, tree_type* restype)
{
        if (!restype)
                return func;

        tree_type* dt = tree_desugar_type(restype);
        if (tree_type_is(dt, TTK_ARRAY))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "function returning array is not allowed");
                return NULL;
        }
        else if (tree_type_is(dt, TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "function returning function is not allowed");
                return NULL;
        }

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
        tree_location loc,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size)
{
        tree_type* arr = tree_new_qual_type(self->context, quals,
                tree_new_array_type(self->context, NULL, size));

        return csema_set_array_eltype(self, loc, arr, eltype);
}

extern tree_type* csema_set_array_eltype(
        csema* self, tree_location loc, tree_type* array, tree_type* eltype)
{
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
