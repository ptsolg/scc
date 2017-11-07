#include "c-parse-stmt.h"
#include "c-parse-expr.h"
#include "c-sema-stmt.h"
#include "c-sema-decl.h"
#include "c-parse-decl.h"

static tree_stmt* _cparse_stmt_no_check(cparser* self, cstmt_context context)
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

static tree_stmt* _cparse_stmt(cparser* self, cstmt_context context)
{
        tree_stmt* s = _cparse_stmt_no_check(self, context);
        if (!s || !csema_check_stmt(self->sema, s, context))
                return NULL;

        return s;
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

        return csema_new_case_stmt(self->sema, kw_loc, colon_loc, value, body);
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

        return csema_new_default_stmt(self->sema, kw_loc, colon_loc, body);
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

        tree_decl* label = csema_def_label_decl(self->sema, id_loc, name, colon_loc);
        if (!label)
                return NULL;

        tree_stmt* stmt = _cparse_stmt(self, context);
        if (!stmt)
                return NULL;

        return csema_new_labeled_stmt(self->sema, label, stmt);
}

extern tree_stmt* cparse_block_stmt(cparser* self, cstmt_context context)
{
        tree_location lbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_LBRACE))
                return NULL;

        tree_stmt* block = csema_new_block_stmt(self->sema, lbrace_loc, context);
        if (!block)
                return NULL;

        csema_enter_scope(self->sema, tree_get_compound_scope(block));
        while (!cparser_at(self, CTK_RBRACE))
        {
                tree_stmt* s = _cparse_stmt(self, context);
                if (!s || !csema_add_stmt(self->sema, s))
                {
                        csema_exit_scope(self->sema);
                        return NULL;
                }
        }
        csema_exit_scope(self->sema);

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
        return csema_new_decl_stmt(self->sema, begin_loc, semicolon_loc, d);
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

        return csema_new_expr_stmt(self->sema, begin_loc, semicolon_loc, e);
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

        return csema_new_if_stmt(self->sema, kw_loc, rbracket_loc, condition, body, else_);
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

        return csema_new_switch_stmt(self->sema, kw_loc, rbracket_loc, value, body);
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

        return csema_new_while_stmt(self->sema, kw_loc, rbracket_loc, condition, body);
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

        return csema_new_do_while_stmt(self->sema, kw_loc, semicolon_loc, condition, body);
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

        csema_push_scope(self->sema);
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

        csema_exit_decl_scope(self->sema);
        return csema_new_for_stmt(self->sema, kw_loc, rbracket_loc, init, condition, step, body);
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

        return csema_new_goto_stmt(self->sema, kw_loc, semicolon_loc, label);
}

extern tree_stmt* cparse_continue_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_CONTINUE))
                return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return csema_new_continue_stmt(self->sema, kw_loc, semicolon_loc);
}

extern tree_stmt* cparse_break_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_BREAK))
                return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return csema_new_break_stmt(self->sema, kw_loc, semicolon_loc);
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

        return csema_new_return_stmt(self->sema, kw_loc, semicolon_loc, value);
}
