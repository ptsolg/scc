#ifndef SSA_FUNCTION_H
#define SSA_FUNCTION_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-block.h"

typedef struct _tree_decl tree_decl;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_block ssa_block;

typedef struct _ssa_function
{
        tree_decl* _function;
        list_head _blocks;
} ssa_function;

extern ssa_function* ssa_new_function(ssa_context* context, tree_decl* func);
extern void ssa_add_function_block(ssa_function* self, ssa_block* block);

extern void ssa_fix_function_content_uids(ssa_function* self);

static inline tree_decl* ssa_get_function_entity(const ssa_function* self);
static inline ssa_block* ssa_get_function_begin(const ssa_function* self);
static inline ssa_block* ssa_get_function_end(ssa_function* self);
static inline ssa_block* ssa_get_function_cend(const ssa_function* self);

#define SSA_FOREACH_FUNCTION_BLOCK(PFUNC, ITNAME)\
        for (ssa_block* ITNAME = ssa_get_function_begin(PFUNC);\
                ITNAME != ssa_get_function_cend(PFUNC);\
                ITNAME = ssa_get_next_block(ITNAME))

static inline void ssa_set_function_entity(ssa_function* self, tree_decl* func);

static inline tree_decl* ssa_get_function_entity(const ssa_function* self)
{
        return self->_function;
}

static inline ssa_block* ssa_get_function_begin(const ssa_function* self)
{
        return (ssa_block*)list_begin(&self->_blocks);
}

static inline ssa_block* ssa_get_function_end(ssa_function* self)
{
        return (ssa_block*)list_end(&self->_blocks);
}

static inline ssa_block* ssa_get_function_cend(const ssa_function* self)
{
        return (ssa_block*)list_cend(&self->_blocks);
}

static inline void ssa_set_function_entity(ssa_function* self, tree_decl* func)
{
        self->_function = func;
}

#ifdef __cplusplus
}
#endif

#endif