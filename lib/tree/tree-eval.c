#include "scc/tree/tree-eval.h"
#include "scc/tree/tree-expr.h"
#include "scc/tree/tree-target.h"
#include "scc/tree/tree-context.h"

static bool tree_eval_binop(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_eval_result rr;
        if (!tree_eval_expr(context, tree_get_binop_rhs(expr), &rr))
                return false;
        if (!tree_eval_expr(context, tree_get_binop_lhs(expr), result))
                return false;

        if (result->kind == TERK_ADDRESS_CONSTANT)
                return true;
        if (rr.kind == TERK_ADDRESS_CONSTANT)
        {
                result->kind = TERK_ADDRESS_CONSTANT;
                return true;
        }

        avalue* lv = &result->value;
        avalue* rv = &rr.value;
        cmp_result cr;

        switch (tree_get_binop_kind(expr))
        {
                case TBK_MUL:
                        avalue_mul(lv, rv);
                        return true;
                case TBK_DIV:
                        return avalue_div(lv, rv) != OR_DIV_BY_ZERO;
                case TBK_MOD:
                        return avalue_mod(lv, rv) == OR_OK;
                case TBK_ADD:
                        avalue_add(lv, rv);
                        return true;
                case TBK_SUB:
                        avalue_sub(lv, rv);
                        return true;
                case TBK_SHL:
                        avalue_shl(lv, rv);
                        return true;
                case TBK_SHR:
                        avalue_shr(lv, rv);
                        return true;
                case TBK_LE:
                        avalue_init_int(lv, 32, true, avalue_cmp(lv, rv) == CR_LE);
                        return true;
                case TBK_GR:
                        avalue_init_int(lv, 32, true, avalue_cmp(lv, rv) == CR_GR);
                        return true;
                case TBK_LEQ:
                        cr = avalue_cmp(lv, rv);
                        avalue_init_int(lv, 32, true, cr == CR_LE || cr == CR_EQ);
                        return true;
                case TBK_GEQ:
                        cr = avalue_cmp(lv, rv);
                        avalue_init_int(lv, 32, true, cr == CR_GR || cr == CR_EQ);
                        return true;
                case TBK_EQ:
                        avalue_init_int(lv, 32, true, avalue_cmp(lv, rv) == CR_EQ);
                        return true;
                case TBK_NEQ:
                        avalue_init_int(lv, 32, true, avalue_cmp(lv, rv) != CR_EQ);
                        return true;
                case TBK_AND:
                        avalue_and(lv, rv);
                        return true;
                case TBK_XOR:
                        avalue_xor(lv, rv);
                        return true;
                case TBK_OR:
                        avalue_or(lv, rv);
                        return true;
                case TBK_LOG_AND:
                        avalue_init_int(lv, 32, true, !avalue_is_zero(lv) && !avalue_is_zero(rv));
                        return true;
                case TBK_LOG_OR:        
                        avalue_init_int(lv, 32, true, !avalue_is_zero(lv) || !avalue_is_zero(rv));
                        return true;

                default:
                        S_ASSERT(0 && "Invalid expression");
                case TBK_COMMA:
                case TBK_UNKNOWN:
                case TBK_ASSIGN:
                case TBK_ADD_ASSIGN:
                case TBK_SUB_ASSIGN:
                case TBK_MUL_ASSIGN:
                case TBK_DIV_ASSIGN:
                case TBK_MOD_ASSIGN:
                case TBK_SHL_ASSIGN:
                case TBK_SHR_ASSIGN:
                case TBK_AND_ASSIGN:
                case TBK_XOR_ASSIGN:
                case TBK_OR_ASSIGN:
                        return false;
        }
}

static bool tree_eval_address(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        return false;
}

static bool tree_eval_dereference(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        return false;
}

static bool tree_eval_unop(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        if (k == TUK_ADDRESS)
                return tree_eval_address(context, expr, result);
        if (k == TUK_DEREFERENCE)
                return tree_eval_dereference(context, expr, result);

        if (!tree_eval_expr_as_arithmetic(context, tree_get_unop_operand(expr), result))
                return false;
        
        switch (tree_get_unop_kind(expr))
        {
                case TUK_PLUS:
                        return true;
                case TUK_MINUS:
                        avalue_neg(&result->value);
                        return true;
                case TUK_NOT:
                        avalue_not(&result->value);
                        return true;
                case TUK_LOG_NOT:
                        result->kind = TERK_INTEGER;
                        avalue_init_int(&result->value, 32, true,
                                avalue_is_zero(&result->value) ? 1 : 0);
                        return true;
   
                default:
                        S_ASSERT(0 && "Invalid expression");
                case TUK_UNKNOWN:
                case TUK_POST_INC:
                case TUK_POST_DEC:
                case TUK_PRE_INC:
                case TUK_PRE_DEC:
                        return false;
        }
}

static bool tree_eval_conditional(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_eval_result cond;
        if (!tree_eval_expr_as_arithmetic(context, tree_get_conditional_condition(expr), &cond))
                return false;
        
        return avalue_is_zero(&cond.value)
                ? tree_eval_expr(context, tree_get_conditional_rhs(expr), result)
                : tree_eval_expr(context, tree_get_conditional_lhs(expr), result);
}

