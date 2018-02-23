#ifndef C_PREPROCESSOR_H
#define C_PREPROCESSOR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-preprocessor-lexer.h"

typedef struct
{
        c_pplexer lexer;
        c_source* source;
        int nhash;
        bool hash_expected;
        bool in_directive;
} c_preprocessor_state;

typedef struct _c_preprocessor
{
        dseq expansion;
        c_preprocessor_state* state;
        c_preprocessor_state files[C_MAX_INCLUDE_NESTING];
        const c_reswords* reswords;
        c_source_manager* source_manager;
        c_logger* logger;
        c_context* context;
} c_preprocessor;

extern void c_preprocessor_init(
        c_preprocessor* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern void c_preprocessor_dispose(c_preprocessor* self);
extern serrcode c_preprocessor_enter(c_preprocessor* self, c_source* source);
extern void c_preprocessor_exit(c_preprocessor* self);
extern c_token* c_preprocess(c_preprocessor* self);

#ifdef __cplusplus
}
#endif

#endif
