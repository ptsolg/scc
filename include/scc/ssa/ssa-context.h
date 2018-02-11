#ifndef SSA_CONTEXT_H
#define SSA_CONTEXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include <setjmp.h>

typedef struct _tree_target_info tree_target_info;
typedef struct _tree_context tree_context;
typedef struct _tree_type tree_type;

typedef struct _ssa_context
{
        obstack nodes;
        base_allocator base_alloc;

        tree_context* tree;
        const tree_target_info* target;
} ssa_context;

extern void ssa_init(ssa_context* self, tree_context* context, jmp_buf on_out_of_mem);

extern void ssa_init_ex(ssa_context* self,
        tree_context* context, jmp_buf on_out_of_mem, allocator* alloc);

extern void ssa_dispose(ssa_context* self);
extern tree_type* ssa_get_type_for_label(ssa_context* self);

static inline void* ssa_allocate(ssa_context* self, ssize bytes)
{
        return obstack_allocate(&self->nodes, bytes);
}

static inline allocator* ssa_get_alloc(ssa_context* self)
{
        return base_allocator_base(&self->base_alloc);
}

static inline const tree_target_info* ssa_get_target(const ssa_context* self)
{
        return self->target;
}

static inline tree_context* ssa_get_tree(const ssa_context* self)
{
        return self->tree;
}

#ifdef __cplusplus
}
#endif

#endif