#ifndef CODEGEN_H
#define CODEGEN_H

#include "scc/core/read-write.h"
#include "scc/core/error.h"
#include "scc/ssa/ssa-opt.h"

typedef struct _ssa_context ssa_context;
typedef struct _tree_module tree_module;

typedef enum
{
        CGOK_SSA,
        CGOK_LLVM,
} codegen_output_kind;

extern errcode codegen_module(
        write_cb* write,
        ssa_context* context,
        const tree_module* module,
        const tree_module* tm_decls,
        codegen_output_kind output,
        const ssa_optimizer_opts* opts);

#endif