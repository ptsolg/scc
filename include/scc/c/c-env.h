#ifndef C_ENV_H
#define C_ENV_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/c/c-lexer.h"
#include "scc/c/c-error.h"
#include "scc/c/c-sema.h"
#include "scc/c/c-parser.h"

extern serrcode c_lex_source(
        c_context* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error,
        dseq* result);

extern tree_module* c_parse_source(
        c_context* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error);

typedef struct _c_env
{
        c_source_manager source_manager;
        c_logger logger;
        c_lexer lexer;
        c_sema sema;
        c_parser parser;
        c_context* context;
} c_env;

extern void c_env_init(c_env* self, c_context* context, file_lookup* source_lookup, FILE* err);
extern void c_env_dispose(c_env* self);

extern serrcode c_env_lex_source(c_env* self, file_entry* source, dseq* result);
extern tree_module* c_env_parse_source(c_env* self, file_entry* source);

#ifdef __cplusplus
}
#endif

#endif