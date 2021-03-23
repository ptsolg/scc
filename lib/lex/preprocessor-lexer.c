#include "scc/lex/preprocessor-lexer.h"
#include "scc/c-common/context.h"

extern void c_init_cond_directive(
        c_cond_directive* self, c_token* token, bool condition, bool has_body)
{
        self->token = token;
        self->condition = condition;
        self->has_body = has_body;
}

extern void c_init_pp_token_lexer(c_pp_lexer* self, c_context* context)
{
        self->kind = CPLK_TOKEN;
        c_token_lexer_init(&self->token_lexer, context);
        c_cond_stack_init(&self->cond_stack);
}

extern void c_init_pp_macro_token_lexer(
        c_pp_lexer* self,
        c_context* context,
        c_macro* macro,
        tree_location loc)
{
        self->kind = CPLK_MACRO;
        c_macro_lexer_init(&self->macro_lexer, context, macro, loc);
}

extern void c_dispose_pp_lexer(c_pp_lexer* self)
{
        if (self->kind == CPLK_TOKEN)
                c_cond_stack_drop(&self->cond_stack);
}

extern c_token* c_pp_lex(c_pp_lexer* self)
{
        switch (self->kind)
        {
                case CPLK_TOKEN:
                        return c_token_lexer_lex_token(&self->token_lexer);
                case CPLK_MACRO:
                        return c_macro_lexer_lex_token(&self->macro_lexer);

                default:
                        UNREACHABLE();
                        return NULL;
        }
}

extern c_cond_directive* c_push_cond_directive(
        c_pp_lexer* self, c_token* token, bool condition)
{
        c_cond_directive d;
        c_init_cond_directive(&d, token, condition, false);
        c_cond_stack_push(&self->cond_stack, d);
        return c_cond_stack_last_ptr(&self->cond_stack);
}

extern c_cond_directive* c_get_cond_directive(const c_pp_lexer* self)
{
        return c_cond_stack_last_ptr(&self->cond_stack);
}

extern void c_pop_cond_directive(c_pp_lexer* self)
{
        c_cond_stack_pop(&self->cond_stack);
}

extern size_t c_cond_stack_depth(const c_pp_lexer* self)
{
        assert(self->kind == CPLK_TOKEN);
        return self->cond_stack.size;
}

extern void c_init_lexer_stack(c_lexer_stack* self, c_context* context)
{
        c_pp_lexer_stack_init(&self->lexers);
}

extern void c_dispose_lexer_stack(c_lexer_stack* self)
{
        while (c_lexer_stack_depth(self))
                c_pop_lexer(self);
        c_pp_lexer_stack_drop(&self->lexers);
}

extern void c_pop_lexer(c_lexer_stack* self)
{
        c_pp_lexer l = c_pp_lexer_stack_pop(&self->lexers);
        c_dispose_pp_lexer(&l);
}

extern size_t c_lexer_stack_depth(const c_lexer_stack* self)
{
        return self->lexers.size;
}

extern c_pp_lexer* c_lexer_stack_top(const c_lexer_stack* self)
{
        return c_pp_lexer_stack_last_ptr(&self->lexers);
}

extern c_pp_lexer* c_lexer_stack_get(
        const c_lexer_stack* self, size_t depth)
{
        return self->lexers.items + depth;
}

static c_pp_lexer* push_lexer(c_lexer_stack* self, c_pp_lexer l)
{
        c_pp_lexer_stack_push(&self->lexers, l);
        return c_pp_lexer_stack_last_ptr(&self->lexers);
}

extern c_pp_lexer* c_push_token_lexer(c_lexer_stack* self, c_context* context)
{
        c_pp_lexer l;
        c_init_pp_token_lexer(&l, context);
        return push_lexer(self, l);
}

extern c_pp_lexer* c_push_macro_lexer(
        c_lexer_stack* self,
        c_context* context,
        c_macro* macro,
        tree_location loc)
{
        c_pp_lexer l;
        c_init_pp_macro_token_lexer(&l, context, macro, loc);
        return push_lexer(self, l);
}
