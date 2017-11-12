#ifndef SSA_BLOCK_H
#define SSA_BLOCK_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-branch.h"

typedef struct _ssa_context ssa_context;

typedef struct _ssa_block
{
        ssa_id _entry_id;
        ssa_branch* _exit;
        dseq _values;
} ssa_block;

extern ssa_block* ssa_new_block(ssa_context* context, ssa_id entry_id, ssa_branch* exit);

static inline ssa_id ssa_get_block_entry_id(const ssa_block* self);
static inline ssa_branch* ssa_get_block_exit(const ssa_block* self);

static inline void ssa_set_block_entry_id(ssa_block* self, ssa_id id);
static inline void ssa_set_block_exit(ssa_block* self, ssa_branch* br);

static inline ssa_id ssa_get_block_entry_id(const ssa_block* self)
{
        return self->_entry_id;
}

static inline ssa_branch* ssa_get_block_exit(const ssa_block* self)
{
        return self->_exit;
}

static inline void ssa_set_block_entry_id(ssa_block* self, ssa_id id)
{
        self->_entry_id = id;
}

static inline void ssa_set_block_exit(ssa_block* self, ssa_branch* br)
{
        self->_exit = br;
}

#ifdef __cplusplus
}
#endif

#endif