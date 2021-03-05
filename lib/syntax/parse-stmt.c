#include "scc/syntax/parser.h"
#include "scc/semantics/sema.h"
#include "scc/tree/stmt.h"

static tree_stmt* _c_parse_stmt_no_check(c_parser* self, int scope_flags)
{
        switch (c_token_get_kind(c_parser_get_token(self)))
        {
                case CTK_CASE:     return c_parse_case_stmt(self, scope_flags);
                case CTK_DEFAULT:  return c_parse_default_stmt(self, scope_flags);
                case CTK_LBRACE:   return c_parse_compound_stmt(self, scope_flags);
                case CTK_IF:       return c_parse_if_stmt(self, scope_flags);
                case CTK_SWITCH:   return c_parse_switch_stmt(self, scope_flags);
                case CTK_WHILE:    return c_parse_while_stmt(self, scope_flags);
                case CTK_DO:       return c_parse_do_while_stmt(self, scope_flags);
                case CTK_FOR:      return c_parse_for_stmt(self, scope_flags);
                case CTK_GOTO:     return c_parse_goto_stmt(self, scope_flags);
                case CTK_CONTINUE: return c_parse_continue_stmt(self);
                case CTK_BREAK:    return c_parse_break_stmt(self);
                case CTK_RETURN:   return c_parse_return_stmt(self);
                case CTK_ATOMIC:   return c_parse_atomic_stmt(self, scope_flags);

                default:
                        if (c_parser_next_token_is(self, CTK_COLON))
                                return c_parse_labeled_stmt(self, scope_flags);

                        return c_parser_at_declaration(self)
                                ? c_parse_decl_stmt(self)
                                : c_parse_expr_stmt(self);
        }
}

static tree_stmt* _c_parse_stmt(c_parser* self, int scope_flags)
{
        scope_flags &= ~SCOPE_FLAG_DECL;

        tree_stmt* s = _c_parse_stmt_no_check(self, scope_flags);
        if (!s || !c_sema_check_stmt(self->sema, s, scope_flags))
                return NULL;

        return s;
}

static tree_stmt* _c_parse_block_item(c_parser* self, int scope_flags)
{
        scope_flags |= SCOPE_FLAG_DECL;

        tree_stmt* s = _c_parse_stmt_no_check(self, scope_flags);
        if (!s || !c_sema_check_stmt(self->sema, s, scope_flags))
                return NULL;

        return s;
}

extern tree_stmt* c_parse_stmt(c_parser* self)
{
        return _c_parse_stmt(self, SCOPE_FLAG_NONE);
}

extern tree_stmt* c_parse_case_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_CASE))
                return NULL;

        tree_expr* value = c_parse_const_expr(self);
        if (!value)
                return NULL;

        tree_location colon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_COLON))
                return NULL;

        tree_stmt* case_stmt = c_sema_start_case_stmt(self->sema, kw_loc, colon_loc, value);
        if (!case_stmt)
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags);
        if (!body)
                return NULL;

        return c_sema_finish_case_stmt(self->sema, case_stmt, body);
}

extern tree_stmt* c_parse_default_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_DEFAULT))
                return NULL;

        tree_location colon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_COLON))
                return NULL;

        tree_stmt* default_stmt = c_sema_start_default_stmt(self->sema, kw_loc, colon_loc);
        if (!default_stmt)
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags);
        if (!body)
                return NULL;

        return c_sema_finish_default_stmt(self->sema, default_stmt, body);
}

extern tree_stmt* c_parse_labeled_stmt(c_parser* self, int scope_flags)
{
        tree_location id_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_ID))
                return NULL;

        tree_id name = c_token_get_string(c_parser_get_prev(self));
        tree_location colon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_COLON))
                return NULL;

        tree_stmt placeholder;
        tree_decl* label = c_sema_define_label_decl(
                self->sema, id_loc, name, colon_loc, &placeholder);
        if (!label)
                return NULL;

        tree_stmt* stmt = _c_parse_stmt(self, scope_flags);
        if (!stmt)
                return NULL;

        return c_sema_new_labeled_stmt(self->sema, label, stmt);
}

extern tree_stmt* c_parse_compound_stmt(c_parser* self, int scope_flags)
{
        tree_location lbrace_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_LBRACE))
                return NULL;

        tree_stmt* compound = c_sema_new_compound_stmt(self->sema, lbrace_loc);
        if (!compound)
                return NULL;

        c_sema_enter_scope(self->sema, tree_get_compound_scope(compound));
        while (!c_parser_at(self, CTK_RBRACE))
        {
                tree_stmt* item = _c_parse_block_item(self, scope_flags);
                if (!item)
                {
                        c_sema_exit_scope(self->sema);
                        return NULL;
                }
                c_sema_add_compound_stmt_item(self->sema, compound, item);
        }
        c_sema_exit_scope(self->sema);

        return c_parser_require(self, CTK_RBRACE) ? compound : NULL;
}

extern tree_stmt* c_parse_decl_stmt(c_parser* self)
{
        tree_location begin_loc = c_parser_get_loc(self);
        tree_decl* d = c_parse_decl(self);
        if (!d)
                return NULL;

        tree_location semicolon_loc = c_token_get_loc(c_parser_get_prev(self));
        return c_sema_new_decl_stmt(self->sema, begin_loc, semicolon_loc, d);
}

