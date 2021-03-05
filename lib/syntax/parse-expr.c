#include "scc/syntax/parser.h"
#include "scc/semantics/sema.h"
#include "errors.h"

const c_token_kind ctk_rbracket_or_comma[] =
{
        CTK_RBRACKET,
        CTK_COMMA,
        CTK_UNKNOWN
};

static tree_expr* _c_parse_expr(c_parser* self);

extern tree_expr* c_parse_paren_expr(c_parser* self)
{
        tree_location lbracket_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* e = _c_parse_expr(self);
        tree_location rbracket_loc = c_parser_get_loc(self);
        return e && c_parser_require(self, CTK_RBRACKET)
                ? c_sema_new_paren_expr(self->sema, lbracket_loc, e, rbracket_loc)
                : NULL;
}

extern tree_expr* c_parse_primary_expr(c_parser* self)
{
        c_token* t = c_parser_get_token(self);
        c_token_kind k = c_token_get_kind(t);
        if (k == CTK_LBRACKET)
                return c_parse_paren_expr(self);

        tree_location loc = c_parser_get_loc(self);
        c_parser_consume_token(self);
        if (k == CTK_ID || k == CTK_CONST_STRING)
        {
                tree_id id = c_token_get_string(t);
                return k == CTK_ID
                        ? c_sema_new_decl_expr(self->sema, id, loc)
                        : c_sema_new_string_literal(self->sema, loc, id);
        }
        else if (k == CTK_CONST_CHAR)
                return c_sema_new_character_literal(self->sema, loc, c_token_get_char(t));
        else if (k == CTK_CONST_DOUBLE)
                return c_sema_new_dp_floating_literal(self->sema, loc, c_token_get_double(t));
        else if (k == CTK_CONST_FLOAT)
                return c_sema_new_sp_floating_literal(self->sema, loc, c_token_get_float(t));
        else if (k == CTK_CONST_INT)
        {
                bool signed_ = c_token_int_is_signed(t);
                bool ext = c_token_get_int_ls(t) == 2;
                return c_sema_new_integer_literal(self->sema,
                        loc, c_token_get_int(t), signed_, ext);
        }
       
        c_error_expected_expr(self->context, loc);
        return NULL;
}

static bool c_parse_argument_expr_list_opt(c_parser* self, tree_expr* call)
{
        if (c_parser_at(self, CTK_RBRACKET))
                return true;

        tree_expr* arg;
        while ((arg = c_parse_assignment_expr(self)))
        {
                c_sema_add_call_expr_arg(self->sema, call, arg);

                if (c_parser_at(self, CTK_RBRACKET))
                        return true;
                else if (!c_parser_require_ex(self, CTK_COMMA, ctk_rbracket_or_comma))
                        return false;
        }
        return false;
}

static inline tree_id c_parse_identifier(c_parser* self)
{
        return c_parser_require(self, CTK_ID)
                ? c_token_get_string(c_parser_get_prev(self))
                : TREE_INVALID_ID;
}

static tree_expr* c_parse_postfix_expr_suffix(c_parser* self, tree_expr* lhs)
{
        c_token_kind k = c_token_get_kind(c_parser_get_token(self));
        tree_location loc = c_parser_get_loc(self);

        if (k == CTK_LSBRACKET)
        {
                c_parser_consume_token(self);
                tree_expr* rhs = _c_parse_expr(self);
                return rhs && c_parser_require(self, CTK_RSBRACKET)
                        ? c_sema_new_subscript_expr(self->sema, loc, lhs, rhs)
                        : NULL;
        }
        else if (k == CTK_LBRACKET)
        {
                c_parser_consume_token(self);
                tree_expr* call = c_sema_new_call_expr(self->sema, loc, lhs);
                if (!call)
                        return NULL;
                if (!c_parse_argument_expr_list_opt(self, call))
                        return NULL;
                if (!c_parser_require(self, CTK_RBRACKET))
                        return NULL;
                return c_sema_check_call_expr_args(self->sema, call);
        }
        else if (k == CTK_DOT || k == CTK_ARROW)
        {
                c_parser_consume_token(self);
                tree_location id_loc = c_parser_get_loc(self);
                tree_id id = c_parse_identifier(self);
                if (id == TREE_INVALID_ID)
                        return NULL;

                return c_sema_new_member_expr(self->sema,
                        loc, lhs, id, id_loc, k == CTK_ARROW);
        }
        else if (k == CTK_PLUS2)
        {
                c_parser_consume_token(self);
                return c_sema_new_unary_expr(self->sema, loc, TUK_POST_INC, lhs);
        }
        else if (k == CTK_MINUS2)
        {
                c_parser_consume_token(self);
                return c_sema_new_unary_expr(self->sema, loc, TUK_POST_DEC, lhs);
        }

        return lhs;
}

