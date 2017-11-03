#include "c-parse-expr.h"
#include "c-parse-decl.h"
#include "c-info.h"
#include "c-sema-expr.h"

const ctoken_kind ctk_rbracket_or_comma[] =
{
        CTK_RBRACKET,
        CTK_COMMA,
        CTK_UNKNOWN
};

extern tree_expr* cparse_paren_expr(cparser* self)
{
        tree_location lbracket_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* e = cparse_expr(self);
        return e && cparser_require(self, CTK_RBRACKET)
                ? csema_new_paren_expr(self->sema, lbracket_loc, e)
                : NULL;
}

extern tree_expr* cparse_primary_expr(cparser* self)
{
        ctoken* t = cparser_get_token(self);
        ctoken_kind k = ctoken_get_kind(t);
        if (k == CTK_LBRACKET)
                return cparse_paren_expr(self);

        tree_location loc = cparser_get_loc(self);
        cparser_consume_token(self);
        if (k == CTK_ID || k == CTK_CONST_STRING)
        {
                tree_id s = ctoken_get_string(t);
                return k == CTK_ID
                        ? csema_new_decl_expr(self->sema, loc, s)
                        : csema_new_string_literal(self->sema, loc, s);
        }
        else if (k == CTK_CONST_CHAR)
                return csema_new_character_literal(self->sema, loc, ctoken_get_char(t));
        else if (k == CTK_CONST_DOUBLE)
                return csema_new_dp_floating_literal(self->sema, loc, ctoken_get_double(t));
        else if (k == CTK_CONST_FLOAT)
                return csema_new_sp_floating_literal(self->sema, loc, ctoken_get_float(t));
        else if (k == CTK_CONST_INT)
        {
                bool signed_ = ctoken_is_int_signed(t);
                bool ext = ctoken_get_int_ls(t) == 2;
                return csema_new_integer_literal(self->sema,
                        loc, ctoken_get_int(t), signed_, ext);
        }
       
        cerror(self->error_manager, CES_ERROR, loc, "expected an expression");
        return NULL;
}

static bool cparse_argument_expr_list_opt(cparser* self, objgroup* args)
{
        if (cparser_at(self, CTK_RBRACKET))
                return true;

        tree_expr* e;
        while ((e = cparse_assignment_expr(self)))
        {
                objgroup_push_back(args, e);

                if (cparser_at(self, CTK_RBRACKET))
                        return true;
                else if (!cparser_require_ex(self, CTK_COMMA, ctk_rbracket_or_comma))
                        return false;
        }
        return false;
}

static inline tree_id cparse_identifier(cparser* self)
{
        return cparser_require(self, CTK_ID)
                ? ctoken_get_string(cparser_get_prev(self))
                : TREE_INVALID_ID;
}

static tree_expr* cparse_rhs_of_postfix_expr(cparser* self, tree_expr* lhs)
{
        ctoken_kind k = ctoken_get_kind(cparser_get_token(self));
        tree_location loc = cparser_get_loc(self);

        if (k == CTK_LSBRACKET)
        {
                cparser_consume_token(self);
                tree_expr* rhs = cparse_expr(self);
                return rhs && cparser_require(self, CTK_RSBRACKET)
                        ? csema_new_subscript_expr(self->sema, loc, lhs, rhs)
                        : NULL;
        }
        else if (k == CTK_LBRACKET)
        {
                cparser_consume_token(self);
                objgroup args;
                csema_init_objgroup(self->sema, &args);
                if (!cparse_argument_expr_list_opt(self, &args))
                        return NULL;

                tree_expr* call = csema_new_call_expr(self->sema, loc, lhs, &args);
                if (!call)
                        return NULL;

                return cparser_require(self, CTK_RBRACKET) ? call : NULL;
        }
        else if (k == CTK_DOT || k == CTK_ARROW)
        {
                cparser_consume_token(self);
                tree_location id_loc = cparser_get_loc(self);
                tree_id id = cparse_identifier(self);
                if (id == TREE_INVALID_ID)
                        return NULL;

                return csema_new_member_expr(self->sema,
                        loc, lhs, id, id_loc, k == CTK_ARROW);
        }
        else if (k == CTK_PLUS2)
        {
                cparser_consume_token(self);
                return csema_new_unary_expr(self->sema, loc, TUK_POST_INC, lhs);
        }
        else if (k == CTK_MINUS2)
        {
                cparser_consume_token(self);
                return csema_new_unary_expr(self->sema, loc, TUK_POST_DEC, lhs);
        }

        return lhs;
}

