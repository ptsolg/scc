#ifndef C_PREPROCESSOR_H
#define C_PREPROCESSOR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-preprocessor-lexer.h"

typedef struct _c_macro c_macro;

typedef struct
{
        c_preprocessor_lexer lexer;
        c_source* source;
        bool hash_expected;
        bool in_directive;
} c_preprocessor_state;

typedef struct _c_preprocessor
{
        dseq macro_expansion;
        c_preprocessor_state* state;
        c_preprocessor_state files[C_MAX_INCLUDE_NESTING];
        strmap macro_lookup;
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
extern c_macro* c_preprocessor_get_macro(const c_preprocessor* self, tree_id name);
extern bool c_preprocessor_macro_defined(const c_preprocessor* self, tree_id name);
extern void c_preprocessor_undef(const c_preprocessor* self, tree_id name);
extern c_token* c_preprocess(c_preprocessor* self);

#ifdef __cplusplus
}
#endif

#endif
