#ifndef C_H
#define C_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/c/source.h"

typedef struct _tree_module tree_module;

extern errcode c_lex_source(c_context* context, file_entry* source, FILE* error, ptrvec* result);
extern tree_module* c_parse_source(c_context* context, file_entry* source, FILE* error);

#ifdef __cplusplus
}
#endif

#endif