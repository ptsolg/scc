#include "scc/c/c-token.h"
#include "scc/c/c-context.h"

extern c_token* c_token_new(c_context* context, c_token_kind kind, tree_location loc)
{
        return c_token_new_ex(context, kind, loc, sizeof(struct _c_token_base));
}

extern c_token* c_token_new_ex(
        c_context* context, c_token_kind kind, tree_location loc, ssize size)
{
        // todo
        c_token* t = c_context_allocate_node(context, size);
        if (!t)
                return NULL;

        c_token_set_kind(t, kind);
        c_token_set_loc(t, loc);
        return t;
}

extern c_token* c_token_new_string_ex(
        c_context* context, c_token_kind kind, tree_location loc, tree_id ref, ssize size)
{
        c_token* t = c_token_new_ex(context, kind, loc, size);
        if (!t)
                return NULL;

        c_token_set_string(t, ref);
        return t;
}

extern c_token* c_token_new_string(c_context* context, tree_location loc, tree_id ref)
{
        return c_token_new_string_ex(context, CTK_CONST_STRING, loc, ref,
                sizeof(struct _c_string_token));
}

extern c_token* c_token_new_angle_string(c_context* context, tree_location loc, tree_id ref)
{
        return c_token_new_string_ex(context, CTK_ANGLE_STRING, loc, ref,
                sizeof(struct _c_string_token));
}

extern c_token* c_token_new_id(c_context* context, tree_location loc, tree_id id)
{
        return c_token_new_string_ex(context, CTK_ID, loc, id, sizeof(struct _c_string_token));
}

extern c_token* c_token_new_float(c_context* context, tree_location loc, float val)
{
        c_token* t = c_token_new_ex(context, CTK_CONST_FLOAT, loc, sizeof(struct _c_float_token));
        if (!t)
                return NULL;

        c_token_set_float(t, val);
        return t;
}

extern c_token* c_token_new_double(c_context* context, tree_location loc, ldouble val)
{
        c_token* t = c_token_new_ex(context, CTK_CONST_DOUBLE, loc, sizeof(struct _c_double_token));
        if (!t)
                return NULL;

        c_token_set_double(t, val);
        return t;
}

extern c_token* c_token_new_int(
        c_context* context, tree_location loc, suint64 val, bool is_signed, int ls)
{
        c_token* t = c_token_new_ex(context, CTK_CONST_INT, loc, sizeof(struct _c_int_token));
        if (!t)
                return NULL;

        c_token_set_int(t, val);
        c_token_set_int_signed(t, is_signed);
        c_token_set_int_ls(t, ls);
        return t;
}

extern c_token* c_token_new_char(c_context* context, tree_location loc, int val)
{
        c_token* t = c_token_new_ex(context, CTK_CONST_CHAR, loc, sizeof(struct _c_char_token));
        if (!t)
                return NULL;

        c_token_set_char(t, val);
        return t;
}

extern c_token* c_token_new_wspace(c_context* context, tree_location loc, int count)
{
        c_token* t = c_token_new_ex(context, CTK_WSPACE, loc, sizeof(struct _c_wspace_token));
        if (!t)
                return NULL;

        c_token_set_spaces(t, count);
        return t;
}

extern c_token* ctoken_new_pp_num(c_context* context, tree_location loc, tree_id ref)
{
        return c_token_new_string_ex(context, CTK_PP_NUM, loc, ref, sizeof(c_token));
}