extern tree_expr* cparse_postfix_expr(cparser* self)
{
        tree_expr* lhs = cparse_primary_expr(self);
        while (1)
        {
                tree_expr* rhs = cparse_rhs_of_postfix_expr(self, lhs);
                if (!rhs || rhs == lhs)
                        return rhs;
                lhs = rhs;
        }
}

extern tree_expr* cparse_unary_expr(cparser* self)
{
        tree_location loc = cparser_get_loc(self);
        if (cparser_at(self, CTK_SIZEOF))
        {
                cparser_consume_token(self);
                csizeof_rhs rhs;

                if (cparser_at(self, CTK_LBRACKET))
                {
                        cparser_consume_token(self);
                        tree_location type_loc = cparser_get_loc(self);
                        tree_type* t = cparse_type_name(self);
                        if (!t || !cparser_require(self, CTK_RBRACKET))
                                return NULL;

                        csizeof_type_init(&rhs, t, type_loc);
                }
                else
                {
                        tree_expr* u = cparse_unary_expr(self);
                        if (!u)
                                return NULL;

                        csizeof_expr_init(&rhs, u);
                }

                return csema_new_sizeof_expr(self->sema, loc, &rhs);
        }

        tree_unop_kind op = ctoken_to_prefix_unary_operator(cparser_get_token(self));
        if (op != TUK_UNKNOWN)
        {
                cparser_consume_token(self);
                tree_expr* rhs = op == TUK_PRE_INC || op == TUK_PRE_DEC
                        ? cparse_unary_expr(self)
                        : cparse_cast_expr(self);

                return csema_new_unary_expr(self->sema, loc, op, rhs);
        }

        return cparse_postfix_expr(self);
}

extern tree_expr* cparse_cast_expr(cparser* self)
{
        if (cparser_at(self, CTK_LBRACKET) && cparser_next_token_starts_type_name(self))
        {
                tree_location loc = cparser_get_loc(self);
                tree_type* t = cparse_paren_type_name(self);
                if (!t)
                        return NULL;

                tree_expr* rhs = cparse_cast_expr(self);
                if (!rhs)
                        return NULL;

                return csema_new_cast_expr(self->sema, loc, t, rhs);
        }
        return cparse_unary_expr(self);
}

static inline tree_expr* cparse_rhs_of_binary_expr(cparser* self, tree_expr* lhs, int min_prec)
{
        while (1)
        {
                int this_prec = cget_operator_precedence(cparser_get_token(self));
                if (this_prec < min_prec)
                        return lhs;

                const ctoken* optoken = cparser_get_token(self);
                tree_location oploc = ctoken_get_loc(optoken);
                cparser_consume_token(self);
                
                tree_expr* ternary_middle = NULL;
                if (ctoken_is(optoken, CTK_QUESTION))
                {
                        ternary_middle = cparse_expr(self);
                        if (!ternary_middle || !cparser_require(self, CTK_COLON))
                                return NULL;
                }

                tree_expr* rhs = cparse_cast_expr(self);
                if (!rhs)
                        return NULL;

                int next_prec = cget_operator_precedence(cparser_get_token(self));
                bool next_right_assoc = this_prec == CPL_ASSIGN
                                     || this_prec == CPL_CONDITIONAL;

                if ((next_prec > this_prec) || (next_right_assoc && next_prec == this_prec))
                        rhs = cparse_rhs_of_binary_expr(self, rhs, next_prec);

                lhs = ctoken_is(optoken, CTK_QUESTION)
                        ? csema_new_conditional_expr(self->sema, oploc, lhs, ternary_middle, rhs)
                        : csema_new_binary_expr(self->sema, oploc, ctoken_to_binop(optoken), lhs, rhs);
                if (!lhs)
                        return NULL;
        }
}

