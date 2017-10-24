#ifndef SCC_ENV_H
#define SCC_ENV_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdio.h>
#include <libscl/objgroup.h>
#include <libscl/arg-parser.h>

typedef enum
{
        SRM_SYNTAX_ONLY,
        SRM_LEX_ONLY,
} scc_run_mode;

typedef struct _scc_env
{
        scc_run_mode mode;
        aparser      parser;
        objgroup     handlers;
        objgroup     input;
        objgroup     include;
        const char*  output;
        FILE*        err;
        serrcode     return_code;

        struct
        {
                bool x32;
                bool print_exp_value;
                bool print_exp_type;
                bool print_impl_casts;
                bool force_brackets;
                int  float_precision;
                int  double_precision;
        } opts;
} scc_env;

extern serrcode scc_init(scc_env* self, FILE* err, int argc, const char** argv);
extern serrcode scc_run(scc_env* self);
extern void     scc_dispose(scc_env* self);

extern serrcode scc_add_lookup_directory(scc_env* self, const char* dir);
extern serrcode scc_add_input(scc_env* self, const char* file);

#ifdef __cplusplus
}
#endif

#endif // !SCC_ENV_H