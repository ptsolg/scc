#ifndef CTOKEN_KIND_H
#define CTOKEN_KIND_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CTOKEN(T) CTK_##T

// C preprocessor token
#define CPP_KEYWORD(T) CTK_PP_##T

// {} [] () & * etc...
#define CPUNCTUATOR(T) CTOKEN(T)

// if else do while etc...
#define CKEYWORD(T)    CTOKEN(T)

typedef enum _ctoken_kind
{
#include "c-token-kind.inc"
} ctoken_kind;

#undef CTOKEN
#undef CPP_KEYWORD
#undef CPUNCTUATOR
#undef CKEYWORD

#define _TOSTR(X) #X
#define CTOKEN(T) _TOSTR(CTK_##T)
#define CPP_KEYWORD(T) _TOSTR(CTK_PP_##T)
#define CPUNCTUATOR(T) CTOKEN(T)
#define CKEYWORD(T)    CTOKEN(T)

static const char* ctoken_kind_to_string[] = 
{
#include "c-token-kind.inc"
        "",
};

#undef CTOKEN
#undef CPP_KEYWORD
#undef CPUNCTUATOR
#undef CKEYWORD
#undef _TOSTR

#ifdef __cplusplus
}
#endif

#endif // !CTOKEN_KIND_H