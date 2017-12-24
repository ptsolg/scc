#ifndef CENV_H
#define CENV_H

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

extern serrcode clex_source(
        ccontext* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error,
        dseq* result);

extern tree_module* cparse_source(
        ccontext* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error);

typedef struct _cenv
{
        csource_manager source_manager;
        clogger logger;
        clexer lexer;
        csema sema;
        cparser parser;
        ccontext* context;
} cenv;

extern void cenv_init(cenv* self, ccontext* context, file_lookup* source_lookup, FILE* err);
extern void cenv_dispose(cenv* self);

extern serrcode cenv_lex_source(cenv* self, file_entry* source, dseq* result);
extern tree_module* cenv_parse_source(cenv* self, file_entry* source);

#ifdef __cplusplus
}
#endif

#endif // !CENV_H