extern tree_stmt* c_parse_expr_stmt(c_parser* self)
{
        tree_expr* e = NULL;
        tree_location begin_loc = c_parser_get_loc(self);
        if (!c_parser_at(self, CTK_SEMICOLON))
                if (!(e = c_parse_expr(self)))
                        return NULL;

        tree_location semicolon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_new_expr_stmt(self->sema, begin_loc, semicolon_loc, e);
}

extern tree_stmt* c_parse_if_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_IF) || !c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* condition = c_parse_expr(self);
        if (!condition)
                return NULL;

        tree_location rbracket_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RBRACKET))
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags);
        if (!body)
                return NULL;

        tree_stmt* else_ = NULL;
        if (c_parser_at(self, CTK_ELSE))
        {
                c_parser_consume_token(self);
                if (!(else_ = _c_parse_stmt(self, scope_flags)))
                        return NULL;
        }

        return c_sema_new_if_stmt(self->sema, kw_loc, rbracket_loc, condition, body, else_);
}

extern tree_stmt* c_parse_switch_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SWITCH) || !c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* value = c_parse_expr(self);
        if (!value)
                return NULL;

        tree_location rbracket_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RBRACKET))
                return NULL;

        tree_stmt* switch_ = c_sema_start_switch_stmt(
                self->sema, kw_loc, rbracket_loc, value, scope_flags);
        if (!switch_)
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags | SCOPE_FLAG_SWITCH);
        if (!body)
                return NULL;

        return c_sema_finish_switch_stmt(self->sema, switch_, body);
}

extern tree_stmt* c_parse_while_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_WHILE) || !c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* condition = c_parse_expr(self);
        if (!condition)
                return NULL;

        tree_location rbracket_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RBRACKET))
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags | SCOPE_FLAG_ITERATION);
        if (!body)
                return NULL;

        return c_sema_new_while_stmt(self->sema, kw_loc, rbracket_loc, condition, body);
}

extern tree_stmt* c_parse_do_while_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_DO))
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags | SCOPE_FLAG_ITERATION);
        if (!body || !c_parser_require(self, CTK_WHILE) || !c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_expr* condition = c_parse_expr(self);
        if (!condition || !c_parser_require(self, CTK_RBRACKET))
                return NULL;

        tree_location semicolon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_new_do_while_stmt(self->sema, kw_loc, semicolon_loc, condition, body);
}

extern tree_stmt* c_parse_for_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_FOR))
                return NULL;
        if (!c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_stmt* init = NULL;
        tree_expr* condition = NULL;
        tree_expr* step = NULL;
        tree_stmt* body = NULL;

        c_sema_push_scope(self->sema);
        if (c_parser_at_declaration(self))
        {
                if (!(init = c_parse_decl_stmt(self)))
                        return NULL;
        }
        else
                if (!(init = c_parse_expr_stmt(self)))
                        return NULL;

        if (!c_parser_at(self, CTK_SEMICOLON))
                if (!(condition = c_parse_expr(self)))
                        return NULL;
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        if (!c_parser_at(self, CTK_RBRACKET))
                if (!(step = c_parse_expr(self)))
                        return NULL;
        tree_location rbracket_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RBRACKET))
                return NULL;

        if (!(body = _c_parse_stmt(self, scope_flags | SCOPE_FLAG_ITERATION)))
                return NULL;

        c_sema_exit_decl_scope(self->sema);
        return c_sema_new_for_stmt(self->sema, kw_loc, rbracket_loc, init, condition, step, body);
}

extern tree_stmt* c_parse_goto_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_GOTO))
                return NULL;

        tree_location id_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_ID))
                return NULL;

        tree_id id = c_token_get_string(c_parser_get_prev(self));
        tree_location semicolon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_new_goto_stmt(self->sema, kw_loc, id_loc, id, semicolon_loc, scope_flags);
}

extern tree_stmt* c_parse_continue_stmt(c_parser* self)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_CONTINUE))
                return NULL;

        tree_location semicolon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_new_continue_stmt(self->sema, kw_loc, semicolon_loc);
}

extern tree_stmt* c_parse_break_stmt(c_parser* self)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_BREAK))
                return NULL;

        tree_location semicolon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_new_break_stmt(self->sema, kw_loc, semicolon_loc);
}

extern tree_stmt* c_parse_return_stmt(c_parser* self)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RETURN))
                return NULL;

        tree_expr* value = NULL;
        if (!c_parser_at(self, CTK_SEMICOLON))
                if (!(value = c_parse_expr(self)))
                        return NULL;

        tree_location semicolon_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_new_return_stmt(self->sema, kw_loc, semicolon_loc, value);
}

extern tree_stmt* c_parse_atomic_stmt(c_parser* self, int scope_flags)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_ATOMIC))
                return NULL;

        tree_stmt* atomic = c_sema_start_atomic_stmt(self->sema, kw_loc);
        if (!atomic)
                return NULL;

        tree_stmt* body = _c_parse_stmt(self, scope_flags | SCOPE_FLAG_ATOMIC);
        if (!body)
                return NULL;

        return c_sema_finish_atomic_stmt(self->sema, atomic, body);;
}
