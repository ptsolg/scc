#include "c-preprocessor-expression.h"
#include "scc/c/c-preprocessor.h"
#include "c-numeric-literal.h"
#include "scc/c/c-context.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-token.h"
#include "scc/c/c-info.h"
#include "scc/tree/tree-target.h"
#include "scc/core/value.h"

static void c_preprocessor_init_int(c_preprocessor* self, int_value* val, bool is_signed, uint64_t v)
{
        size_t intmax_t_bits = 8 * tree_get_intmax_t_size(self->context->tree->target);
        size_t uintmax_t_bits = 8 * tree_get_uintmax_t_size(self->context->tree->target);
        int_init(val, is_signed ? intmax_t_bits : uintmax_t_bits, is_signed, v);
}

static bool c_preprocessor_require(const c_preprocessor* self, c_token_kind k, const c_token* t)
{
        if (!c_token_is(t, k))
        {
                c_error_missing_token_in_expression(self->logger, k, c_token_get_loc(t));
                return false;
        }
        return true;
}

static c_token* _c_preprocessor_evaluate_expr(c_preprocessor*, c_token*, int_value*);

static bool c_preprocessor_evaluate_primary_expr(c_preprocessor* self, c_token* tok, int_value* result)
{
        c_token_kind k = c_token_get_kind(tok);
        tree_location loc = c_token_get_loc(tok);

        if (k == CTK_ID)
        {
                c_preprocessor_init_int(self, result, true, 0);
                return true;
        }
        else if (k == CTK_CONST_CHAR)
        {
                c_preprocessor_init_int(self, result, true, c_token_get_char(tok));
                return true;
        }
        else if (k == CTK_PP_NUM)
        {
                const char* num_string = tree_get_id_string(self->context->tree, c_token_get_string(tok));
                c_numeric_literal literal;
                c_parse_numeric_literal(num_string, loc, &literal, self->logger);
                switch (literal.kind)
                {
                        case CNLK_INTEGER:
                                c_preprocessor_init_int(self, result,
                                        literal.integer.is_signed, literal.integer.value);
                                return true;

                        case CNLK_SP_FLOATING:
                        case CNLK_DP_FLOATING:
                                c_error_floating_constant_in_preprocessor_expression(self->logger, loc);
                                return false;
                        default:
                                return false;
                }
        }
        else if (k == CTK_LBRACKET)
        {
                if (!(tok = c_preprocess_non_macro(self)))
                        return false;
                if (!(tok = _c_preprocessor_evaluate_expr(self, tok, result)))
                        return false;
                return c_preprocessor_require(self, CTK_RBRACKET, tok);
        }

        if (!c_token_is(tok, CTK_EOD))
                c_error_token_is_not_valid_in_preprocessor_expressions(self->logger, tok);
        else
                c_error_expected_expr(self->logger, c_token_get_loc(tok));
        return false;
}

static bool c_preprocessor_evaluate_defined(c_preprocessor* self, int_value* result)
{
        c_token* tok = c_preprocess_non_wspace(self);
        if (!tok)
                return false;

        bool rbracket_expected = false;
        if (c_token_is(tok, CTK_LBRACKET))
        {
                rbracket_expected = true;
                if (!(tok = c_preprocess_non_wspace(self)))
                        return false;
        }

        if (!c_preprocessor_require(self, CTK_ID, tok))
                return false;

        c_preprocessor_init_int(self, result, true,
                c_preprocessor_macro_defined(self, c_token_get_string(tok)) ? 1 : 0);

        if (rbracket_expected)
        {
                if (!(tok = c_preprocess_non_wspace(self)))
                        return false;
                if (!c_preprocessor_require(self, CTK_RBRACKET, tok))
                        return false;
        }
        return true;
}

static bool c_preprocessor_evaluate_unary_expr(c_preprocessor* self, c_token* tok, int_value* result)
{
        c_token_kind k = c_token_get_kind(tok);
        if (k == CTK_ID && c_token_get_string(tok) == self->defined_id)
                return c_preprocessor_evaluate_defined(self, result);
        if (k != CTK_MINUS && k != CTK_TILDE && k != CTK_EXCLAIM && k != CTK_PLUS)
                return c_preprocessor_evaluate_primary_expr(self, tok, result);

        if (!(tok = c_preprocess_non_macro(self)))
                return false;
        if (!c_preprocessor_evaluate_unary_expr(self, tok, result))
                return false;

        if (k == CTK_MINUS)
                int_neg(result);
        else if (k == CTK_TILDE)
                int_not(result);
        else if (k == CTK_EXCLAIM)
                c_preprocessor_init_int(self, result, true, int_is_zero(result) ? 1 : 0);
        else if (k == CTK_PLUS)
                ;
        else
                UNREACHABLE();

        return true;
}

static void c_pp_arithmetic_conversion(int_value* lhs, int_value* rhs)
{
        if (!int_is_signed(lhs))
                int_set_signed(rhs, false);
        else if (!int_is_signed(rhs))
                int_set_signed(lhs, false);
}

