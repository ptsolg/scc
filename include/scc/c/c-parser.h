#ifndef CPARSER_H
#define CPARSER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-lexer.h"

typedef struct _clogger clogger;
typedef struct _csema csema;
typedef struct _tree_scope tree_scope;
typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_module tree_module;

typedef struct _cparser
{
        // prev, current and next
        ctoken* buffer[3];
        clexer* lexer;
        csema* sema;
        clogger* logger;
        int* on_error;
} cparser;

extern void cparser_init(cparser* self, clexer* lexer, csema* sema, clogger* logger);

extern void cparser_dispose(cparser* self);
extern void cparser_set_on_error(cparser* self, int* b);
extern void cparser_die(const cparser* self, int code);

extern void cparser_enter_token_stream(cparser* self);

// tries to resolve error, never returns NULL
extern ctoken* cparser_handle_lex_error(const cparser* self);

extern ctoken* cparser_consume_token(cparser* self);
extern ctoken* cparser_get_token(const cparser* self);
extern ctoken* cparser_get_next(const cparser* self);
extern ctoken* cparser_get_prev(const cparser* self);
extern bool cparser_at(const cparser* self, ctoken_kind k);
extern bool cparser_next_token_is(cparser* self, ctoken_kind k);
extern bool cparser_require(cparser* self, ctoken_kind k);
extern bool cparser_require_ex(cparser* self, ctoken_kind k, const ctoken_kind expected[]);

extern tree_location cparser_get_loc(const cparser* self);

#ifdef __cplusplus
}
#endif

#endif // !CPARSER_H
