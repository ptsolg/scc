#ifndef C_PREPROCESSOR_LEXER
#define C_PREPROCESSOR_LEXER

#ifdef __cplusplus
extern "C" {
#endif

#include "c-token-lexer.h"
#include "c-macro-lexer.h"
#include "scc/core/dseq-common.h"

typedef struct
{
        c_token* token;
        bool condition;
} c_cond_directive_info;

extern void c_cond_directive_info_init(c_cond_directive_info* self, c_token* token, bool condition);

typedef enum
{
        CPLK_TOKEN,
        CPLK_MACRO,
} c_preprocessor_lexer_kind;

typedef struct _c_preprocessor_lexer
{
        c_preprocessor_lexer_kind kind;
        union
        {
                struct
                {
                        c_token_lexer token_lexer;
                        struct _dseq cond_stack;
                };
                c_macro_lexer macro_lexer;
        };
} c_preprocessor_lexer;

extern void c_preprocessor_lexer_init_token(
        c_preprocessor_lexer* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern void c_preprocessor_lexer_init_macro(
        c_preprocessor_lexer* self,
        c_context* context,
        const c_reswords* reswords,
        c_macro* macro,
        c_logger* logger,
        tree_location loc);

extern void c_preprocessor_lexer_dispose(c_preprocessor_lexer* self);

extern c_token* c_preprocessor_lexer_lex_token(c_preprocessor_lexer* self);

extern c_cond_directive_info* c_preprocessor_lexer_push_conditional_directive(
        c_preprocessor_lexer* self, c_token* token, bool condition);

extern c_cond_directive_info* c_preprocessor_lexer_get_conditional_directive(const c_preprocessor_lexer* self);
extern void c_preprocessor_lexer_pop_conditional_directive(c_preprocessor_lexer* self);
extern size_t c_preprocessor_lexer_get_conditional_directive_stack_depth(const c_preprocessor_lexer* self);

typedef struct _c_preprocessor_lexer_stack
{
        struct _dseq stack;
} c_preprocessor_lexer_stack;

extern void c_preprocessor_lexer_stack_init(c_preprocessor_lexer_stack* self, c_context* context);
extern void c_preprocessor_lexer_stack_dispose(c_preprocessor_lexer_stack* self);
extern void c_preprocessor_lexer_stack_pop_lexer(c_preprocessor_lexer_stack* self);
extern size_t c_preprocessor_lexer_stack_depth(const c_preprocessor_lexer_stack* self);
extern c_preprocessor_lexer* c_preprocessor_lexer_stack_top(const c_preprocessor_lexer_stack* self);
extern c_preprocessor_lexer* c_preprocessor_lexer_stack_get(
        const c_preprocessor_lexer_stack* self, size_t depth);

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_push_token_lexer(
        c_preprocessor_lexer_stack* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern c_preprocessor_lexer* c_preprocessor_lexer_stack_push_macro_lexer(
        c_preprocessor_lexer_stack* self,
        c_context* context,
        const c_reswords* reswords,
        c_macro* macro,
        c_logger* logger,
        tree_location loc);

#ifdef __cplusplus
}
#endif

#endif