extern tree_expr* c_parse_postfix_expr(c_parser* self)
{
        tree_expr* lhs = c_parse_primary_expr(self);
        while (1)
        {
                tree_expr* rhs = c_parse_postfix_expr_suffix(self, lhs);
                if (!rhs || rhs == lhs)
                        return rhs;
                lhs = rhs;
        }
}

static tree_expr* c_parse_sizeof_expr(c_parser* self)
{
        void* operand;
        bool contains_type = false;
        tree_location loc = c_parser_get_loc(self);

        if (c_parser_at(self, CTK_LBRACKET)
                && c_parser_token_starts_type_name(self, c_parser_get_next(self)))
        {
                c_parser_consume_token(self);
                loc = c_parser_get_loc(self);
                operand = c_parse_type_name(self);
                assert(operand);
                contains_type = true;
                if (!c_parser_require(self, CTK_RBRACKET))
                        return NULL;
        }
        else if (!(operand = c_parse_unary_expr(self)))
                return NULL;

        return c_sema_new_sizeof_expr(self->sema, loc, operand, contains_type);
}

static tree_unop_kind c_token_to_prefix_unary_operator(const c_token* self)
{
        switch (c_token_get_kind(self))
        {
        case CTK_AMP: return TUK_ADDRESS;
        case CTK_STAR: return TUK_DEREFERENCE;
        case CTK_PLUS: return TUK_PLUS;
        case CTK_MINUS: return TUK_MINUS;
        case CTK_TILDE: return TUK_NOT;
        case CTK_EXCLAIM: return TUK_LOG_NOT;
        case CTK_PLUS2: return TUK_PRE_INC;
        case CTK_MINUS2: return TUK_PRE_DEC;
        default: return TUK_UNKNOWN;
        }
}

extern tree_expr* c_parse_unary_expr(c_parser* self)
{
        tree_location loc = c_parser_get_loc(self);
        if (c_parser_at(self, CTK_SIZEOF))
        {
                c_parser_consume_token(self);
                return c_parse_sizeof_expr(self);
        }

        tree_unop_kind op = c_token_to_prefix_unary_operator(c_parser_get_token(self));
        if (op != TUK_UNKNOWN)
        {
                c_parser_consume_token(self);
                tree_expr* rhs = op == TUK_PRE_INC || op == TUK_PRE_DEC
                        ? c_parse_unary_expr(self)
                        : c_parse_cast_expr(self);

                return c_sema_new_unary_expr(self->sema, loc, op, rhs);
        }

        return c_parse_postfix_expr(self);
}

extern tree_expr* c_parse_cast_expr(c_parser* self)
{
        if (c_parser_at(self, CTK_LBRACKET) && c_parser_next_token_starts_type_name(self))
        {
                tree_location loc = c_parser_get_loc(self);
                tree_type* t = c_parse_paren_type_name(self);
                if (!t)
                        return NULL;

                tree_expr* rhs = c_parse_cast_expr(self);
                if (!rhs)
                        return NULL;

                return c_sema_new_cast_expr(self->sema, loc, t, rhs);
        }
        return c_parse_unary_expr(self);
}

