#ifndef C_PREPROCESSOR_LEXER
#define C_PREPROCESSOR_LEXER

#ifdef __cplusplus
extern "C" {
#endif

#include "c-token-lexer.h"
#include "c-macro-lexer.h"
#include "scc/core/dseq-common.h"

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
                c_token_lexer token_lexer;
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

extern c_token* c_preprocessor_lexer_lex_token(c_preprocessor_lexer* self);

typedef struct _c_preprocessor_lexer_stack
{
        struct _dseq lexers;
} c_preprocessor_lexer_stack;

extern void c_preprocessor_lexer_stack_init(c_preprocessor_lexer_stack* self, c_context* context);
extern void c_preprocessor_lexer_stack_dispose(c_preprocessor_lexer_stack* self);
extern void c_preprocessor_lexer_stack_pop_lexer(c_preprocessor_lexer_stack* self);
extern ssize c_preprocessor_lexer_stack_size(const c_preprocessor_lexer_stack* self);
extern c_preprocessor_lexer* c_preprocessor_lexer_stack_top(const c_preprocessor_lexer_stack* self);

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
