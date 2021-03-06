#ifndef C_PREPROCESSOR_H
#define C_PREPROCESSOR_H

#include "scc/lex/preprocessor-lexer.h"
#include "scc/lex/pragma.h"

typedef struct _c_macro c_macro;

typedef struct _c_preprocessor
{
        c_pp_lexer* lexer;
        size_t token_lexer_depth;
        c_lexer_stack lexer_stack;
        struct hashmap macro_lookup;
        const c_reswords* reswords;
        c_context* context;
        c_pragma_handlers pragma_handlers;

        struct
        {
                tree_id defined;
                tree_id link;
        } id;

        struct
        {
                c_token* next_unexpanded_token;
                c_token* next_expanded_token;
        } lookahead;

        struct
        {
                c_macro* line;
                c_macro* file;
        } builtin_macro;
} c_preprocessor;

extern void c_preprocessor_init(
        c_preprocessor* self, const c_reswords* reswords, c_context* context);

extern void c_preprocessor_dispose(c_preprocessor* self);
extern errcode c_preprocessor_enter_source(c_preprocessor* self, c_source* source);
extern void c_preprocessor_exit(c_preprocessor* self);
extern c_macro* c_preprocessor_get_macro(const c_preprocessor* self, tree_id name);
extern bool c_preprocessor_define_macro(c_preprocessor* self, c_macro* macro);
extern bool c_preprocessor_macro_defined(const c_preprocessor* self, tree_id name);
extern bool c_preprocessor_undef(c_preprocessor* self, tree_id name);

extern c_token* c_preprocess_non_comment(c_preprocessor* self);
extern c_token* c_preprocess_non_wspace(c_preprocessor* self);
extern c_token* c_preprocess_non_directive(c_preprocessor* self);
extern c_token* c_preprocess_non_macro(c_preprocessor* self);
extern c_token* c_preprocess(c_preprocessor* self);

#endif