static inline int get_operator_precedence(const c_token* t)
{
        switch (c_token_get_kind(t))
        {
                case CTK_STAR:
                case CTK_SLASH:
                case CTK_PERCENT:
                        return PRECEDENCE_MULTIPLICATIVE;
                case CTK_PLUS:
                case CTK_MINUS:
                        return PRECEDENCE_ADDITIVE;
                case CTK_LE2:
                case CTK_GR2:
                        return PRECEDENCE_SHIFT;
                case CTK_LE:
                case CTK_GR:
                case CTK_LEQ:
                case CTK_GREQ:
                        return PRECEDENCE_RELATIONAL;
                case CTK_EQ2:
                case CTK_EXCLAIM_EQ:
                        return PRECEDENCE_EQUALITY;
                case CTK_AMP:
                        return PRECEDENCE_AND;
                case CTK_CARET:
                        return PRECEDENCE_XOR;
                case CTK_VBAR:
                        return PRECEDENCE_OR;
                case CTK_AMP2:
                        return PRECEDENCE_LOG_AND;
                case CTK_VBAR2:
                        return PRECEDENCE_LOG_OR;
                case CTK_QUESTION:
                        return PRECEDENCE_CONDITIONAL;
                case CTK_EQ:
                case CTK_PLUS_EQ:
                case CTK_MINUS_EQ:
                case CTK_STAR_EQ:
                case CTK_SLASH_EQ:
                case CTK_PERCENT_EQ:
                case CTK_LE2_EQ:
                case CTK_GR2_EQ:
                case CTK_AMP_EQ:
                case CTK_CARET_EQ:
                case CTK_VBAR_EQ:
                        return PRECEDENCE_ASSIGN;
                case CTK_COMMA:
                        return PRECEDENCE_COMMA;
                default:
                        return PRECEDENCE_UNKNOWN;
        }
}

static tree_binop_kind c_token_to_binop(const c_token* self)
{
        switch (c_token_get_kind(self))
        {
                case CTK_STAR: return TBK_MUL;
                case CTK_SLASH: return TBK_DIV;
                case CTK_PERCENT: return TBK_MOD;
                case CTK_PLUS: return TBK_ADD;
                case CTK_MINUS: return TBK_SUB;
                case CTK_LE2: return TBK_SHL;
                case CTK_GR2: return TBK_SHR;
                case CTK_LE: return TBK_LE;
                case CTK_GR: return TBK_GR;
                case CTK_LEQ: return TBK_LEQ;
                case CTK_GREQ: return TBK_GEQ;
                case CTK_EQ2: return TBK_EQ;
                case CTK_EXCLAIM_EQ: return TBK_NEQ;
                case CTK_AMP: return TBK_AND;
                case CTK_CARET: return TBK_XOR;
                case CTK_VBAR: return TBK_OR;
                case CTK_AMP2: return TBK_LOG_AND;
                case CTK_VBAR2: return TBK_LOG_OR;
                case CTK_EQ: return TBK_ASSIGN;
                case CTK_PLUS_EQ: return TBK_ADD_ASSIGN;
                case CTK_MINUS_EQ: return TBK_SUB_ASSIGN;
                case CTK_STAR_EQ: return TBK_MUL_ASSIGN;
                case CTK_SLASH_EQ: return TBK_DIV_ASSIGN;
                case CTK_PERCENT_EQ: return TBK_MOD_ASSIGN;
                case CTK_LE2_EQ: return TBK_SHL_ASSIGN;
                case CTK_GR2_EQ: return TBK_SHR_ASSIGN;
                case CTK_AMP_EQ: return TBK_AND_ASSIGN;
                case CTK_CARET_EQ: return TBK_XOR_ASSIGN;
                case CTK_VBAR_EQ: return TBK_OR_ASSIGN;
                case CTK_COMMA: return TBK_COMMA;
                default: return TBK_UNKNOWN;
        }
}

static inline tree_expr* c_parse_rhs_of_binary_expr(c_parser* self, tree_expr* lhs, int min_prec)
{
        while (1)
        {
                int this_prec = get_operator_precedence(c_parser_get_token(self));
                if (this_prec < min_prec)
                        return lhs;

                const c_token* optoken = c_parser_get_token(self);
                tree_location oploc = c_token_get_loc(optoken);
                c_parser_consume_token(self);
                
                tree_expr* ternary_middle = NULL;
                if (c_token_is(optoken, CTK_QUESTION))
                {
                        ternary_middle = _c_parse_expr(self);
                        if (!ternary_middle || !c_parser_require(self, CTK_COLON))
                                return NULL;
                }

                tree_expr* rhs = c_parse_cast_expr(self);
                if (!rhs)
                        return NULL;

                int next_prec = get_operator_precedence(c_parser_get_token(self));
                bool next_right_assoc = next_prec == PRECEDENCE_ASSIGN || next_prec == PRECEDENCE_CONDITIONAL;
                if ((next_prec > this_prec) || (next_right_assoc && next_prec == this_prec))
                        rhs = c_parse_rhs_of_binary_expr(self, rhs, this_prec + !next_right_assoc);

                lhs = c_token_is(optoken, CTK_QUESTION)
                        ? c_sema_new_conditional_expr(self->sema, oploc, lhs, ternary_middle, rhs)
                        : c_sema_new_binary_expr(self->sema, oploc, c_token_to_binop(optoken), lhs, rhs);
                if (!lhs)
                        return NULL;
        }
}

