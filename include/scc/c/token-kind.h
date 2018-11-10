#ifndef C_TOKEN_KIND_H
#define C_TOKEN_KIND_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define C_TOKEN(T) CTK_##T

// C preprocessor token
#define C_PP_KEYWORD(T) CTK_PP_##T

// {} [] () & * etc...
#define C_PUNCTUATOR(T) C_TOKEN(T)

// if else do while etc...
#define C_KEYWORD(T) C_TOKEN(T)

typedef enum _c_token_kind
{
#include "token-kind.inc"
} c_token_kind;

#undef C_TOKEN
#undef C_PP_KEYWORD
#undef C_PUNCTUATOR
#undef C_KEYWORD

#define _TOSTR(X) #X
#define C_TOKEN(T) _TOSTR(CTK_##T)
#define C_PP_KEYWORD(T) _TOSTR(CTK_PP_##T)
#define C_PUNCTUATOR(T) C_TOKEN(T)
#define C_KEYWORD(T) C_TOKEN(T)

static const char* c_token_kind_to_string[] = 
{
#include "token-kind.inc"
        "",
};

#undef C_TOKEN
#undef C_PP_KEYWORD
#undef C_PUNCTUATOR
#undef C_KEYWORD
#undef _TOSTR

#ifdef __cplusplus
}
#endif

#endif
