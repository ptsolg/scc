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
        return c_token_new_string_ex(context, CTK_ANGLE_STRING, loc, ref, sizeof(struct _c_string_token));
}

extern c_token* c_token_new_id(c_context* context, tree_location loc, tree_id id)
{
        return c_token_new_string_ex(context, CTK_ID, loc, id, sizeof(struct _c_string_token));
}

extern c_token* c_token_new_end_of_macro(c_context* context, tree_location loc, tree_id macro)
{
        return c_token_new_string_ex(context, CTK_EOM, loc, macro, sizeof(struct _c_string_token));
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

extern c_token* c_token_new_pp_num(c_context* context, tree_location loc, tree_id ref)
{
        return c_token_new_string_ex(context, CTK_PP_NUM, loc, ref, sizeof(c_token));
}

extern c_token* c_token_copy(c_context* context, c_token* token)
{
        tree_location loc = c_token_get_loc(token);
        c_token_kind k = c_token_get_kind(token);
        if (k == CTK_ID)
                return c_token_new_id(context, loc, c_token_get_string(token));
        else if (k == CTK_CONST_STRING)
                return c_token_new_string(context, loc, c_token_get_string(token));
        else if (k == CTK_ANGLE_STRING)
                return c_token_new_angle_string(context, loc, c_token_get_string(token));
        else if (k == CTK_CONST_FLOAT)
                return c_token_new_float(context, loc, c_token_get_float(token));
        else if (k == CTK_CONST_DOUBLE)
                return c_token_new_double(context, loc, c_token_get_double(token));
        else if (k == CTK_CONST_INT)
                return c_token_new_int(context, loc, c_token_get_int(token),
                        c_token_int_is_signed(token), c_token_get_int_ls(token));
        else if (k == CTK_CONST_CHAR)
                return c_token_new_char(context, loc, c_token_get_char(token));
        else if (k == CTK_WSPACE)
                return c_token_new_wspace(context, loc, c_token_get_spaces(token));
        else if (k == CTK_PP_NUM)
                return c_token_new_pp_num(context, loc, c_token_get_string(token));

        return c_token_new(context, k, loc);
}

extern c_token* c_token_copy_with_new_loc(c_context* context, c_token* token, tree_location new_loc)
{
        c_token* copy = c_token_copy(context, token);
        if (copy)
                c_token_set_loc(copy, new_loc);
        return copy;
}