extern tree_expr* c_parse_assignment_expr(c_parser* self)
{
        return c_parse_expr_ex(self, PRECEDENCE_ASSIGN);
}

static tree_expr* _c_parse_expr_ex(c_parser* self, int min_prec, bool finish)
{
        tree_expr* lhs = c_parse_cast_expr(self);
        if (!lhs)
                return NULL;

        tree_expr* expr = c_parse_rhs_of_binary_expr(self, lhs, min_prec);
        if (!expr)
                return NULL;

        return finish ? c_sema_finish_expr(self->sema, expr) : expr;
}

extern tree_expr* c_parse_expr_ex(c_parser* self, int min_prec)
{
        return _c_parse_expr_ex(self, min_prec, true);
}

static tree_expr* _c_parse_expr(c_parser* self)
{
        return _c_parse_expr_ex(self, PRECEDENCE_COMMA, false);
}

extern tree_expr* c_parse_expr(c_parser* self)
{
        return c_parse_expr_ex(self, PRECEDENCE_COMMA);
}

extern tree_expr* c_parse_const_expr(c_parser* self)
{
        return c_parse_expr_ex(self, PRECEDENCE_CONDITIONAL);
}

static tree_expr* c_parse_designation(c_parser* self)
{
        tree_expr* designation = c_sema_new_designation(self->sema, c_parser_get_loc(self));
        while (1)
        {
                tree_designator* designator = NULL;
                c_token_kind k = c_token_get_kind(c_parser_get_token(self));
                tree_id loc = c_parser_get_loc(self);

                if (k == CTK_DOT)
                {
                        c_parser_consume_token(self);
                        loc = c_parser_get_loc(self);
                        tree_id field = c_parse_identifier(self);
                        if (field == TREE_INVALID_ID)
                                return NULL;

                        designator = c_sema_new_field_designator(self->sema, loc, field);

                }
                else if (k == CTK_LSBRACKET)
                {
                        c_parser_consume_token(self);
                        tree_expr* index = c_parse_const_expr(self);
                        if (!index || !c_parser_require(self, CTK_RSBRACKET))
                                return NULL;

                        designator = c_sema_new_array_designator(self->sema, loc, index);
                }
                else if (c_parser_require(self, CTK_EQ))
                        return designation;
                else
                        return NULL;

                if (!designator)
                        return NULL;
                if (!c_sema_add_designation_designator(self->sema, designation, designator))
                        return NULL;
        }
}

static tree_expr* c_parse_initializer_list(c_parser* self, tree_location lbrace_loc)
{
        if (c_parser_at(self, CTK_RBRACE))
        {
                c_error_empty_initializer(self->context, c_parser_get_loc(self));
                return NULL;
        }

        tree_expr* list = c_sema_new_initializer_list(self->sema, lbrace_loc);
        while (1)
        {
                tree_expr* designation = NULL;
                if (c_parser_at(self, CTK_DOT) || c_parser_at(self, CTK_LSBRACKET))
                        if (!(designation = c_parse_designation(self)))
                                return NULL;

                tree_expr* init = c_parse_initializer(self);
                if (!init)
                        return NULL;

                if (designation)
                {
                        if (!c_sema_set_designation_initializer(self->sema, designation, init))
                                return NULL;
                        if (!c_sema_add_initializer_list_expr(self->sema, list, designation))
                                return NULL;
                }
                else if (!c_sema_add_initializer_list_expr(self->sema, list, init))
                        return NULL;

                if (c_parser_at(self, CTK_COMMA))
                        c_parser_consume_token(self);
                if (c_parser_at(self, CTK_RBRACE))
                {
                        tree_set_init_list_has_trailing_comma(
                                list, c_token_is(c_parser_get_prev(self), CTK_COMMA));
                        return list;
                }
        }
}

extern tree_expr* c_parse_initializer(c_parser* self)
{
        tree_expr* init;
        tree_location loc = c_parser_get_loc(self);

        if (!c_parser_at(self, CTK_LBRACE))
        {
                if (!(init = c_parse_assignment_expr(self)))
                        return NULL;
                return init;
        }

        c_parser_consume_token(self); // {
        if (!(init = c_parse_initializer_list(self, loc)))
                return NULL;

        return c_parser_require(self, CTK_RBRACE) ? init : NULL;
}
