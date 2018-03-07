#include "scc/c/c-preprocessor-lexer.h"
#include "scc/c/c-context.h"
#include "scc/core/dseq-instance.h"

#define DSEQ_VALUE_TYPE    c_cond_directive_info
#define DSEQ_TYPE          c_pp_cond_stack
#define DSEQ_INIT          c_pp_cond_stack_init
#define DSEQ_INIT_ALLOC    c_pp_cond_stack_init_alloc
#define DSEQ_DISPOSE       c_pp_cond_stack_dispose
#define DSEQ_GET_SIZE      c_pp_cond_stack_size
#define DSEQ_GET_CAPACITY  c_pp_cond_stack_capacity
#define DSEQ_GET_ALLOCATOR c_pp_cond_stack_allocator
#define DSEQ_RESERVE       c_pp_cond_stack_reserve
#define DSEQ_RESIZE        c_pp_cond_stack_resize
#define DSEQ_GET_BEGIN     c_pp_cond_stack_begin
#define DSEQ_GET_END       c_pp_cond_stack_end
#define DSEQ_GET           c_pp_cond_stack_get
#define DSEQ_SET           c_pp_cond_stack_set
#define DSEQ_APPEND        c_pp_cond_stack_append

#include "scc/core/dseq.h"

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

extern void c_cond_directive_info_init(c_cond_directive_info* self, c_token* token, bool condition)
{
        self->token = token;
        self->condition = condition;
}

extern void c_preprocessor_lexer_init_token(
        c_preprocessor_lexer* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        self->kind = CPLK_TOKEN;
        c_token_lexer_init(&self->token_lexer, reswords, source_manager, logger, context);
        c_pp_cond_stack_init_alloc(&self->cond_stack, c_context_get_allocator(context));
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

extern void c_preprocessor_lexer_dispose(c_preprocessor_lexer* self)
{
        if (self->kind == CPLK_TOKEN)
                c_pp_cond_stack_dispose(&self->cond_stack);
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
                        UNREACHABLE();
                        return NULL;
        }
}

extern c_cond_directive_info* c_preprocessor_lexer_push_conditional_directive(
        c_preprocessor_lexer* self, c_token* token, bool condition)
{
        size_t depth = c_preprocessor_lexer_get_conditional_directive_stack_depth(self);
        c_pp_cond_stack_resize(&self->cond_stack, depth + 1);
        c_cond_directive_info* info = c_pp_cond_stack_begin(&self->cond_stack) + depth;
        info->token = token;
        info->condition = condition;
        return info;
}

extern c_cond_directive_info* c_preprocessor_lexer_get_conditional_directive(const c_preprocessor_lexer* self)
{
        size_t depth = c_preprocessor_lexer_get_conditional_directive_stack_depth(self);
        assert(depth);
        return c_pp_cond_stack_begin(&self->cond_stack) + depth - 1;
}

extern void c_preprocessor_lexer_pop_conditional_directive(c_preprocessor_lexer* self)
{
        size_t depth = c_preprocessor_lexer_get_conditional_directive_stack_depth(self);
        assert(depth);
        c_pp_cond_stack_resize(&self->cond_stack, depth - 1);
}

extern size_t c_preprocessor_lexer_get_conditional_directive_stack_depth(const c_preprocessor_lexer* self)
{
        assert(self->kind == CPLK_TOKEN);
        return c_pp_cond_stack_size(&self->cond_stack);
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

#include "scc/core/dseq.h"

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
        c_pp_lexer_stack_init_alloc(&self->stack, c_context_get_allocator(context));
}

extern void c_preprocessor_lexer_stack_dispose(c_preprocessor_lexer_stack* self)
{
        while (c_preprocessor_lexer_stack_depth(self))
                c_preprocessor_lexer_stack_pop_lexer(self);
        c_pp_lexer_stack_dispose(&self->stack);
}

extern void c_preprocessor_lexer_stack_pop_lexer(c_preprocessor_lexer_stack* self)
{
        size_t size = c_pp_lexer_stack_size(&self->stack);
        assert(size);
        c_preprocessor_lexer_dispose(c_preprocessor_lexer_stack_top(self));
        c_pp_lexer_stack_resize(&self->stack, size - 1);
}

extern size_t c_preprocessor_lexer_stack_depth(const c_preprocessor_lexer_stack* self)
{
        return c_pp_lexer_stack_size(&self->stack);
}

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_top(const c_preprocessor_lexer_stack* self)
{
        size_t size = c_preprocessor_lexer_stack_depth(self);
        assert(size);
        return c_pp_lexer_stack_begin(&self->stack) + size - 1;
}

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_get(
        const c_preprocessor_lexer_stack* self, size_t depth)
{
        return c_pp_lexer_stack_begin(&self->stack) + depth;
}

static c_preprocessor_lexer* c_preprocessor_lexer_stack_push_lexer(c_preprocessor_lexer_stack* self)
{
        size_t size = c_pp_lexer_stack_size(&self->stack);
        c_pp_lexer_stack_resize(&self->stack, size + 1);
        return c_pp_lexer_stack_begin(&self->stack) + size;
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