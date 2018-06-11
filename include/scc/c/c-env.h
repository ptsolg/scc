#ifndef C_ENV_H
#define C_ENV_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/c/c-lexer.h"
#include "scc/c/c-error.h"
#include "scc/c/c-sema.h"
#include "scc/c/c-parser.h"
#include "scc/c/c-source.h"

extern errcode c_lex_source(c_context* context, file_entry* source, FILE* error, ptrvec* result);
extern tree_module* c_parse_source(c_context* context, file_entry* source, FILE* error);

typedef struct _c_env
{
        c_logger logger;
        c_lexer lexer;
        c_sema sema;
        c_parser parser;
        c_context* context;
} c_env;

extern void c_env_init(c_env* self, c_context* context, FILE* err);
extern void c_env_dispose(c_env* self);

extern errcode c_env_lex_source(c_env* self, file_entry* source, ptrvec* result);
extern tree_module* c_env_parse_source(c_env* self, file_entry* source);

#ifdef __cplusplus
}
#endif

#endif