extern tree_expr* cparse_assignment_expr(cparser* self)
{
        return cparse_expr_ex(self, CPL_ASSIGN);
}

extern tree_expr* cparse_expr(cparser* self)
{
        return cparse_expr_ex(self, CPL_COMMA);
}

extern tree_expr* cparse_expr_ex(cparser* self, int min_prec)
{
        tree_expr* lhs = cparse_cast_expr(self);
        return lhs
                ? cparse_rhs_of_binary_expr(self, lhs, min_prec)
                : NULL;
}

extern tree_expr* cparse_const_expr(cparser* self)
{
        return cparse_expr_ex(self, CPL_CONDITIONAL);
}

static tree_designation* cparse_designation(cparser* self, tree_type* t)
{
        tree_designation* designation = csema_new_designation(self->sema);
        while (1)
        {
                tree_designator* designator = NULL;
                ctoken_kind k = ctoken_get_kind(cparser_get_token(self));
                tree_id loc = cparser_get_loc(self);

                if (k == CTK_DOT)
                {
                        cparser_consume_token(self);
                        tree_id member = cparse_identifier(self);
                        if (member == TREE_INVALID_ID)
                                return NULL;

                        designator = csema_new_member_designator(self->sema, loc, t, member);

                }
                else if (k == CTK_LSBRACKET)
                {
                        cparser_consume_token(self);
                        tree_expr* index = cparse_const_expr(self);
                        if (!index || !cparser_require(self, CTK_RSBRACKET))
                                return NULL;

                        designator = csema_new_array_designator(self->sema, loc, t, index);
                }
                else if (cparser_require(self, CTK_EQ))
                        return designation;
                else
                        return NULL;

                if (!designator)
                        return NULL;
                if (!csema_finish_designator(self->sema, designation, designator))
                        return NULL;
                if (!(t = csema_get_designator_type(self->sema, designator)))
                        return NULL;
        }
}

static inline tree_expr* _cparse_initializer(cparser*, tree_type*);

static tree_expr* cparse_initializer_list(cparser* self, tree_type* t)
{
        if (cparser_at(self, CTK_RBRACE))
        {
                cerror(self->error_manager, CES_ERROR, cparser_get_loc(self),
                        "empty initializer list is invalid in C99");
                return NULL;
        }
      
        tree_expr* init = csema_new_init_expr(self->sema, cparser_get_loc(self));

        cinit_iterator it;
        cinit_iterator_init(&it, t);
        while (1)
        {
                tree_designation* d = NULL;
                if (cparser_at(self, CTK_DOT) || cparser_at(self, CTK_LSBRACKET))
                        if (!(d = cparse_designation(self, t)))
                                return NULL;

                tree_type* initialized_type;
                if (d)
                        initialized_type = csema_get_designation_type(self->sema, d);
                else
                {
                        if (!cinit_iterator_advance(&it))
                                return NULL; // invalid type

                        initialized_type = cinit_iterator_get_pos_type(&it);
                }

                tree_expr* designation_init = _cparse_initializer(self, initialized_type);
                if (!designation_init)
                        return NULL;

                if (!d) // build empty designation
                        d = csema_new_designation(self->sema);

                if (!csema_finish_designation(self->sema, init, d, designation_init))
                        return NULL;

                if (cparser_at(self, CTK_COMMA))
                        cparser_consume_token(self);
                if (cparser_at(self, CTK_RBRACE))
                {
                        if (ctoken_is(cparser_get_prev(self), CTK_COMMA))
                                if (!csema_add_empty_designation(self->sema, init))
                                        return NULL;
                        return init;
                }
        }
}

static inline tree_expr* _cparse_initializer(cparser* self, tree_type* t)
{
        if (!cparser_at(self, CTK_LBRACE))
                return cparse_assignment_expr(self);

        cparser_consume_token(self); // {
        tree_expr* init = cparse_initializer_list(self, t);
        if (!init)
                return NULL;

        return cparser_require(self, CTK_RBRACE)
                ? init
                : NULL;
}

extern tree_expr* cparse_initializer(cparser* self, tree_decl* decl)
{
        return _cparse_initializer(self, tree_desugar_type(tree_get_decl_type(decl)));
}