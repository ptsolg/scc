#ifndef C_LEXER_H
#define C_LEXER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-preprocessor.h"
#include "c-token.h"
#include "c-reswords.h"

typedef struct _c_lexer
{
        c_preprocessor pp;
        c_reswords reswords;
} c_lexer;

extern void c_lexer_init(
        c_lexer* self,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern serrcode c_lexer_enter_source_file(c_lexer* self, c_source* source);
extern void c_lexer_init_reswords(c_lexer* self);
extern void c_lexer_dispose(c_lexer* self);

extern c_token* c_lex(c_lexer* self);

#ifdef __cplusplus
}
#endif

#endif
