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
#include "ssa-value.h"

typedef struct _ssa_context ssa_context;

typedef struct _ssa_block
{
        list_node _node;
        ssa_label _entry;
        ssa_branch* _exit;
        list_head _instr_list;
} ssa_block;

extern ssa_block* ssa_new_block(ssa_context* context, ssa_id entry_id, ssa_branch* exit);

extern void ssa_add_block_instr(ssa_block* self, ssa_instr* i);

extern ssa_instr* ssa_block_get_first_phi(const ssa_block* self);

static inline ssa_value* ssa_get_block_label(ssa_block* self);
static inline const ssa_value* ssa_get_block_clabel(const ssa_block* self);

static inline ssa_branch* ssa_get_block_exit(const ssa_block* self);

static inline ssa_instr* ssa_get_block_instrs_begin(const ssa_block* self);
static inline ssa_instr* ssa_get_block_instrs_end(ssa_block* self);
static inline const ssa_instr* ssa_get_block_instrs_cend(const ssa_block* self);

static inline ssa_block* ssa_get_next_block(const ssa_block* self);
static inline ssa_block* ssa_get_prev_block(const ssa_block* self);

static inline void ssa_set_block_exit(ssa_block* self, ssa_branch* exit);

#define SSA_FOREACH_BLOCK_INSTR(PBLOCK, ITNAME)\
        for (ssa_instr* ITNAME = ssa_get_block_instrs_begin(PBLOCK);\
                ITNAME != ssa_get_block_instrs_cend(PBLOCK);\
                ITNAME = ssa_get_next_instr(ITNAME))

static inline ssa_value* ssa_get_block_label(ssa_block* self)
{
        return (ssa_value*)&self->_entry;
}

static inline const ssa_value* ssa_get_block_clabel(const ssa_block* self)
{
        return (const ssa_value*)&self->_entry;
}

static inline ssa_branch* ssa_get_block_exit(const ssa_block* self)
{
        return self->_exit;
}

static inline ssa_instr* ssa_get_block_instrs_begin(const ssa_block* self)
{
        return (ssa_instr*)list_begin(&self->_instr_list);
}

static inline ssa_instr* ssa_get_block_instrs_end(ssa_block* self)
{
        return (ssa_instr*)list_end(&self->_instr_list);
}

static inline const ssa_instr* ssa_get_block_instrs_cend(const ssa_block* self)
{
        return (const ssa_instr*)list_cend(&self->_instr_list);
}

static inline ssa_block* ssa_get_next_block(const ssa_block* self)
{
        return (ssa_block*)list_node_next(&self->_node);
}

static inline ssa_block* ssa_get_prev_block(const ssa_block* self)
{
        return (ssa_block*)list_node_prev(&self->_node);
}

static inline void ssa_set_block_exit(ssa_block* self, ssa_branch* exit)
{
        self->_exit = exit;
}

#ifdef __cplusplus
}
#endif

#endif