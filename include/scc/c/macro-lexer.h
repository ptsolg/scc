#ifndef C_MACRO_LEXER_H
#define C_MACRO_LEXER_H

#include "scc/tree/common.h"
#include "scc/core/vec.h"

typedef struct _c_context c_context;
typedef struct _c_macro c_macro;
typedef struct _c_macro_args c_macro_args;
typedef struct _c_token c_token;
typedef struct _c_logger c_logger;
typedef struct _c_reswords c_reswords;

typedef struct _c_macro_lexer
{
        c_token** pos;
        ptrvec tokens;
        c_context* context;
        c_macro* macro;
        c_logger* logger;
        tree_location loc;
} c_macro_lexer;

extern void c_macro_lexer_init(
        c_macro_lexer* self,
        c_context* context,
        c_macro* macro,
        c_logger* logger,
        tree_location loc);

extern errcode c_macro_lexer_substitute_macro_args(c_macro_lexer* self, c_macro_args* args);
extern c_token* c_macro_lexer_lex_token(c_macro_lexer* self);

#endif