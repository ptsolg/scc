#include "errors.h"
#include "scc/c-common/context.h"
#include "scc/lex/misc.h"
#include "scc/lex/reswords-info.h"
#include "scc/tree/context.h"
#include "scc/tree/decl.h"

static const char* c_context_get_id_string(c_context* self, tree_id id)
{
        return tree_get_id_string(self->tree, id);
}

extern void c_error_expected_a_before_b(
        c_context* self, tree_location loc, c_token_kind a, c_token_kind b)
{
        const c_resword_info* b_info = c_get_token_kind_info(b);
        c_error(self, CES_ERROR, loc, "expected %s before %s %s",
                c_get_token_kind_info(a)->desription, b_info->desription, b_info->kind);
}

extern void c_error_expected_a_or_b_before_c(
        c_context* self, tree_location loc, c_token_kind a, c_token_kind b, c_token_kind c)
{
        const c_resword_info* c_info = c_get_token_kind_info(c);
        c_error(self, CES_ERROR, loc, "expected %s or %s before %s %s",
                c_get_token_kind_info(a)->desription,
                c_get_token_kind_info(b)->desription,
                c_info->desription,
                c_info->kind);
}

extern void c_error_expected_one_of(
        c_context* self, tree_location loc, c_token_kind* v, size_t n, c_token_kind end)
{
        char buffer[1024];
        sprintf(buffer, "expected one of: ");
        for (size_t i = 0; i < n; i++)
        {
                strcat(buffer, c_get_token_kind_info(v[i])->desription);
                if (i + 1 < n)
                        strcat(buffer, ", ");
        }
        const c_resword_info* end_info = c_get_token_kind_info(end);
        c_error(self, CES_ERROR, loc, "%s before %s %s",
                buffer, end_info->desription, end_info->kind);
}

extern void c_error_unknown_type_name(c_context* self, const c_token* id)
{
        c_error(self, CES_ERROR, c_token_get_loc(id), "unknown type name '%s'",
                c_context_get_id_string(self, c_token_get_string(id)));
}

extern void c_error_expected_type_specifier(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected type specifier");
}

extern void c_error_invalid_type_specifier(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "invalid combination of type specifiers");
}

extern void c_error_empty_struct(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty struct/union is invalid in C99");
}

extern void c_error_empty_enum(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty enum is invalid in C99");
}

extern void c_error_expected_expr(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected an expression");
}

extern void c_error_empty_initializer(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty initializer list is invalid in C99");
}

extern void c_error_function_initialized_like_a_variable_(c_context* self, const tree_decl* func)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(func),
                "function '%s' is initialized like a variable",
                c_context_get_id_string(self, tree_get_decl_name(func)));
}