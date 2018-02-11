#include "scc/c/c-token.h"
#include "scc/c/c-context.h"

extern ctoken* ctoken_new(ccontext* context, ctoken_kind kind, tree_location loc)
{
        return ctoken_new_ex(context, kind, loc, sizeof(struct _ctoken_base));
}

extern ctoken* ctoken_new_ex(
        ccontext* context, ctoken_kind kind, tree_location loc, ssize size)
{
        // todo
        ctoken* t = callocate(context, size);
        if (!t)
                return NULL;

        ctoken_set_kind(t, kind);
        ctoken_set_loc(t, loc);
        return t;
}

extern ctoken* ctoken_new_string_ex(
        ccontext* context, ctoken_kind kind, tree_location loc, tree_id ref, ssize size)
{
        ctoken* t = ctoken_new_ex(context, kind, loc, size);
        if (!t)
                return NULL;

        ctoken_set_string(t, ref);
        return t;
}

extern ctoken* ctoken_new_string(ccontext* context, tree_location loc, tree_id ref)
{
        return ctoken_new_string_ex(context, CTK_CONST_STRING, loc, ref,
                sizeof(struct _cstring_token));
}

extern ctoken* ctoken_new_angle_string(ccontext* context, tree_location loc, tree_id ref)
{
        return ctoken_new_string_ex(context, CTK_ANGLE_STRING, loc, ref,
                sizeof(struct _cstring_token));
}

extern ctoken* ctoken_new_id(ccontext* context, tree_location loc, tree_id id)
{
        return ctoken_new_string_ex(context, CTK_ID, loc, id, sizeof(struct _cstring_token));
}

extern ctoken* ctoken_new_float(ccontext* context, tree_location loc, float val)
{
        ctoken* t = ctoken_new_ex(context, CTK_CONST_FLOAT, loc, sizeof(struct _cfloat_token));
        if (!t)
                return NULL;

        ctoken_set_float(t, val);
        return t;
}

extern ctoken* ctoken_new_double(ccontext* context, tree_location loc, ldouble val)
{
        ctoken* t = ctoken_new_ex(context, CTK_CONST_DOUBLE, loc, sizeof(struct _cdouble_token));
        if (!t)
                return NULL;

        ctoken_set_double(t, val);
        return t;
}

extern ctoken* ctoken_new_int(
        ccontext* context, tree_location loc, suint64 val, bool is_signed, int ls)
{
        ctoken* t = ctoken_new_ex(context, CTK_CONST_INT, loc, sizeof(struct _cint_token));
        if (!t)
                return NULL;

        ctoken_set_int(t, val);
        ctoken_set_int_signed(t, is_signed);
        ctoken_set_int_ls(t, ls);
        return t;
}

extern ctoken* ctoken_new_char(ccontext* context, tree_location loc, int val)
{
        ctoken* t = ctoken_new_ex(context, CTK_CONST_CHAR, loc, sizeof(struct _cchar_token));
        if (!t)
                return NULL;

        ctoken_set_char(t, val);
        return t;
}

extern ctoken* ctoken_new_wspace(ccontext* context, tree_location loc, int count)
{
        ctoken* t = ctoken_new_ex(context, CTK_WSPACE, loc, sizeof(struct _cwspace_token));
        if (!t)
                return NULL;

        ctoken_set_spaces(t, count);
        return t;
}

extern ctoken* ctoken_new_pp_num(ccontext* context, tree_location loc, tree_id ref)
{
        return ctoken_new_string_ex(context, CTK_PP_NUM, loc, ref, sizeof(ctoken));
}
