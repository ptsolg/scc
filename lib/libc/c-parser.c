#include "c-parser.h"
#include "c-error.h"
#include "c-info.h"
#include "c-reswords.h"
#include <setjmp.h>

extern void cparser_init(cparser* self, clexer* lexer, cprog* prog, cerror_manager* error_manager)
{
        self->lexer = lexer;
        self->prog = prog;
        self->error_manager = error_manager;
        self->on_error = NULL;
        self->buffer[0] = NULL;
        self->buffer[1] = NULL;
        self->buffer[2] = NULL;
}

extern void cparser_set_on_error(cparser* self, int* b)
{
        self->on_error = b;
}

extern void cparser_dispose(cparser* self)
{
        // todo
}

extern void cparser_die(const cparser* self, int code)
{
        longjmp(self->on_error, code);
}

extern void cparser_enter_token_stream(cparser* self)
{
        cparser_consume_token(self);
        cparser_consume_token(self);
}

extern ctoken* cparser_handle_lex_error(const cparser* self)
{
        cparser_die(self, S_ERROR);
        // todo
        return NULL;
}

extern ctoken* cparser_consume_token(cparser* self)
{
        ctoken* t = clex(self->lexer);
        if (!t)
                return cparser_handle_lex_error(self);
        S_ASSERT(t);

        self->buffer[0] = self->buffer[1];
        self->buffer[1] = self->buffer[2];
        self->buffer[2] = t;
        return cparser_get_token(self);
}

extern ctoken* cparser_get_token(const cparser* self)
{
        return self->buffer[1];
}

extern ctoken* cparser_get_next(const cparser* self)
{
        return self->buffer[2];
}

extern bool cparser_at(const cparser* self, ctoken_kind k)
{
        return ctoken_is(cparser_get_token(self), k);
}

extern ctoken* cparser_get_prev(const cparser* self)
{
        return self->buffer[0];
}

extern bool cparser_next_token_is(cparser* self, ctoken_kind k)
{
        return ctoken_is(cparser_get_next(self), k);
}

extern bool cparser_require(cparser* self, ctoken_kind k)
{
        return cparser_require_ex(self, k, NULL);
}

static void cparser_print_expected(const cparser* self, ctoken_kind k, const ctoken_kind expected_ex[])
{
        ctoken_kind expected[128];
        ctoken_kind* it = expected;
        if(k != CTK_UNKNOWN)
                *it++ = k;
        while (expected_ex && *expected_ex != CTK_UNKNOWN)
        {
                if (k != *expected_ex)
                        *it++ = *expected_ex++;
                else
                        expected_ex++;
        }

        ssize size = it - expected;

        tree_location loc = cparser_get_loc(self);
        const cresword_info* current = cget_token_info(cparser_get_token(self));
        if (size == 0)
                S_UNREACHABLE();
        else if (size == 1)
        {
                cerror(
                        self->error_manager,
                        CES_ERROR,
                        loc,
                        "expected %s before %s %s",
                        cget_token_kind_info(expected[0])->desription,
                        current->desription,
                        current->kind);
        }
        else if (size == 2)
        {
                cerror(
                        self->error_manager,
                        CES_ERROR,
                        loc,
                        "expected %s or %s before %s %s",
                        cget_token_kind_info(expected[0])->desription,
                        cget_token_kind_info(expected[1])->desription,
                        current->desription,
                        current->kind);
        }
        else
        {
                char buffer[1024];
                sprintf(buffer, "expected one of: ");

                for (ssize i = 0; i < size; i++)
                {
                        strcat(buffer, cget_token_kind_info(expected[i])->desription);
                        if (i + 1 < size)
                                strcat(buffer, ", ");
                }
                cerror(
                        self->error_manager,
                        CES_ERROR,
                        loc,
                        "%s before %s %s",
                        buffer,
                        current->desription,
                        current->kind);
        }
}

extern bool cparser_require_ex(cparser* self, ctoken_kind k, const ctoken_kind expected[])
{
        if (!cparser_at(self, k))
        {
                cparser_print_expected(self, k, expected);
                return false;
        }
        cparser_consume_token(self);
        return true;
}

extern tree_location cparser_get_loc(const cparser* self)
{
        return ctoken_get_loc(cparser_get_token(self));
}