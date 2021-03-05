#ifndef SSA_EMIT_H
#define SSA_EMIT_H

typedef struct _ssa_module ssa_module;
typedef struct _ssa_context ssa_context;
typedef struct _tree_module tree_module;
typedef struct _ssa_optimizer_opts ssa_optimizer_opts;

typedef struct
{
        const tree_module* tm;
} ssa_implicitl_modules;

extern ssa_module* ssa_emit_module(
        ssa_context* context,
        const tree_module* module,
        const ssa_optimizer_opts* opts,
        const ssa_implicitl_modules* ext);

#endif