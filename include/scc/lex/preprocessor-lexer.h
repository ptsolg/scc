#ifndef C_PREPROCESSOR_LEXER
#define C_PREPROCESSOR_LEXER


#include "scc/lex/macro-lexer.h"
#include "scc/lex/token-lexer.h"

typedef struct
{
        c_token* token;
        bool condition;
        bool has_body;
} c_cond_directive;

extern void c_init_cond_directive(
        c_cond_directive* self, c_token* token, bool condition, bool has_body);

typedef enum
{
        CPLK_TOKEN,
        CPLK_MACRO,
} c_pp_lexer_kind;

#define VEC   c_cond_stack
#define VEC_T c_cond_directive
#include "scc/core/vec.inc"

typedef struct _c_pp_lexer
{
        c_pp_lexer_kind kind;
        union
        {
                struct
                {
                        c_token_lexer token_lexer;
                        struct c_cond_stack cond_stack;
                };
                c_macro_lexer macro_lexer;
        };
} c_pp_lexer;

extern void c_init_pp_token_lexer(c_pp_lexer* self, c_context* context);

extern void c_init_pp_macro_token_lexer(
        c_pp_lexer* self,
        c_context* context,
        c_macro* macro,
        tree_location loc);

extern void c_dispose_pp_lexer(c_pp_lexer* self);

extern c_token* c_pp_lex(c_pp_lexer* self);

extern c_cond_directive* c_push_cond_directive(
        c_pp_lexer* self, c_token* token, bool condition);

extern c_cond_directive* c_get_cond_directive(const c_pp_lexer* self);
extern void c_pop_cond_directive(c_pp_lexer* self);
extern size_t c_cond_stack_depth(const c_pp_lexer* self);

#define VEC   c_pp_lexer_stack
#define VEC_T c_pp_lexer
#include "scc/core/vec.inc"

typedef struct _c_lexer_stack
{
        struct c_pp_lexer_stack lexers;
} c_lexer_stack;

extern void c_init_lexer_stack(c_lexer_stack* self, c_context* context);
extern void c_dispose_lexer_stack(c_lexer_stack* self);
extern void c_pop_lexer(c_lexer_stack* self);
extern size_t c_lexer_stack_depth(const c_lexer_stack* self);
extern c_pp_lexer* c_lexer_stack_top(const c_lexer_stack* self);
extern c_pp_lexer* c_lexer_stack_get(
        const c_lexer_stack* self, size_t depth);

extern c_pp_lexer* c_push_token_lexer(c_lexer_stack* self, c_context* context);

extern c_pp_lexer* c_push_macro_lexer(
        c_lexer_stack* self,
        c_context* context,
        c_macro* macro,
        tree_location loc);

#endif
