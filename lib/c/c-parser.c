#include "scc/c/c-parser.h"
#include "scc/c/c-errors.h"
#include "c-misc.h"
#include "scc/c/c-reswords.h"
#include <setjmp.h>

extern void c_parser_init(c_parser* self, c_lexer* lexer, c_sema* sema, c_logger* logger)
{
        self->lexer = lexer;
        self->sema = sema;
        self->logger = logger;
        self->on_error = NULL;
        self->buffer[0] = NULL;
        self->buffer[1] = NULL;
        self->buffer[2] = NULL;
}

extern void c_parser_set_on_error(c_parser* self, int* b)
{
        self->on_error = b;
}

extern void c_parser_dispose(c_parser* self)
{
        // todo
}

extern void c_parser_die(const c_parser* self, int code)
{
        longjmp(self->on_error, code);
}

extern void c_parser_enter_token_stream(c_parser* self)
{
        c_parser_consume_token(self);
        c_parser_consume_token(self);
}

extern c_token* c_parser_handle_lex_error(const c_parser* self)
{
        c_parser_die(self, EC_ERROR);
        // todo
        return NULL;
}

extern c_token* c_parser_consume_token(c_parser* self)
{
        c_token* t = c_lex(self->lexer);
        if (!t)
                return c_parser_handle_lex_error(self);
        assert(t);

        self->buffer[0] = self->buffer[1];
        self->buffer[1] = self->buffer[2];
        self->buffer[2] = t;
        return c_parser_get_token(self);
}

extern c_token* c_parser_get_token(const c_parser* self)
{
        return self->buffer[1];
}

extern c_token* c_parser_get_next(const c_parser* self)
{
        return self->buffer[2];
}

extern bool c_parser_at(const c_parser* self, c_token_kind k)
{
        return c_token_is(c_parser_get_token(self), k);
}

extern c_token* c_parser_get_prev(const c_parser* self)
{
        return self->buffer[0];
}

extern bool c_parser_next_token_is(c_parser* self, c_token_kind k)
{
        return c_token_is(c_parser_get_next(self), k);
}

extern bool c_parser_require(c_parser* self, c_token_kind k)
{
        return c_parser_require_ex(self, k, NULL);
}

static void c_parser_print_expected(const c_parser* self, c_token_kind k, const c_token_kind expected_ex[])
{
        c_token_kind expected[128];
        c_token_kind* it = expected;
        if(k != CTK_UNKNOWN)
                *it++ = k;
        while (expected_ex && *expected_ex != CTK_UNKNOWN)
        {
                if (k != *expected_ex)
                        *it++ = *expected_ex++;
                else
                        expected_ex++;
        }

        size_t size = it - expected;
        tree_location loc = c_parser_get_loc(self);
        c_token_kind current = c_token_get_kind(c_parser_get_token(self));

        if (size == 0)
                UNREACHABLE();
        else if (size == 1)
                c_error_expected_a_before_b(self->logger, loc, expected[0], current);
        else if (size == 2)
                c_error_expected_a_or_b_before_c(self->logger,
                        loc, expected[0], expected[1], current);
        else
                c_error_expected_one_of(self->logger, loc, expected, size, current);
}

extern bool c_parser_require_ex(c_parser* self, c_token_kind k, const c_token_kind expected[])
{
        if (!c_parser_at(self, k))
        {
                c_parser_print_expected(self, k, expected);
                return false;
        }
        c_parser_consume_token(self);
        return true;
}

extern tree_location c_parser_get_loc(const c_parser* self)
{
        return c_token_get_loc(c_parser_get_token(self));
}