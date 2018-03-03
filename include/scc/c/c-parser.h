#ifndef C_PARSER_H
#define C_PARSER_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-lexer.h"

typedef struct _c_logger c_logger;
typedef struct _c_sema c_sema;
typedef struct _tree_scope tree_scope;
typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_module tree_module;

typedef struct _c_parser
{
        // prev, current and next
        c_token* buffer[3];
        c_lexer* lexer;
        c_sema* sema;
        c_logger* logger;
        int* on_error;
} c_parser;

extern void c_parser_init(c_parser* self, c_lexer* lexer, c_sema* sema, c_logger* logger);

extern void c_parser_dispose(c_parser* self);
extern void c_parser_set_on_error(c_parser* self, int* b);
extern void c_parser_die(const c_parser* self, int code);

extern void c_parser_enter_token_stream(c_parser* self);

// tries to resolve error, never returns NULL
extern c_token* c_parser_handle_lex_error(const c_parser* self);

extern c_token* c_parser_consume_token(c_parser* self);
extern c_token* c_parser_get_token(const c_parser* self);
extern c_token* c_parser_get_next(const c_parser* self);
extern c_token* c_parser_get_prev(const c_parser* self);
extern bool c_parser_at(const c_parser* self, c_token_kind k);
extern bool c_parser_next_token_is(c_parser* self, c_token_kind k);
extern bool c_parser_require(c_parser* self, c_token_kind k);
extern bool c_parser_require_ex(c_parser* self, c_token_kind k, const c_token_kind expected[]);

extern tree_location c_parser_get_loc(const c_parser* self);

#ifdef __cplusplus
}
#endif

#endif
