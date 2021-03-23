#include "scc/syntax/parser.h"
#include "scc/lex/reswords.h"
#include "scc/semantics/sema.h"
#include "errors.h"
#include <setjmp.h>

extern void c_parser_init(c_parser* self, c_context* context, c_lexer* lexer, c_sema* sema)
{
        self->lexer = lexer;
        self->sema = sema;
        self->context = context;
        self->on_error = NULL;
        self->buffer[0] = NULL;
        self->buffer[1] = NULL;
        self->buffer[2] = NULL;
}

extern void c_parser_set_on_error(c_parser* self, void* b)
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
                c_error_expected_a_before_b(self->context, loc, expected[0], current);
        else if (size == 2)
                c_error_expected_a_or_b_before_c(self->context,
                        loc, expected[0], expected[1], current);
        else
                c_error_expected_one_of(self->context, loc, expected, size, current);
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

extern tree_module* c_parse_module(c_parser* self)
{
        while (!c_parser_at(self, CTK_EOF))
                if (!c_parse_decl(self))
                        return NULL;

        return self->sema->module;
}

static void init_builtin_function(c_sema* self, tree_function_builtin_kind kind, const char* name, const char* decl)
{
        c_source* s = c_source_emulate(&self->ccontext->source_manager, name, decl);
        c_lexer lexer;
        c_lexer_init(&lexer, self->ccontext);
        if (!s || EC_FAILED(c_lexer_enter_source_file(&lexer, s)))
                goto error;

        c_parser parser;
        c_parser_init(&parser, self->ccontext, &lexer, self);
        jmp_buf on_parser_error;
        c_parser_set_on_error(&parser, on_parser_error);
        if (setjmp(on_parser_error))
                goto error;

        c_parser_enter_token_stream(&parser);
        tree_decl* func = c_parse_decl(&parser);
        if (!func)
                goto error;

        tree_set_func_builtin_kind(func, kind);
        tree_set_decl_implicit(func, true);
        return;

error:
        assert(0 && "unexpected error");
}

static void init_builtin_functions(c_sema* self)
{
        init_builtin_function(self,
                TFBK_ATOMIC_CMPXCHG_32_WEAK_SEQ_CST, "__atomic_cmpxchg_32_weak_seq_cst",
                "static int __atomic_cmpxchg_32_weak_seq_cst("
                "volatile unsigned* ptr, unsigned expected, unsigned desired);");

        init_builtin_function(self,
                TFBK_ATOMIC_ADD_FETCH_32_SEQ_CST, "__atomic_add_fetch_32_seq_cst",
                "static unsigned __atomic_add_fetch_32_seq_cst(volatile unsigned* ptr, unsigned value);");

        init_builtin_function(self,
                TFBK_ATOMIC_XCHG_32_SEQ_CST, "__atomic_xchg_32_seq_cst",
                "static void __atomic_xchg_32_seq_cst(volatile unsigned* ptr, unsigned value);");

        init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_SEQ_CST, "__atomic_fence_st_seq_cst",
                "static void __atomic_fence_st_seq_cst();");

        init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_ACQ, "__atomic_fence_st_acq",
                "static void __atomic_fence_st_acq();");

        init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_REL, "__atomic_fence_st_rel",
                "static void __atomic_fence_st_rel();");

        init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_ACQ_REL, "__atomic_fence_st_acq_rel",
                "static void __atomic_fence_st_acq_rel();");
}

extern tree_module* c_parse_source(
        c_context* context, file_entry* source, c_pragma_handlers handlers, FILE* error)
{
        c_lexer lexer;
        c_parser parser;
        c_sema sema;
        tree_module* result = NULL;

        c_lexer_init(&lexer, context);
        lexer.pp.pragma_handlers = handlers;
        c_sema_init(&sema, context);
        c_sema_new_module(&sema);
        init_builtin_functions(&sema);
        c_parser_init(&parser, context, &lexer, &sema);
        
        c_source* s = c_source_get_from_file(&context->source_manager, source);
        if (!source || EC_FAILED(c_lexer_enter_source_file(&lexer, s)))
                goto cleanup;

        jmp_buf on_parser_error;
        if (setjmp(on_parser_error))
                goto cleanup;

        c_parser_set_on_error(&parser, on_parser_error);
        c_parser_enter_token_stream(&parser);
        result = c_parse_module(&parser);

cleanup:
        c_parser_dispose(&parser);
        c_sema_dispose(&sema);
        c_lexer_dispose(&lexer);
        return result;
}
