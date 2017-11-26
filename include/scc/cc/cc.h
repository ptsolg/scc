#ifndef SCC_CC_H
#define SCC_CC_H

#include "scc/c/c-tree.h"
#include "scc/c/c-source.h"
#include "scc/c/c-error.h"
#include "scc/ssa/ssa-context.h"
#include <stdio.h>
#include <setjmp.h>

typedef enum
{
        SCTK_32,
        SCTK_64,
} scc_cc_target_kind;

typedef enum
{
        SCRM_DEFAULT,
        SCRM_LEX_ONLY,
        SCRM_SYNTAX_ONLY,
} scc_cc_run_mode;

typedef enum
{
        SCPF_NONE = 0,
        SCPF_EXPR_VALUE = 1,
        SCPF_EXPR_TYPE = 2,
        SCPF_IMPL_CASTS = 4,
        SCPF_EVAL_RESULT = 8,
        SCPF_FORCE_BRACKETS = 16,
} scc_cc_print_flags;

typedef struct _scc_cc_print_opts
{
        scc_cc_print_flags flags;
        int float_precision;
        int double_precision;
} scc_cc_print_opts;

typedef enum
{
        SCOK_LLVM,
        SCOK_SSA,
        SCOK_C,
} scc_cc_output_kind;

typedef struct _scc_cc_opts
{
        scc_cc_target_kind target;
        scc_cc_run_mode mode;
        scc_cc_print_opts print;
        scc_cc_output_kind output;
} scc_cc_opts;

typedef struct _scc_cc
{
        base_allocator alloc;
        scc_cc_opts opts;
        FILE* log;
        FILE* out;

        tree_target_info target;
        tree_context tree;
        ccontext c;
        csource_manager source_manager;
        cerror_manager error_manager;
        dseq sources;
        ssa_context ssa;
} scc_cc;

extern void scc_cc_init(scc_cc* self, FILE* log, jmp_buf on_fatal_error);
extern void scc_cc_dispose(scc_cc* self);

extern void scc_cc_error(scc_cc* self, const char* format, ...);
extern void scc_cc_file_doesnt_exist(scc_cc* self, const char* file);
extern void scc_cc_unable_to_open(scc_cc* self, const char* file);
extern FILE* scc_cc_open_existing_file(scc_cc* self, const char* file, const char* mode);
extern FILE* scc_cc_open_file(scc_cc* self, const char* file, const char* mode);

extern void scc_cc_set_mode(scc_cc* self, scc_cc_run_mode mode);
extern void scc_cc_set_output(scc_cc* self, FILE* out);
extern void scc_cc_set_log(scc_cc* self, FILE* log);

extern serrcode scc_cc_add_input_file(scc_cc* self, const char* file);
extern serrcode scc_cc_emulate_file(scc_cc* self, const char* filename, const char* content);
extern void scc_cc_add_lookup_dir(scc_cc* self, const char* dir);

extern serrcode scc_cc_lex(scc_cc* self);
extern serrcode scc_cc_parse(scc_cc* self);
extern serrcode scc_cc_gen(scc_cc* self);

extern serrcode scc_cc_run(scc_cc* self);

extern serrcode scc_cc_parse_opts(scc_cc* self, int argc, const char** argv);

#endif