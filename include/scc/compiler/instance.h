#ifndef SCC_INSTANCE_H
#define SCC_INSTANCE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdio.h>
#include "scc/scl/dseq.h"
#include "scc/scl/arg-parser.h"

typedef enum
{
        SRM_DEFAULT,
        SRM_SYNTAX_ONLY,
        SRM_LEX_ONLY,
} scc_run_mode;

typedef struct _scc_instance
{
        scc_run_mode mode;
        aparser parser;
        dseq handlers;
        dseq input;
        dseq include;
        const char* output;
        FILE* err;
        serrcode return_code;

        struct
        {
                bool x32;
                bool print_expr_value;
                bool print_expr_type;
                bool print_impl_casts;
                bool print_eval_result;
                bool force_brackets;
                bool emit_ssa;
                int float_precision;
                int double_precision;
        } opts;
} scc_instance;

extern serrcode scc_init(scc_instance* self, FILE* err, int argc, const char** argv);
extern serrcode scc_run(scc_instance* self);
extern void scc_dispose(scc_instance* self);

extern serrcode scc_add_lookup_directory(scc_instance* self, const char* dir);
extern serrcode scc_add_input(scc_instance* self, const char* file);

#ifdef __cplusplus
}
#endif

#endif // !SCC_INSTANCE_H
