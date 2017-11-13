#include "scc/ssa/ssaize-expr.h"
#include "scc/tree/tree-expr.h"

extern ssa_value* ssaize_binary_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_unary_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_call_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_subscript_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_conditional_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_integer_literal(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_character_literal(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_floating_literal(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_string_literal(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_decl_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_member_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_cast_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_sizeof_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_paren_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_init_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_impl_init_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

S_STATIC_ASSERT(TEK_SIZE == 18, "ssaize_expr_table needs an update");
static ssa_value* (*ssaize_expr_table[TEK_SIZE])(ssaizer*, const tree_expr*) =
{
        NULL, // TEK_UNKNOWN
        &ssaize_binary_expr, // TEK_BINARY
        &ssaize_unary_expr, // TEK_UNARY
        &ssaize_call_expr, // TEK_CALL
        &ssaize_subscript_expr, // TEK_SUBSCRIPT
        &ssaize_conditional_expr, // TEK_CONDITIONAL
        &ssaize_integer_literal, // TEK_INTEGER_LITERAL
        &ssaize_character_literal, // TEK_CHARACTER_LITERAL
        &ssaize_floating_literal, // TEK_FLOATING_LITERAL
        &ssaize_string_literal, // TEK_STRING_LITERAL
        &ssaize_decl_expr, // TEK_DECL
        &ssaize_member_expr, // TEK_MEMBER
        &ssaize_cast_expr, // TEK_EXPLICIT_CAST
        &ssaize_cast_expr, // TEK_IMPLICIT_CAST
        &ssaize_sizeof_expr, // TEK_SIZEOF
        &ssaize_paren_expr, // TEK_PAREN
        &ssaize_init_expr, // TEK_INIT
        &ssaize_impl_init_expr, // TEK_IMPL_INIT
};

extern ssa_value* ssaize_expr(ssaizer* self, const tree_expr* expr)
{
        S_ASSERT(expr);
        tree_expr_kind k = tree_get_expr_kind(expr);
        S_ASSERT(k >= 0 && k < TEK_SIZE);
        return ssaize_expr_table[k](self, expr);
}
