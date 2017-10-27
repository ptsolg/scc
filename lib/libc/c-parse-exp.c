#include "c-parse-exp.h"
#include "c-parse-decl.h"
#include "c-info.h"
#include "c-prog-exp.h"

const ctoken_kind ctk_rbracket_or_comma[] =
{
        CTK_RBRACKET,
        CTK_COMMA,
        CTK_UNKNOWN
};

extern tree_exp* cparse_paren_exp(cparser* self)
{
        if (!cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_exp* e = cparse_exp(self);
        return e && cparser_require(self, CTK_RBRACKET)
                ? cprog_build_paren_exp(self->prog, cparser_get_loc(self), e)
                : NULL;
}

extern tree_exp* cparse_primary_exp(cparser* self)
{
        ctoken*     t = cparser_get_token(self);
        ctoken_kind k = ctoken_get_kind(t);
        if (k == CTK_LBRACKET)
                return cparse_paren_exp(self);

        tree_location loc = cparser_get_loc(self);
        cparser_consume_token(self);
        if (k == CTK_ID || k == CTK_CONST_STRING)
        {
                tree_id s = ctoken_get_string(t);
                return k == CTK_ID
                        ? cprog_build_decl_exp(self->prog, loc, s)
                        : cprog_build_string_literal(self->prog, loc, s);
        }
        else if (k == CTK_CONST_CHAR)
                return cprog_build_character_literal(self->prog, loc, ctoken_get_char(t));
        else if (k == CTK_CONST_DOUBLE)
                return cprog_build_floating_lliteral(self->prog, loc, ctoken_get_double(t));
        else if (k == CTK_CONST_FLOAT)
                return cprog_build_floating_literal(self->prog, loc, ctoken_get_float(t));
        else if (k == CTK_CONST_INT)
        {
                bool signed_ = ctoken_is_int_signed(t);
                bool ext = ctoken_get_int_ls(t) == 2;
                return cprog_build_integer_literal(self->prog,
                        loc, ctoken_get_int(t), signed_, ext);
        }
       
        cerror(self->error_manager, CES_ERROR, loc, "expected an expression");
        return NULL;
}

static bool cparse_argument_exp_list_opt(cparser* self, objgroup* args)
{
        if (cparser_at(self, CTK_RBRACKET))
                return true;

        tree_exp* e;
        while ((e = cparse_assignment_exp(self)))
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

static tree_exp* cparse_rhs_of_postfix_exp(cparser* self, tree_exp* lhs)
{
        ctoken_kind   k   = ctoken_get_kind(cparser_get_token(self));
        tree_location loc = cparser_get_loc(self);

        if (k == CTK_LSBRACKET)
        {
                cparser_consume_token(self);
                tree_exp* rhs = cparse_exp(self);
                return rhs && cparser_require(self, CTK_RSBRACKET)
                        ? cprog_build_subscript_exp(self->prog, loc, lhs, rhs)
                        : NULL;
        }
        else if (k == CTK_LBRACKET)
        {
                cparser_consume_token(self);
                objgroup args;
                cprog_init_objgroup(self->prog, &args);
                if (!cparse_argument_exp_list_opt(self, &args))
                        return NULL;

                tree_exp* call = cprog_build_call_exp(self->prog, loc, lhs, &args);
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

                return cprog_build_member_exp(self->prog,
                        loc, lhs, id, id_loc, k == CTK_ARROW);
        }
        else if (k == CTK_PLUS2)
        {
                cparser_consume_token(self);
                return cprog_build_unop(self->prog, loc, TUK_POST_INC, lhs);
        }
        else if (k == CTK_MINUS2)
        {
                cparser_consume_token(self);
                return cprog_build_unop(self->prog, loc, TUK_POST_DEC, lhs);
        }

        return lhs;
}

extern tree_exp* cparse_postfix_exp(cparser* self)
{
        tree_exp* lhs = cparse_primary_exp(self);
        while (1)
        {
                tree_exp* rhs = cparse_rhs_of_postfix_exp(self, lhs);
                if (!rhs || rhs == lhs)
                        return rhs;
                lhs = rhs;
        }
}

extern tree_exp* cparse_unary_exp(cparser* self)
{
        tree_location loc = cparser_get_loc(self);
        if (cparser_at(self, CTK_SIZEOF))
        {
                cparser_consume_token(self);
                csizeof_rhs rhs;

                if (cparser_at(self, CTK_LBRACKET))
                {
                        tree_type* t = cparse_paren_type_name(self);
                        if (!t)
                                return NULL;
                        csizeof_type_init(&rhs, t, cparser_get_loc(self));
                }
                else
                {
                        tree_exp* u = cparse_unary_exp(self);
                        if (!u)
                                return NULL;

                        csizeof_exp_init(&rhs, u);
                }

                return cprog_build_sizeof(self->prog, loc, &rhs);
        }

        tree_unop_kind op = ctoken_to_prefix_unary_operator(cparser_get_token(self));
        if (op != TUK_UNKNOWN)
        {
                cparser_consume_token(self);
                tree_exp* rhs = op == TUK_PRE_INC || op == TUK_PRE_DEC
                        ? cparse_unary_exp(self)
                        : cparse_cast_exp(self);

                return cprog_build_unop(self->prog, loc, op, rhs);
        }

        return cparse_postfix_exp(self);
}

extern tree_exp* cparse_cast_exp(cparser* self)
{
        if (cparser_at(self, CTK_LBRACKET) && cparser_next_token_starts_type_name(self))
        {
                tree_location loc = cparser_get_loc(self);
                tree_type* t = cparse_paren_type_name(self);
                if (!t)
                        return NULL;

                tree_exp* rhs = cparse_cast_exp(self);
                if (!rhs)
                        return NULL;

                return cprog_build_cast(self->prog, loc, t, rhs);
        }
        return cparse_unary_exp(self);
}

static inline tree_exp* cparse_rhs_of_binary_exp(cparser* self, tree_exp* lhs, int min_prec)
{
        while (1)
        {
                int this_prec = cget_operator_precedence(cparser_get_token(self));
                if (this_prec < min_prec)
                        return lhs;

                const ctoken* optoken = cparser_get_token(self);
                tree_location oploc   = ctoken_get_loc(optoken);
                cparser_consume_token(self);
                
                tree_exp* ternary_middle = NULL;
                if (ctoken_is(optoken, CTK_QUESTION))
                {
                        ternary_middle = cparse_exp(self);
                        if (!ternary_middle || !cparser_require(self, CTK_COLON))
                                return NULL;
                }

                tree_exp* rhs = cparse_cast_exp(self);
                if (!rhs)
                        return NULL;

                int next_prec = cget_operator_precedence(cparser_get_token(self));
                bool next_right_assoc = this_prec == CPL_ASSIGN
                                     || this_prec == CPL_CONDITIONAL;

                if ((next_prec > this_prec) || (next_right_assoc && next_prec == this_prec))
                        rhs = cparse_rhs_of_binary_exp(self, rhs, next_prec);

                lhs = ctoken_is(optoken, CTK_QUESTION)
                        ? cprog_build_conditional(self->prog, oploc, lhs, ternary_middle, rhs)
                        : cprog_build_binop(self->prog, oploc, ctoken_to_binop(optoken), lhs, rhs);
                if (!lhs)
                        return NULL;
        }
}

extern tree_exp* cparse_assignment_exp(cparser* self)
{
        return cparse_exp_ex(self, CPL_ASSIGN);
}

extern tree_exp* cparse_exp(cparser* self)
{
        return cparse_exp_ex(self, CPL_COMMA);
}

extern tree_exp* cparse_exp_ex(cparser* self, int min_prec)
{
        tree_exp* lhs = cparse_cast_exp(self);
        return lhs
                ? cparse_rhs_of_binary_exp(self, lhs, min_prec)
                : NULL;
}

extern tree_const_exp* cparse_const_exp(cparser* self)
{
        tree_exp* root = cparse_exp_ex(self, CPL_CONDITIONAL);
        return root
                ? cprog_build_const_exp(self->prog, root)
                : NULL;
}

static tree_designation* cparse_designation(cparser* self, tree_type* t)
{
        tree_designation* designation = cprog_build_designation(self->prog);
        while (1)
        {
                tree_designator* designator = NULL;
                ctoken_kind k = ctoken_get_kind(cparser_get_token(self));
                tree_id loc   = cparser_get_loc(self);

                if (k == CTK_DOT)
                {
                        cparser_consume_token(self);
                        tree_id member = cparse_identifier(self);
                        if (member == TREE_INVALID_ID)
                                return NULL;

                        designator = cprog_build_member_designator(self->prog, loc, t, member);

                }
                else if (k == CTK_LSBRACKET)
                {
                        cparser_consume_token(self);
                        tree_const_exp* index = cparse_const_exp(self);
                        if (!index || !cparser_require(self, CTK_RSBRACKET))
                                return NULL;

                        designator = cprog_build_array_designator(self->prog, loc, t, index);
                }
                else if (cparser_require(self, CTK_EQ))
                        return designation;
                else
                        return NULL;

                if (!designator)
                        return NULL;
                if (!cprog_finish_designator(self->prog, designation, designator))
                        return NULL;
                if (!(t = cprog_get_designator_type(self->prog, designator)))
                        return NULL;
        }
}

static inline tree_exp* _cparse_initializer(cparser*, tree_type*);

static tree_exp* cparse_initializer_list(cparser* self, tree_type* t)
{
        if (cparser_at(self, CTK_RBRACE))
        {
                cerror(self->error_manager, CES_ERROR, cparser_get_loc(self),
                        "empty initializer list is invalid in C99");
                return NULL;
        }
      
        tree_exp* init = cprog_build_init_exp(self->prog, cparser_get_loc(self));

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
                        initialized_type = cprog_get_designation_type(self->prog, d);
                else
                {
                        if (!cinit_iterator_advance(&it))
                                return NULL; // invalid type

                        initialized_type = cinit_iterator_get_pos_type(&it);
                }

                tree_exp* designation_init = _cparse_initializer(self, initialized_type);
                if (!designation_init)
                        return NULL;

                if (!d) // build empty designation
                        d = cprog_build_designation(self->prog);

                if (!cprog_finish_designation(self->prog, init, d, designation_init))
                        return NULL;

                if (cparser_at(self, CTK_COMMA))
                        cparser_consume_token(self);
                if (cparser_at(self, CTK_RBRACE))
                {
                        if (ctoken_is(cparser_get_prev(self), CTK_COMMA))
                                if (!cprog_add_empty_designation(self->prog, init))
                                        return NULL;
                        return init;
                }
        }
}

static inline tree_exp* _cparse_initializer(cparser* self, tree_type* t)
{
        if (!cparser_at(self, CTK_LBRACE))
                return cparse_assignment_exp(self);

        cparser_consume_token(self); // {
        tree_exp* init = cparse_initializer_list(self, t);
        if (!init)
                return NULL;

        return cparser_require(self, CTK_RBRACE)
                ? init
                : NULL;
}

extern tree_exp* cparse_initializer(cparser* self, tree_decl* decl)
{
        return _cparse_initializer(self, tree_desugar_type(tree_get_decl_type(decl)));
}