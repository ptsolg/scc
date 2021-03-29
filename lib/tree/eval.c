#include "scc/tree/eval.h"
#include "scc/core/num.h"
#include "scc/tree/expr.h"
#include "scc/tree/target.h"
#include "scc/tree/context.h"
#include "scc/tree/type.h"

static bool tree_eval_binop(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_eval_result rr;
        if (!tree_eval_expr(context, tree_get_binop_rhs(expr), &rr))
                return false;
        if (!tree_eval_expr(context, tree_get_binop_lhs(expr), result))
                return false;

        if (result->kind == TERK_ADDRESS_CONSTANT)
                return rr.kind == TERK_INTEGER;
        if (rr.kind == TERK_ADDRESS_CONSTANT)
        {
                if (result->kind != TERK_INTEGER)
                        return false;

                result->kind = TERK_ADDRESS_CONSTANT;
                return true;
        }

        struct num* lv = &result->value;
        struct num* rv = &rr.value;
        int cr;

        switch (tree_get_binop_kind(expr))
        {
                case TBK_MUL:
                        num_mul(lv, rv);
                        return true;
                case TBK_DIV:
                        return num_div(lv, rv) != OR_DIV_BY_ZERO;
                case TBK_MOD:
                        return num_mod(lv, rv) == OR_OK;
                case TBK_ADD:
                        num_add(lv, rv);
                        return true;
                case TBK_SUB:
                        num_sub(lv, rv);
                        return true;
                case TBK_SHL:
                        num_bit_shl(lv, rv);
                        return true;
                case TBK_SHR:
                        num_bit_shr(lv, rv);
                        return true;
                case TBK_LE:
                        init_int(lv, num_cmp(lv, rv) == -1, 32);
                        return true;
                case TBK_GR:
                        init_int(lv, num_cmp(lv, rv) == 1, 32);
                        return true;
                case TBK_LEQ:
                        cr = num_cmp(lv, rv);
                        init_int(lv, cr <= 0, 32);
                        return true;
                case TBK_GEQ:
                        cr = num_cmp(lv, rv);
                        init_int(lv, cr >= 0, 32);
                        return true;
                case TBK_EQ:
                        init_int(lv, num_cmp(lv, rv) == 0, 32);
                        return true;
                case TBK_NEQ:
                        init_int(lv, num_cmp(lv, rv) != 0, 32);
                        return true;
                case TBK_AND:
                        num_bit_and(lv, rv);
                        return true;
                case TBK_XOR:
                        num_bit_xor(lv, rv);
                        return true;
                case TBK_OR:
                        num_bit_or(lv, rv);
                        return true;
                case TBK_LOG_AND:
                        init_int(lv, !num_is_zero(lv) && !num_is_zero(rv), 32);
                        return true;
                case TBK_LOG_OR:        
                        init_int(lv, !num_is_zero(lv) || !num_is_zero(rv), 32);
                        return true;

                default:
                        assert(0 && "Invalid expression");
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

static bool _tree_eval_expr(tree_context*, const tree_expr*, tree_eval_result*, bool);

static bool tree_eval_address(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        if (!_tree_eval_expr(context, tree_get_unop_operand(expr), result, true))
                return false;
        result->kind = TERK_ADDRESS_CONSTANT;
        return true;
}

static bool tree_eval_unop(
        tree_context* context, const tree_expr* expr, tree_eval_result* result, bool addresof_operand)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        if (k == TUK_ADDRESS)
                return tree_eval_address(context, expr, result);
        if (k == TUK_DEREFERENCE)
        {
                return addresof_operand 
                        ? tree_eval_expr(context, tree_get_unop_operand(expr), result)
                        : false;
        }

        if (!tree_eval_expr_as_arithmetic(context, tree_get_unop_operand(expr), result))
                return false;
        
        switch (tree_get_unop_kind(expr))
        {
                case TUK_PLUS:
                        return true;
                case TUK_MINUS:
                        num_neg(&result->value);
                        return true;
                case TUK_NOT:
                        num_bit_neg(&result->value);
                        return true;
                case TUK_LOG_NOT:
                        result->kind = TERK_INTEGER;
                        init_int(&result->value, num_is_zero(&result->value), 32);
                        return true;
   
                default:
                        assert(0 && "Invalid expression");
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
        
        return num_is_zero(&cond.value)
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
        uint64_t lit = tree_get_integer_literal(expr);
        unsigned bits = ext ? 64 : 32;
        if (tree_type_is_signed_integer(t))
                init_int(&result->value, lit, bits);
        else
                init_uint(&result->value, lit, bits);
        return true;
}

static bool tree_eval_character_literal(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        result->kind = TERK_INTEGER;
        init_int(&result->value, tree_get_character_literal(expr), 8);
        return true;
}

static bool tree_eval_floating_literal(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        result->kind = TERK_FLOATING;

        const struct num* value = tree_get_floating_literal_cvalue(expr);
        if (tree_builtin_type_is(tree_get_expr_type(expr), TBTK_FLOAT))
                init_f32(&result->value, num_f32(value));
        else
                init_f64(&result->value, num_f64(value));

        return true;
}

static bool tree_eval_cast(
        tree_context* context, const tree_expr* expr, tree_eval_result* result, bool addressof_operand)
{
        if (!_tree_eval_expr(context, tree_get_cast_operand(expr), result, addressof_operand))
                return false;

        tree_type* cast_type = tree_desugar_type(tree_get_expr_type(expr));
        if (result->kind == TERK_ADDRESS_CONSTANT)
        {
                if (tree_type_is_pointer(cast_type))
                        return true;

                result->kind = TERK_INVALID;
                result->error = expr;
                return false;
        }

        if (!tree_type_is_arithmetic(cast_type))
        {
                result->kind = TERK_INVALID;
                return false;
        }

        tree_builtin_type_kind builtin = tree_get_builtin_type_kind(cast_type);
        if (builtin == TBTK_FLOAT)
        {
                result->kind = TERK_FLOATING;
                num_to_f32(&result->value);
        }
        else if (builtin == TBTK_DOUBLE)
        {
                result->kind = TERK_FLOATING;
                num_to_f64(&result->value);
        }
        else
        {
                assert(tree_type_is_integer(cast_type));

                result->kind = TERK_INTEGER;
                uint bits = 8 * tree_get_builtin_type_size(context->target, builtin);
                if (tree_type_is_signed_integer(cast_type))
                        num_to_int(&result->value, bits);
                else
                        num_to_uint(&result->value, bits);
        }

        return true;
}

static bool tree_eval_sizeof(tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        tree_type* t = tree_sizeof_contains_type(expr)
                ? tree_get_sizeof_type(expr)
                : tree_get_expr_type(tree_get_sizeof_expr(expr));

        result->kind = TERK_INTEGER;
        init_uint(
                &result->value,
                tree_get_sizeof(context->target, t),
                tree_get_pointer_size(context->target) * 8);

        return true;
}

static bool tree_eval_decl(
        tree_context* context, const tree_decl* decl, tree_eval_result* result, bool addressof_operand)
{
        tree_decl_kind k = tree_get_decl_kind(decl);
        if (k == TDK_VAR || k == TDK_FIELD || k == TDK_INDIRECT_FIELD)
        {
                bool is_array = tree_type_is(tree_desugar_type(tree_get_decl_type(decl)), TTK_ARRAY);
                if (!addressof_operand && !is_array)
                        return false;
                result->kind = TERK_ADDRESS_CONSTANT;
                return true;
        }
        else if (k == TDK_FUNCTION)
        {
                result->kind = TERK_ADDRESS_CONSTANT;
                return true;
        }
        else if (k != TDK_ENUMERATOR)
                return false;

        result->kind = TERK_INTEGER;
        init_int(&result->value,
                num_i64(tree_get_enumerator_cvalue(decl)),
                8 * tree_get_builtin_type_size(context->target, TBTK_INT32));

        return true;
}

static bool tree_eval_member(
        tree_context* context, const tree_expr* expr, tree_eval_result* result, bool addressof_operand)
{
        if (tree_member_expr_is_arrow(expr))
                return false;
        if (!_tree_eval_expr(context, tree_get_member_expr_lhs(expr), result, true))
                return false;
        if (!tree_eval_decl(context, tree_get_member_expr_decl(expr), result, addressof_operand))
                return false;
        result->kind = TERK_ADDRESS_CONSTANT;
        return true;
}

static bool tree_eval_subscript(
        tree_context* context, const tree_expr* expr, tree_eval_result* result, bool addressof_operand)
{
        if (!addressof_operand)
                return false;
        if (!_tree_eval_expr(context, tree_get_subscript_lhs(expr), result, true))
                return false;
        if (!tree_eval_expr_as_integer(context, tree_get_subscript_rhs(expr), result))
                return false;
        result->kind = TERK_ADDRESS_CONSTANT;
        return true;
}

static bool _tree_eval_expr(
        tree_context* context, const tree_expr* expr, tree_eval_result* result, bool addressof_operand)
{
        assert(result);

        result->kind = TERK_INVALID;
        result->error = expr;
        init_uint(&result->value, 0, 32);

        if (!expr)
                return TERK_INVALID;

        switch (tree_get_expr_kind(expr))
        {
                case TEK_BINARY:
                        return tree_eval_binop(context, expr, result);
                case TEK_UNARY:
                        return tree_eval_unop(context, expr, result, addressof_operand);
                case TEK_CONDITIONAL:
                        return tree_eval_conditional(context, expr, result);
                case TEK_INTEGER_LITERAL:
                        return tree_eval_integer_literal(context, expr, result);
                case TEK_CHARACTER_LITERAL:
                        return tree_eval_character_literal(context, expr, result);
                case TEK_FLOATING_LITERAL:
                        return tree_eval_floating_literal(context, expr, result);
                case TEK_DECL:
                        return tree_eval_decl(context,
                                tree_get_decl_expr_entity(expr), result, addressof_operand);
                case TEK_MEMBER:
                        return tree_eval_member(context, expr, result, addressof_operand);
                case TEK_CAST:
                        return tree_eval_cast(context, expr, result, addressof_operand);
                case TEK_SIZEOF:
                        return tree_eval_sizeof(context, expr, result);
                case TEK_PAREN:
                        return _tree_eval_expr(context,
                                tree_get_paren_expr(expr), result, addressof_operand);
                case TEK_IMPL_INIT:
                        return _tree_eval_expr(context,
                                tree_get_impl_init_expr(expr), result, addressof_operand);
                case TEK_STRING_LITERAL:
                        result->kind = TERK_ADDRESS_CONSTANT;
                        return true;
                case TEK_SUBSCRIPT:
                        return tree_eval_subscript(context, expr, result, addressof_operand);

                default:
                        assert(0 && "Invalid expression");
                case TEK_UNKNOWN:
                case TEK_CALL:
                case TEK_DESIGNATION:
                case TEK_INIT_LIST:
                        return false;
        }
}

extern bool tree_eval_expr(
        tree_context* context, const tree_expr* expr, tree_eval_result* result)
{
        return _tree_eval_expr(context, expr, result, false);
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