static bool tree_eval_integer_literal(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_type* t = tree_get_expr_type(expr);
        bool ext = tree_builtin_type_is(t, TBTK_INT64)
                || tree_builtin_type_is(t, TBTK_UINT64);

        result->kind = TERK_INTEGER;
        avalue_init_int(
                &result->value,
                ext ? 64 : 32,
                tree_type_is_signed_integer(t),
                tree_get_integer_literal(expr));

        return true;
}

static bool tree_eval_character_literal(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        result->kind = TERK_INTEGER;
        avalue_init_int(&result->value, 8, true, tree_get_character_literal(expr));
        return true;
}

static bool tree_eval_floating_literal(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        result->kind = TERK_FLOATING;

        const float_value* value = tree_get_floating_literal_cvalue(expr);
        if (tree_builtin_type_is(tree_get_expr_type(expr), TBTK_FLOAT))
                avalue_init_sp(&result->value, float_get_sp(value));
        else
                avalue_init_dp(&result->value, float_get_dp(value));

        return true;
}

static bool tree_eval_cast(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_type* cast_type = tree_get_expr_type(expr);
        if (!tree_type_is_arithmetic(cast_type))
                return false;

        tree_expr* e = tree_get_cast_operand(expr);
        tree_type* et = tree_get_expr_type(e);
        if (!tree_eval_expr(context, e, result))
                return false;

        if (result->kind == TERK_ADDRESS_CONSTANT)
        {
                if (tree_type_is_pointer(cast_type))
                        return true;

                result->kind = TERK_INVALID;
                result->error = expr;
                return false;

        }

        tree_builtin_type_kind builtin = tree_get_builtin_type_kind(cast_type);
        if (builtin == TBTK_FLOAT)
        {
                result->kind = TERK_FLOATING;
                avalue_to_sp(&result->value);
        }
        else if (builtin == TBTK_DOUBLE)
        {
                result->kind = TERK_FLOATING;
                avalue_to_dp(&result->value);
        }
        else
        {
                S_ASSERT(tree_type_is_integer(cast_type));

                result->kind = TERK_INTEGER;
                uint bits = 8 * tree_get_builtin_type_size(context->target, builtin);
                avalue_to_int(&result->value, bits, tree_type_is_signed_integer(cast_type));
        }

        return true;
}

static bool tree_eval_sizeof(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_type* t = tree_sizeof_contains_type(expr)
                ? tree_get_sizeof_type(expr)
                : tree_get_expr_type(tree_get_sizeof_expr(expr));

        result->kind = TERK_INTEGER;
        avalue_init_int(
                &result->value,
                tree_get_pointer_size(context->target) * 8,
                false,
                tree_get_sizeof(context->target, t));

        return true;
}

static bool tree_eval_member(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        return false;
}

static bool tree_eval_decl(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_decl* d = tree_get_decl_expr_entity(expr);
        if (!tree_decl_is(d, TDK_ENUMERATOR))
                return false;

        result->kind = TERK_INTEGER;
        avalue_init_int(&result->value,
                8 * tree_get_builtin_type_size(context->target, TBTK_INT32),
                true,
                int_get_i32(tree_get_enumerator_cvalue(d)));

        return true;
}

extern bool tree_eval_expr(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        S_ASSERT(result);

        result->kind = TERK_INVALID;
        result->error = expr;
        avalue_init_int(&result->value, 32, false, 0);

        if (!expr)
                return TERK_INVALID;

        switch (tree_get_expr_kind(expr))
        {
                case TEK_BINARY:
                        return tree_eval_binop(context, expr, result);
                case TEK_UNARY:
                        return tree_eval_unop(context, expr, result);
                case TEK_CONDITIONAL:
                        return tree_eval_conditional(context, expr, result);
                case TEK_INTEGER_LITERAL:
                        return tree_eval_integer_literal(context, expr, result);
                case TEK_CHARACTER_LITERAL:
                        return tree_eval_character_literal(context, expr, result);
                case TEK_FLOATING_LITERAL:
                        return tree_eval_floating_literal(context, expr, result);
                case TEK_DECL:
                        return tree_eval_decl(context, expr, result);
                case TEK_MEMBER:
                        return tree_eval_member(context, expr, result);
                case TEK_CAST:
                        return tree_eval_cast(context, expr, result);
                case TEK_SIZEOF:
                        return tree_eval_sizeof(context, expr, result);
                case TEK_PAREN:
                        return tree_eval_expr(context, tree_get_paren_expr(expr), result);
                case TEK_IMPL_INIT:
                        return tree_eval_expr(context, tree_get_impl_init_expr(expr), result);

                default:
                        S_ASSERT(0 && "Invalid expression");
                case TEK_UNKNOWN:
                case TEK_CALL:
                case TEK_SUBSCRIPT:
                case TEK_STRING_LITERAL:
                case TEK_DESIGNATION:
                case TEK_INIT_LIST:
                        return false;
        }
}

extern bool tree_eval_expr_as_integer(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        if (!tree_eval_expr(context, expr, result))
                return false;

        return result->kind == TERK_INTEGER;
}

extern bool tree_eval_expr_as_arithmetic(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        if (!tree_eval_expr(context, expr, result))
                return false;

        return result->kind == TERK_INTEGER || result->kind == TERK_FLOATING;
}