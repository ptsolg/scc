#ifndef C_LEXER_H
#define C_LEXER_H

#include "preprocessor.h"
#include "token.h"
#include "reswords.h"
#include <stdio.h>

typedef struct _file_entry file_entry;

typedef struct _c_lexer
{
        c_preprocessor pp;
        c_reswords reswords;
} c_lexer;

extern void c_lexer_init(c_lexer* self, c_context* context);
extern errcode c_lexer_enter_source_file(c_lexer* self, c_source* source);
extern void c_lexer_dispose(c_lexer* self);

extern c_token* c_lex(c_lexer* self);

extern errcode c_lex_source(c_context* context, file_entry* source, FILE* error, ptrvec* result);

#endif
