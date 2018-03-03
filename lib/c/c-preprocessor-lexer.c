#include "scc/c/c-preprocessor-lexer.h"
#include "scc/c/c-context.h"

extern void c_preprocessor_lexer_init_token(
        c_preprocessor_lexer* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        self->kind = CPLK_TOKEN;
        c_token_lexer_init(&self->token_lexer, reswords, source_manager, logger, context);
}

extern void c_preprocessor_lexer_init_macro(
        c_preprocessor_lexer* self,
        c_context* context,
        const c_reswords* reswords,
        c_macro* macro,
        c_logger* logger,
        tree_location loc)
{
        self->kind = CPLK_MACRO;
        c_macro_lexer_init(&self->macro_lexer, context, reswords, macro, logger, loc);
}

extern c_token* c_preprocessor_lexer_lex_token(c_preprocessor_lexer* self)
{
        switch (self->kind)
        {
                case CPLK_TOKEN:
                        return c_token_lexer_lex_token(&self->token_lexer);
                case CPLK_MACRO:
                        return c_macro_lexer_lex_token(&self->macro_lexer);

                default:
                        S_UNREACHABLE();
                        return NULL;
        }
}

#define DSEQ_VALUE_TYPE    c_preprocessor_lexer
#define DSEQ_TYPE          c_pp_lexer_stack
#define DSEQ_INIT          c_pp_lexer_stack_init
#define DSEQ_INIT_ALLOC    c_pp_lexer_stack_init_alloc
#define DSEQ_DISPOSE       c_pp_lexer_stack_dispose
#define DSEQ_GET_SIZE      c_pp_lexer_stack_size
#define DSEQ_GET_CAPACITY  c_pp_lexer_stack_capacity
#define DSEQ_GET_ALLOCATOR c_pp_lexer_stack_allocator
#define DSEQ_RESERVE       c_pp_lexer_stack_reserve
#define DSEQ_RESIZE        c_pp_lexer_stack_resize
#define DSEQ_GET_BEGIN     c_pp_lexer_stack_begin
#define DSEQ_GET_END       c_pp_lexer_stack_end
#define DSEQ_GET           c_pp_lexer_stack_get
#define DSEQ_SET           c_pp_lexer_stack_set
#define DSEQ_APPEND        c_pp_lexer_stack_append

#include "scc/scl/dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

extern void c_preprocessor_lexer_stack_init(c_preprocessor_lexer_stack* self, c_context* context)
{
        c_pp_lexer_stack_init_alloc(&self->lexers, c_context_get_allocator(context));
}

extern void c_preprocessor_lexer_stack_dispose(c_preprocessor_lexer_stack* self)
{
        c_pp_lexer_stack_dispose(&self->lexers);
}

extern void c_preprocessor_lexer_stack_pop_lexer(c_preprocessor_lexer_stack* self)
{
        ssize size = c_pp_lexer_stack_size(&self->lexers);
        S_ASSERT(size);
        c_pp_lexer_stack_resize(&self->lexers, size - 1);
}

extern ssize c_preprocessor_lexer_stack_size(const c_preprocessor_lexer_stack* self)
{
        return c_pp_lexer_stack_size(&self->lexers);
}

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_top(const c_preprocessor_lexer_stack* self)
{
        ssize size = c_preprocessor_lexer_stack_size(self);
        S_ASSERT(size);
        return c_pp_lexer_stack_begin(&self->lexers) + size - 1;
}

static c_preprocessor_lexer* c_preprocessor_lexer_stack_push_lexer(c_preprocessor_lexer_stack* self)
{
        ssize size = c_pp_lexer_stack_size(&self->lexers);
        c_pp_lexer_stack_resize(&self->lexers, size + 1);
        return c_pp_lexer_stack_begin(&self->lexers) + size;
}

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_push_token_lexer(
        c_preprocessor_lexer_stack* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        c_preprocessor_lexer* lexer = c_preprocessor_lexer_stack_push_lexer(self);
        c_preprocessor_lexer_init_token(lexer, reswords, source_manager, logger, context);
        return lexer;
}

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_push_macro_lexer(
        c_preprocessor_lexer_stack* self,
        c_context* context,
        const c_reswords* reswords,
        c_macro* macro,
        c_logger* logger,
        tree_location loc)
{
        c_preprocessor_lexer* lexer = c_preprocessor_lexer_stack_push_lexer(self);
        c_preprocessor_lexer_init_macro(lexer, context, reswords, macro, logger, loc);
        return lexer;
}