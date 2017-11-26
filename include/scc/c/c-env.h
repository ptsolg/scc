#ifndef CENV_H
#define CENV_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-tree.h"
#include "c-source.h"
#include "c-error.h"

typedef struct _cenv
{
        ccontext context;
        csource_manager source_manager;
        cerror_manager error_manager;
        tree_target_info target;
} cenv;

extern void cenv_init(cenv* self, tree_context* tree, FILE* output, jmp_buf* fatal);
extern void cenv_dispose(cenv* self);
extern serrcode cenv_add_lookup(cenv* self, const char* dir);
extern tree_module* cparse_file(cenv* self, const char* file);
extern tree_module* cparse_string(cenv* self, const char* strt);
extern serrcode clex_file(cenv* self, const char* file, dseq* result);
extern serrcode clex_string(cenv* self, const char* str, dseq* result);

#ifdef __cplusplus
}
#endif

#endif // !CENV_H