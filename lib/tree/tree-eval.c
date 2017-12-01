#include "scc/tree/tree-eval.h"

extern void tree_init_eval_info(tree_eval_info* self, const tree_target_info* target)
{
        self->_target = target;
        self->_error = NULL;
}

extern const tree_expr* tree_get_eval_eror(const tree_eval_info* self)
{
        return self->_error;
}

extern bool tree_eval_as_integer(tree_eval_info* info, const tree_expr* expr, int_value* result)
{
        avalue r;
        if (!tree_eval_as_arithmetic(info, expr, &r))
                return false;

        if (!avalue_is_int(&r))
                return false;

        *result = avalue_get_int(&r);
        return true;
}

static bool tree_eval_mul(avalue* lhs, const avalue* rhs)
{
        avalue_mul(lhs, rhs);
        return true;
}

static bool tree_eval_div(avalue* lhs, const avalue* rhs)
{
        return avalue_div(lhs, rhs) != OR_DIV_BY_ZERO;
}

static bool tree_eval_mod(avalue* lhs, const avalue* rhs)
{
        return avalue_mod(lhs, rhs) == OR_OK;
}

static bool tree_eval_add(avalue* lhs, const avalue* rhs)
{
        avalue_add(lhs, rhs);
        return true;
}

static bool tree_eval_sub(avalue* lhs, const avalue* rhs)
{
        avalue_sub(lhs, rhs);
        return true;
}

static bool tree_eval_shl(avalue* lhs, const avalue* rhs)
{
        return avalue_shl(lhs, rhs) != OR_INVALID;
}

static bool tree_eval_shr(avalue* lhs, const avalue* rhs)
{
        return avalue_shr(lhs, rhs) != OR_INVALID;
}

