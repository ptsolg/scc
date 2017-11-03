#include "c-parse-stmt.h"
#include "c-parse-exp.h"
#include "c-prog-stmt.h"
#include "c-parse-decl.h"

static tree_stmt* _cparse_stmt(cparser* self, cstmt_context context)
{
        switch (ctoken_get_kind(cparser_get_token(self)))
        {
                case CTK_CASE: return cparse_case_stmt(self, context);
                case CTK_DEFAULT: return cparse_default_stmt(self, context);
                case CTK_LBRACE: return cparse_block_stmt(self, context);
                case CTK_IF: return cparse_if_stmt(self, context);
                case CTK_SWITCH: return cparse_switch_stmt(self, context);
                case CTK_WHILE: return cparse_while_stmt(self, context);
                case CTK_DO: return cparse_do_while_stmt(self, context);
                case CTK_FOR: return cparse_for_stmt(self, context);
                case CTK_GOTO: return cparse_goto_stmt(self);
                case CTK_CONTINUE: return cparse_continue_stmt(self);
                case CTK_BREAK: return cparse_break_stmt(self);
                case CTK_RETURN: return cparse_return_stmt(self);

                default:
                        if (cparser_next_token_is(self, CTK_COLON))
                                return cparse_labeled_stmt(self, context);

                        return cparser_at_declaration(self)
                                ? cparse_decl_stmt(self)
                                : cparse_expr_stmt(self);
        }
}

extern tree_stmt* cparse_stmt(cparser* self)
{
        return _cparse_stmt(self, CSC_NONE);
}

extern tree_stmt* cparse_case_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_CASE))
                return NULL;

        tree_expr* value = cparse_const_expr(self);
        if (!value)
                return NULL;

        tree_location colon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_COLON))
                return NULL;

        tree_stmt* body = _cparse_stmt(self, context);
        if (!body)
                return NULL;

        return cprog_build_case_stmt(self->prog, kw_loc, colon_loc, value, body);
}

extern tree_stmt* cparse_default_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_DEFAULT))
                return NULL;

        tree_location colon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_COLON))
                return NULL;

        tree_stmt* body = _cparse_stmt(self, context);
        if (!body)
                return NULL;

        return cprog_build_default_stmt(self->prog, kw_loc, colon_loc, body);
}

extern tree_stmt* cparse_labeled_stmt(cparser* self, cstmt_context context)
{
        tree_location id_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_ID))
                return NULL;

        tree_id name = ctoken_get_string(cparser_get_prev(self));
        tree_location colon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_COLON))
                return NULL;

        tree_stmt* target = _cparse_stmt(self, context);
        if (!target)
                return NULL;

        return cprog_build_labeled_stmt(self->prog, id_loc, colon_loc, name, target);
}

extern tree_stmt* cparse_block_stmt(cparser* self, cstmt_context context)
{
        tree_location lbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_LBRACE))
                return NULL;

        tree_stmt* block = cprog_build_block_stmt(self->prog, lbrace_loc, context);
        if (!block)
                return NULL;

        cprog_enter_scope(self->prog, tree_get_compound_scope(block));
        while (!cparser_at(self, CTK_RBRACE))
        {
                tree_stmt* s = _cparse_stmt(self, context);
                if (!s || !cprog_finish_stmt(self->prog, s))
                {
                        cprog_exit_scope(self->prog);
                        return NULL;
                }
        }
        cprog_exit_scope(self->prog);

        return cparser_require(self, CTK_RBRACE)
                ? block
                : NULL;
}

extern tree_stmt* cparse_decl_stmt(cparser* self)
{
        tree_location begin_loc = cparser_get_loc(self);
        tree_decl* d = cparse_decl(self);
        if (!d)
                return NULL;

        tree_location semicolon_loc = ctoken_get_loc(cparser_get_prev(self));
        return cprog_build_decl_stmt(self->prog, begin_loc, semicolon_loc, d);
}

extern tree_stmt* cparse_expr_stmt(cparser* self)
{
        tree_expr* e = NULL;
        tree_location begin_loc = cparser_get_loc(self);
        if (!cparser_at(self, CTK_SEMICOLON))
                if (!(e = cparse_expr(self)))
                        return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return cprog_build_expr_stmt(self->prog, begin_loc, semicolon_loc, e);
}

