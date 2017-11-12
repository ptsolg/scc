#ifndef SSAIZER_H
#define SSAIZER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"

typedef struct _ssa_context ssa_context;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_module ssa_module;
typedef struct _tree_decl tree_decl;
typedef struct _tree_module tree_module;
typedef struct _ssa_value ssa_value;

typedef struct _ssa_block_info
{
        ssa_block* block;
        htab defs;
} ssa_block_info;

typedef struct _ssaizer
{
        ssa_context* context;
        ssa_block* block;
        htab* defs;
        dseq block_info;
} ssaizer;

extern void ssaizer_init(ssaizer* self, ssa_context* context);
extern void ssaizer_dispose(ssaizer* self);

extern void ssaizer_enter_block(ssaizer* self, ssa_block* block);
extern void ssaizer_exit_block(ssaizer* self);

extern void ssaizer_set_def(ssaizer* self, const tree_decl* var, ssa_value* val);
extern ssa_value* ssaizer_get_def(ssaizer* self, const tree_decl* var);

extern ssa_block* ssaize_function(ssaizer* self, const tree_decl* func);
extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module);

#ifdef __cplusplus
}
#endif

#endif