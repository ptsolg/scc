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

extern void ssa_add_block_value(ssa_block* self, ssa_value* val);

static inline ssa_id ssa_get_block_entry_id(const ssa_block* self);
static inline ssa_branch* ssa_get_block_exit(const ssa_block* self);

static inline ssa_value** ssa_get_block_begin(const ssa_block* self);
static inline ssa_value** ssa_get_block_end(const ssa_block* self);

#define SSA_FOREACH_BLOCK_VALUE(PBLOCK, ITNAME)\
        for (ssa_value** ITNAME = ssa_get_block_begin(PBLOCK);\
                ITNAME != ssa_get_block_end(PBLOCK); ITNAME++)

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

static inline ssa_value** ssa_get_block_begin(const ssa_block* self)
{
        return (ssa_value**)dseq_begin_ptr(&self->_values);
}

static inline ssa_value** ssa_get_block_end(const ssa_block* self)
{
        return (ssa_value**)dseq_end_ptr(&self->_values);
}

#ifdef __cplusplus
}
#endif

#endif