static bool c_preprocessor_evaluate_binary_expr(
        c_preprocessor* self, const c_token* op, int_value* lhs, int_value* rhs)
{
        tree_location oploc = c_token_get_loc(op);
        c_pp_arithmetic_conversion(lhs, rhs);
        cmp_result cr;
        switch (c_token_get_kind(op))
        {
                case CTK_STAR:
                        int_mul(lhs, rhs);
                        return true;
                case CTK_SLASH:
                        if (int_div(lhs, rhs) == OR_DIV_BY_ZERO)
                        {
                                c_error_division_by_zero_in_preprocessor_expression(
                                        self->logger, oploc);
                                return false;
                        }
                        return true;
                case CTK_PERCENT:
                        if (int_mod(lhs, rhs) == OR_DIV_BY_ZERO)
                        {
                                c_error_division_by_zero_in_preprocessor_expression(
                                        self->logger, oploc);
                                return false;
                        }
                        return true;
                case CTK_PLUS:
                        int_add(lhs, rhs);
                        return true;
                case CTK_MINUS:
                        int_sub(lhs, rhs);
                        return true;
                case CTK_LE2:
                        int_shl(lhs, rhs);
                        return true;
                case CTK_GR2:
                        int_shr(lhs, rhs);
                        return true;
                case CTK_LE:
                        c_preprocessor_init_int(self, lhs, true,
                                int_cmp(lhs, rhs) == CR_LE ? 1 : 0);
                        return true;
                case CTK_GR:
                        c_preprocessor_init_int(self, lhs, true,
                                int_cmp(lhs, rhs) == CR_GR ? 1 : 0);
                        return true;
                case CTK_LEQ:
                        cr = int_cmp(lhs, rhs);
                        c_preprocessor_init_int(self, lhs, true,
                                cr == CR_LE || cr == CR_EQ ? 1 : 0);
                        return true;
                case CTK_GREQ:
                        cr = int_cmp(lhs, rhs);
                        c_preprocessor_init_int(self, lhs, true,
                                cr == CR_GR || cr == CR_EQ ? 1 : 0);
                        return true;
                case CTK_EQ2:
                        c_preprocessor_init_int(self, lhs, true,
                                int_cmp(lhs, rhs) == CR_EQ ? 1 : 0);
                        return true;
                case CTK_EXCLAIM_EQ:
                        c_preprocessor_init_int(self, lhs, true,
                                int_cmp(lhs, rhs) != CR_EQ ? 1 : 0);
                        return true;
                case CTK_AMP:
                        int_and(lhs, rhs);
                        return true;
                case CTK_CARET:
                        int_xor(lhs, rhs);
                        return true;
                case CTK_VBAR:
                        int_or(lhs, rhs);
                        return true;
                case CTK_AMP2:
                        c_preprocessor_init_int(self, lhs, true,
                                !int_is_zero(lhs) && !int_is_zero(rhs) ? 1 : 0);
                        return true;
                case CTK_VBAR2:
                        c_preprocessor_init_int(self, lhs, true,
                                !int_is_zero(lhs) || !int_is_zero(rhs) ? 1 : 0);
                        return true;
                default:
                        c_error_token_is_not_valid_in_preprocessor_expressions(self->logger, op);
                        return false;
        }
}

static c_token* c_preprocessor_evaluate_rhs_of_binary_expr(
        c_preprocessor* self, c_token* tok, int_value* lhs, int min_prec)
{
        while (1)
        {
                int this_prec = c_get_operator_precedence(tok);
                if (this_prec < min_prec)
                        return tok;

                const c_token* optoken = tok;
                if (!(tok = c_preprocess_non_macro(self)))
                        return NULL;

                int_value ternary_middle;
                if (c_token_is(optoken, CTK_QUESTION))
                {
                        if (!(tok = _c_preprocessor_evaluate_expr(self, tok, &ternary_middle)))
                                return NULL;
                        if (!c_preprocessor_require(self, CTK_COLON, tok))
                                return NULL;
                        if (!(tok = c_preprocess_non_macro(self)))
                                return NULL;
                }

                int_value rhs;
                if (!c_preprocessor_evaluate_unary_expr(self, tok, &rhs))
                        return NULL;

                if (!(tok = c_preprocess_non_macro(self)))
                        return NULL;

                int next_prec = c_get_operator_precedence(tok);
                bool next_right_assoc = this_prec == CPL_ASSIGN
                                     || this_prec == CPL_CONDITIONAL;

                if ((next_prec > this_prec) || (next_right_assoc && next_prec == this_prec))
                        if (!c_preprocessor_evaluate_rhs_of_binary_expr(self, tok, &rhs, next_prec))
                                return NULL;

                if (c_token_is(optoken, CTK_QUESTION))
                {
                        c_pp_arithmetic_conversion(&rhs, &ternary_middle);
                        *lhs = int_is_zero(lhs) ? rhs : ternary_middle;
                }
                else if (!c_preprocessor_evaluate_binary_expr(self, optoken, lhs, &rhs))
                        return NULL;
        }
}

static c_token* _c_preprocessor_evaluate_expr(c_preprocessor* self, c_token* tok, int_value* result)
{
        if (!c_preprocessor_evaluate_unary_expr(self, tok, result))
                return NULL;

        if (!(tok = c_preprocess_non_macro(self)))
                return NULL;

        return c_preprocessor_evaluate_rhs_of_binary_expr(self, tok, result, CPL_COMMA);
}

extern c_token* c_preprocessor_evaluate_expr(c_preprocessor* self, int_value* result)
{
        c_token* t = c_preprocess_non_macro(self);
        if (!t)
                return NULL;

        return _c_preprocessor_evaluate_expr(self, t, result);
}