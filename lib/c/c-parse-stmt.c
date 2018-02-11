#include "scc/c/c-parse-stmt.h"
#include "scc/c/c-parse-expr.h"
#include "scc/c/c-sema-stmt.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-parse-decl.h"

static tree_stmt* _cparse_stmt_no_check(cparser* self, int scope_flags)
{
        switch (ctoken_get_kind(cparser_get_token(self)))
        {
                case CTK_CASE:     return cparse_case_stmt(self, scope_flags);
                case CTK_DEFAULT:  return cparse_default_stmt(self, scope_flags);
                case CTK_LBRACE:   return cparse_block_stmt(self, scope_flags);
                case CTK_IF:       return cparse_if_stmt(self, scope_flags);
                case CTK_SWITCH:   return cparse_switch_stmt(self, scope_flags);
                case CTK_WHILE:    return cparse_while_stmt(self, scope_flags);
                case CTK_DO:       return cparse_do_while_stmt(self, scope_flags);
                case CTK_FOR:      return cparse_for_stmt(self, scope_flags);
                case CTK_GOTO:     return cparse_goto_stmt(self);
                case CTK_CONTINUE: return cparse_continue_stmt(self);
                case CTK_BREAK:    return cparse_break_stmt(self);
                case CTK_RETURN:   return cparse_return_stmt(self);

                default:
                        if (cparser_next_token_is(self, CTK_COLON))
                                return cparse_labeled_stmt(self, scope_flags);

                        return cparser_at_declaration(self)
                                ? cparse_decl_stmt(self)
                                : cparse_expr_stmt(self);
        }
}

static tree_stmt* _cparse_stmt(cparser* self, int scope_flags)
{
        scope_flags &= ~TSF_DECL;

        tree_stmt* s = _cparse_stmt_no_check(self, scope_flags);
        if (!s || !csema_check_stmt(self->sema, s, scope_flags))
                return NULL;

        return s;
}

static tree_stmt* _cparse_block_item(cparser* self, int scope_flags)
{
        scope_flags |= TSF_DECL;

        tree_stmt* s = _cparse_stmt_no_check(self, scope_flags);
        if (!s || !csema_check_stmt(self->sema, s, scope_flags))
                return NULL;

        return s;
}

extern tree_stmt* cparse_stmt(cparser* self)
{
        return _cparse_stmt(self, TSF_NONE);
}

extern tree_stmt* cparse_case_stmt(cparser* self, int scope_flags)
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

        tree_stmt* case_stmt = csema_new_case_stmt(self->sema, kw_loc, colon_loc, value);
        if (!case_stmt)
                return NULL;

        tree_stmt* body = _cparse_stmt(self, scope_flags);
        if (!body)
                return NULL;

        csema_set_case_stmt_body(self->sema, case_stmt, body);
        return case_stmt;
}

extern tree_stmt* cparse_default_stmt(cparser* self, int scope_flags)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_DEFAULT))
                return NULL;

        tree_location colon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_COLON))
                return NULL;

        tree_stmt* default_stmt = csema_new_default_stmt(self->sema, kw_loc, colon_loc);
        if (!default_stmt)
                return NULL;

        tree_stmt* body = _cparse_stmt(self, scope_flags);
        if (!body)
                return NULL;

        csema_set_default_stmt_body(self->sema, default_stmt, body);
        return default_stmt;
}

extern tree_stmt* cparse_labeled_stmt(cparser* self, int scope_flags)
{
        tree_location id_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_ID))
                return NULL;

        tree_id name = ctoken_get_string(cparser_get_prev(self));
        tree_location colon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_COLON))
                return NULL;

        tree_stmt placeholder;
        tree_decl* label = csema_define_label_decl(
                self->sema, id_loc, name, colon_loc, &placeholder);
        if (!label)
                return NULL;

        tree_stmt* stmt = _cparse_stmt(self, scope_flags);
        if (!stmt)
                return NULL;

        return csema_new_labeled_stmt(self->sema, label, stmt);
}

extern tree_stmt* cparse_block_stmt(cparser* self, int scope_flags)
{
        tree_location lbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_LBRACE))
                return NULL;

        tree_stmt* block = csema_new_block_stmt(self->sema, lbrace_loc, scope_flags);
        if (!block)
                return NULL;

        csema_enter_scope(self->sema, tree_get_compound_scope(block));
        while (!cparser_at(self, CTK_RBRACE))
        {
                tree_stmt* s = _cparse_block_item(self, scope_flags);
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

extern tree_stmt* cparse_if_stmt(cparser* self, int scope_flags)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_IF) || !cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* condition = cparse_expr(self);
        if (!condition)
                return NULL;

        tree_location rbracket_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACKET))
                return NULL;

        tree_stmt* body = _cparse_stmt(self, scope_flags);
        if (!body)
                return NULL;

        tree_stmt* else_ = NULL;
        if (cparser_at(self, CTK_ELSE))
        {
                cparser_consume_token(self);
                if (!(else_ = _cparse_stmt(self, scope_flags)))
                        return NULL;
        }

        return csema_new_if_stmt(self->sema, kw_loc, rbracket_loc, condition, body, else_);
}

extern tree_stmt* cparse_switch_stmt(cparser* self, int scope_flags)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SWITCH) || !cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* value = cparse_expr(self);
        if (!value)
                return NULL;

        tree_location rbracket_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACKET))
                return NULL;

        tree_stmt* switch_ = csema_start_switch_stmt(self->sema, kw_loc, rbracket_loc, value);
        if (!switch_)
                return NULL;

        tree_stmt* body = _cparse_stmt(self, scope_flags | TSF_SWITCH);
        if (!csema_finish_switch_stmt(self->sema, switch_, body))
                return NULL;

        return switch_;
}

extern tree_stmt* cparse_while_stmt(cparser* self, int scope_flags)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_WHILE) || !cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* condition = cparse_expr(self);
        if (!condition)
                return NULL;

        tree_location rbracket_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACKET))
                return NULL;

        tree_stmt* body = _cparse_stmt(self, scope_flags | TSF_ITERATION);
        if (!body)
                return NULL;

        return csema_new_while_stmt(self->sema, kw_loc, rbracket_loc, condition, body);
}

extern tree_stmt* cparse_do_while_stmt(cparser* self, int scope_flags)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_DO))
                return NULL;

        tree_stmt* body = _cparse_stmt(self, scope_flags | TSF_ITERATION);
        if (!body || !cparser_require(self, CTK_WHILE) || !cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* condition = cparse_expr(self);
        if (!condition || !cparser_require(self, CTK_RBRACKET))
                return NULL;

        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return csema_new_do_while_stmt(self->sema, kw_loc, semicolon_loc, condition, body);
}

extern tree_stmt* cparse_for_stmt(cparser* self, int scope_flags)
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

        if (!(body = _cparse_stmt(self, scope_flags | TSF_ITERATION)))
                return NULL;

        csema_exit_decl_scope(self->sema);
        return csema_new_for_stmt(self->sema, kw_loc, rbracket_loc, init, condition, step, body);
}

extern tree_stmt* cparse_goto_stmt(cparser* self)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_GOTO))
                return NULL;

        tree_location id_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_ID))
                return NULL;

        tree_id id = ctoken_get_string(cparser_get_prev(self));
        tree_location semicolon_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return csema_new_goto_stmt(self->sema, kw_loc, id_loc, id, semicolon_loc);
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