extern tree_stmt* cparse_if_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_IF))
                return NULL;

        tree_expr* condition = cparse_paren_expr(self);
        if (!condition)
                return NULL;
        tree_location rbracket_loc = ctoken_get_loc(cparser_get_prev(self));

        tree_stmt* body = _cparse_stmt(self, context);
        if (!body)
                return NULL;

        tree_stmt* else_ = NULL;
        if (cparser_at(self, CTK_ELSE))
        {
                cparser_consume_token(self);
                if (!(else_ = _cparse_stmt(self, context)))
                        return NULL;
        }

        return cprog_build_if_stmt(self->prog, kw_loc, rbracket_loc, condition, body, else_);
}

extern tree_stmt* cparse_switch_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SWITCH))
                return NULL;

        tree_expr* value = cparse_paren_expr(self);
        if (!value)
                return NULL;
        tree_location rbracket_loc = ctoken_get_loc(cparser_get_prev(self));

        tree_stmt* body = _cparse_stmt(self, context | CSC_SWITCH);
        if (!body)
                return NULL;

        return cprog_build_switch_stmt(self->prog, kw_loc, rbracket_loc, value, body);
}

extern tree_stmt* cparse_while_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_WHILE))
                return NULL;

        tree_expr* condition = cparse_paren_expr(self);
        if (!condition)
                return NULL;
        tree_location rbracket_loc = ctoken_get_loc(cparser_get_prev(self));

        tree_stmt* body = _cparse_stmt(self, context | CSC_ITERATION);
        if (!body)
                return NULL;

        return cprog_build_while_stmt(self->prog, kw_loc, rbracket_loc, condition, body);
}

extern tree_stmt* cparse_do_while_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_DO))
                return NULL;

        tree_stmt* body = _cparse_stmt(self, context | CSC_ITERATION);
        if (!body || !cparser_require(self, CTK_WHILE))
                return NULL;

        tree_expr* condition = cparse_paren_expr(self);
        if (!condition)
                return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return cprog_build_do_while_stmt(self->prog, kw_loc, semicolon_loc, condition, body);
}

extern tree_stmt* cparse_for_stmt(cparser* self, cstmt_context context)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_FOR))
                return NULL;
        if (!cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_stmt* init = NULL;
        tree_expr* condition = NULL;
        tree_expr* step = NULL;
        tree_stmt* body = NULL;

        cprog_push_scope(self->prog);
        if (cparser_at_declaration(self))
        {
                if (!(init = cparse_decl_stmt(self)))
                        return NULL;
        }
        else
                if (!(init = cparse_expr_stmt(self)))
                        return NULL;

        if (!cparser_at(self, CTK_SEMICOLON))
                if (!(condition = cparse_expr(self)))
                        return NULL;
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        if (!cparser_at(self, CTK_RBRACKET))
                if (!(step = cparse_expr(self)))
                        return NULL;
        tree_location rbracket_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACKET))
                return NULL;

        if (!(body = _cparse_stmt(self, context | CSC_ITERATION)))
                return NULL;

        cprog_exit_decl_scope(self->prog);
        return cprog_build_for_stmt(self->prog, kw_loc, rbracket_loc, init, condition, step, body);
}

extern tree_stmt* cparse_goto_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_GOTO) || !cparser_require(self, CTK_ID))
                return NULL;

        tree_id label = ctoken_get_string(cparser_get_prev(self));
        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return cprog_build_goto_stmt(self->prog, kw_loc, semicolon_loc, label);
}

extern tree_stmt* cparse_continue_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_CONTINUE))
                return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return cprog_build_continue_stmt(self->prog, kw_loc, semicolon_loc);
}

extern tree_stmt* cparse_break_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_BREAK))
                return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return cprog_build_break_stmt(self->prog, kw_loc, semicolon_loc);
}

extern tree_stmt* cparse_return_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RETURN))
                return NULL;

        tree_expr* value = NULL;
        if (!cparser_at(self, CTK_SEMICOLON))
                if (!(value = cparse_expr(self)))
                        return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return cprog_build_return_stmt(self->prog, kw_loc, semicolon_loc, value);
}