static bool tree_eval_le(avalue* lhs, const avalue* rhs)
{
        int v = avalue_cmp(lhs, rhs) == CR_LE;
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_gr(avalue* lhs, const avalue* rhs)
{
        int v = avalue_cmp(lhs, rhs) == CR_GR;
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_leq(avalue* lhs, const avalue* rhs)
{
        int v = avalue_cmp(lhs, rhs) == CR_LE || avalue_cmp(lhs, rhs) == CR_EQ;
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_geq(avalue* lhs, const avalue* rhs)
{
        int v = avalue_cmp(lhs, rhs) == CR_GR || avalue_cmp(lhs, rhs) == CR_EQ;
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_eq(avalue* lhs, const avalue* rhs)
{
        int v = avalue_cmp(lhs, rhs) == CR_EQ;
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_neq(avalue* lhs, const avalue* rhs)
{
        int v = avalue_cmp(lhs, rhs) != CR_EQ;
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_and(avalue* lhs, const avalue* rhs)
{
        return avalue_and(lhs, rhs) != OR_INVALID;
}

static bool tree_eval_xor(avalue* lhs, const avalue* rhs)
{
        return avalue_xor(lhs, rhs) != OR_INVALID;
}

static bool tree_eval_or(avalue* lhs, const avalue* rhs)
{
        return avalue_or(lhs, rhs) != OR_INVALID;
}

static bool tree_eval_log_and(avalue* lhs, const avalue* rhs)
{
        int v = !avalue_is_zero(lhs) && !avalue_is_zero(rhs);
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool tree_eval_log_or(avalue* lhs, const avalue* rhs)
{
        int v = !avalue_is_zero(lhs) || !avalue_is_zero(rhs);
        avalue_init_int(lhs, 32, true, v);
        return true;
}

static bool(*const tree_binop_eval_table[TBK_SIZE])(avalue*, const avalue*) =
{
        NULL, // TBK_UNKNOWN
        &tree_eval_mul, // TBK_MUL
        &tree_eval_div, // TBK_DIV
        &tree_eval_mod, // TBK_MOD
        &tree_eval_add, // TBK_ADD
        &tree_eval_sub, // TBK_SUB
        &tree_eval_shl, // TBK_SHL
        &tree_eval_shr, // TBK_SHR
        &tree_eval_le, // TBK_LE
        &tree_eval_gr, // TBK_GR
        &tree_eval_leq, // TBK_LEQ
        &tree_eval_geq, // TBK_GEQ
        &tree_eval_eq, // TBK_EQ
        &tree_eval_neq, // TBK_NEQ
        &tree_eval_and, // TBK_AND
        &tree_eval_xor, // TBK_XOR
        &tree_eval_or, // TBK_OR
        &tree_eval_log_and, // TBK_LOG_AND
        &tree_eval_log_or, // TBK_LOG_OR
        NULL, // TBK_ASSIGN
        NULL, // TBK_ADD_ASSIGN
        NULL, // TBK_SUB_ASSIGN
        NULL, // TBK_MUL_ASSIGN
        NULL, // TBK_DIV_ASSIGN
        NULL, // TBK_MOD_ASSIGN
        NULL, // TBK_SHL_ASSIGN
        NULL, // TBK_SHR_ASSIGN
        NULL, // TBK_AND_ASSIGN
        NULL, // TBK_XOR_ASSIGN
        NULL, // TBK_OR_ASSIGN
        NULL, // TBK_COMMA
};

static bool tree_eval_binop(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        const tree_expr* lhs = tree_get_binop_lhs(expr);
        const tree_expr* rhs = tree_get_binop_rhs(expr);
        const tree_type* lt = tree_get_expr_type(lhs);
        const tree_type* rt = tree_get_expr_type(rhs);

        if (!tree_type_is_arithmetic(lt) || !tree_type_is_arithmetic(rt))
                return false;

        if (!tree_eval_as_arithmetic(info, lhs, result))
                return false;

        avalue rr;
        if (!tree_eval_as_arithmetic(info, rhs, &rr))
                return false;

        tree_binop_kind k = tree_get_binop_kind(expr);
        TREE_ASSERT_BINOP_KIND(k);

        if (!tree_binop_eval_table[k] || !tree_binop_eval_table[k](result, &rr))
        {
                info->_error = expr;
                return false;
        }

        return true;
}

static bool tree_eval_plus(avalue* result)
{
        return true;
}

static bool tree_eval_minus(avalue* result)
{
        avalue_neg(result);
        return true;
}

static bool tree_eval_not(avalue* result)
{
        return avalue_not(result) != OR_INVALID;
}

static bool tree_eval_log_not(avalue* result)
{
        avalue_init_int(result, 32, true, avalue_is_zero(result) ? 1 : 0);
        return true;
}

static bool(*const tree_unop_eval_table[TUK_SIZE])(avalue*) =
{
        NULL, // TUK_UNKNOWN
        NULL, // TUK_POST_INC
        NULL, // TUK_POST_DEC
        NULL, // TUK_PRE_INC
        NULL, // TUK_PRE_DEC
        &tree_eval_plus, // TUK_PLUS
        &tree_eval_minus, // TUK_MINUS
        &tree_eval_not, // TUK_NOT
        &tree_eval_log_not, // TUK_LOG_NOT
        NULL, // TUK_DEREFERENCE
        NULL, // TUK_ADDRESS
};

static bool tree_eval_unop(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        if (!expr)
                return false;

        tree_unop_kind k = tree_get_unop_kind(expr);
        TREE_ASSERT_UNOP_KIND(k);

        if (!tree_eval_as_arithmetic(info, tree_get_unop_expr(expr), result))
                return false;
        if (!tree_unop_eval_table[k])
        {
                info->_error = expr;
                return false;
        }

        return tree_unop_eval_table[k](result);
}

static bool tree_eval_conditional(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        avalue cond;
        if (!tree_eval_as_arithmetic(info, tree_get_conditional_condition(expr), &cond))
                return false;

        return avalue_is_zero(&cond)
                ? tree_eval_as_arithmetic(info, tree_get_conditional_rhs(expr), result)
                : tree_eval_as_arithmetic(info, tree_get_conditional_lhs(expr), result);
}

static bool tree_eval_int_literal(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        tree_type* t = tree_get_expr_type(expr);
        bool ext = tree_builtin_type_is(t, TBTK_INT64)
                || tree_builtin_type_is(t, TBTK_UINT64);

        avalue_init_int(
                result,
                ext ? 64 : 32,
                tree_type_is_signed_integer(t),
                tree_get_integer_literal(expr));
        return true;
}

static bool tree_eval_char_literal(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        avalue_init_int(result, 8, true, tree_get_character_literal(expr));
        return true;
}

static bool tree_eval_flt_literal(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        if (tree_builtin_type_is(tree_get_expr_type(expr), TBTK_FLOAT))
                avalue_init_sp(result, tree_get_floating_literal(expr));
        else
                avalue_init_dp(result, tree_get_floating_lliteral(expr));
        return true;
}

static bool tree_eval_cast(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        tree_type* cast_type = tree_get_expr_type(expr);
        if (!tree_type_is_arithmetic(cast_type))
        {
                info->_error = expr;
                return false;
        }

        tree_expr* e = tree_get_cast_expr(expr);
        tree_type* et = tree_get_expr_type(e);
        if (!tree_eval_as_arithmetic(info, e, result))
                return false;

        tree_builtin_type_kind cast = tree_get_builtin_type_kind(cast_type);
        if (cast == TBTK_FLOAT)
                avalue_to_sp(result);
        else if (cast == TBTK_DOUBLE)
                avalue_to_dp(result);
        else
        {
                S_ASSERT(tree_type_is_integer(cast_type));
                uint bits = 8 * tree_get_builtin_type_size(info->_target, cast);
                avalue_to_int(result, bits, tree_type_is_signed_integer(cast_type));
        }

        return true;
}

static bool tree_eval_sizeof(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        tree_type* t = tree_sizeof_is_unary(expr)
                ? tree_get_expr_type(tree_get_sizeof_expr(expr))
                : tree_get_sizeof_type(expr);

        avalue_init_int(
                result,
                tree_get_pointer_size(info->_target) * 8,
                false,
                tree_get_sizeof(info->_target, t));
        return true;
}

static bool tree_eval_paren(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        return tree_eval_as_arithmetic(info, tree_get_paren_expr(expr), result);
}

static bool tree_eval_decl(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        tree_decl* d = tree_get_decl_expr_entity(expr);
        if (!tree_decl_is(d, TDK_ENUMERATOR))
                return false;

        avalue_init_int(result,
                8 * tree_get_builtin_type_size(info->_target, TBTK_INT32),
                true,
                int_get_i32(tree_get_enumerator_cvalue(d)));
        return true;
}

static bool tree_eval_impl_init(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        return tree_eval_as_arithmetic(info, tree_get_impl_init_expr(expr), result);
}

static bool(*const tree_eval_table[TEK_SIZE])(
        tree_eval_info*, const tree_expr*, avalue*) =
{
        NULL, // TEK_UNKNOWN
        &tree_eval_binop, // TEK_BINARY
        &tree_eval_unop, // TEK_UNARY
        NULL, // TEK_CALL
        NULL, // TEK_SUBSCRIPT
        &tree_eval_conditional, // TEK_CONDITIONAL
        &tree_eval_int_literal, // TEK_INTEGER_LITERAL
        &tree_eval_char_literal, // TEK_CHARACTER_LITERAL
        &tree_eval_flt_literal, // TEK_FLOATING_LITERAL
        NULL, // TEK_STRING_LITERAL
        &tree_eval_decl, // TEK_DECL
        NULL, // TEK_MEMBER
        &tree_eval_cast, // TEK_EXPLICIT_CAST
        &tree_eval_cast, // TEK_IMPLICIT_CAST
        &tree_eval_sizeof, // TEK_SIZEOF
        &tree_eval_paren, // TEK_PAREN
        NULL, // TEK_INIT
        &tree_eval_impl_init, // TEK_IMPL_INIT
};

extern bool tree_eval_as_arithmetic(tree_eval_info* info, const tree_expr* expr, avalue* result)
{
        if (!expr)
                return false;

        tree_expr_kind k = tree_get_expr_kind(expr);
        TREE_ASSERT_EXPR_KIND(k);

        if (!tree_eval_table[k])
        {
                info->_error = expr;
                return false;
        }

        return tree_eval_table[k](info, expr, result);
}
