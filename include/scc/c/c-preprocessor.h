#ifndef C_PREPROCESSOR_H
#define C_PREPROCESSOR_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/dseq-common.h"
#include "c-preprocessor-lexer.h"

typedef struct _c_macro c_macro;
typedef struct _dseq dseq;

typedef enum
{
        CBMK_DATE,                 // __DATE__
        CBMK_FILE,                 // __FILE__
        CBMK_LINE,                 // __LINE__
        CBMK_STDC,                 // __STDC__
        CBMK_STDC_HOSTED,          // __STDC_HOSTED__
        CBMK_STDC_MB_MIGHT_NEQ_WC, // __STDC_MB_MIGHT_NEQ_WC_
        CBMK_STDC_VERSION,         // __STDC_VERSION__
        CBMK_TIME,                 // __TIME__

        CBMK_SCC,                  // __SCC__

        CBMK_SIZE,
} c_builtin_macro_kind;

typedef tree_id c_builtin_macro_names[CBMK_SIZE];

typedef struct _c_preprocessor
{
        c_preprocessor_lexer* lexer;
        c_preprocessor_lexer_stack lexer_stack;
        c_builtin_macro_names builtin_macro_names;
        strmap macro_lookup;
        const c_reswords* reswords;
        c_source_manager* source_manager;
        c_logger* logger;
        c_context* context;

        struct
        {
                c_token* next_unexpanded_token;
                c_token* next_expanded_token;
        } lookahead;
} c_preprocessor;

extern void c_preprocessor_init(
        c_preprocessor* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern void c_preprocessor_dispose(c_preprocessor* self);
extern errcode c_preprocessor_enter_source(c_preprocessor* self, c_source* source);
extern void c_preprocessor_exit(c_preprocessor* self);
extern c_macro* c_preprocessor_get_macro(const c_preprocessor* self, tree_id name);
extern bool c_preprocessor_macro_defined(const c_preprocessor* self, tree_id name);
extern bool c_preprocessor_undef(c_preprocessor* self, tree_id name);

// returns non-comment token
extern c_token* c_preprocess_comment(c_preprocessor* self);
// returns non-white-space token
extern c_token* c_preprocess_wspace(c_preprocessor* self, bool skip_eol);
extern c_token* c_preprocess_directive(c_preprocessor* self);
extern c_token* c_preprocess_macro(c_preprocessor* self);
extern c_token* c_preprocess(c_preprocessor* self);

#ifdef __cplusplus
}
#endif

#endif
