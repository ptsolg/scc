#ifndef SSA_BLOCK_H
#define SSA_BLOCK_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-value.h"

typedef struct _ssa_context ssa_context;

typedef struct _ssa_block
{
        list_node _node;
        struct _ssa_label _entry;
        list_head _instr_list;
} ssa_block;

extern ssa_block* ssa_new_block(ssa_context* context, ssa_id entry_id);

extern ssa_instr* ssa_block_get_first_phi(const ssa_block* self);

static inline ssa_value* ssa_get_block_label(ssa_block* self);
static inline ssa_block* ssa_get_label_block(ssa_value* self);
static inline const ssa_value* ssa_get_block_clabel(const ssa_block* self);

static inline ssa_instr* ssa_get_block_instrs_begin(const ssa_block* self);
static inline ssa_instr* ssa_get_block_instrs_end(ssa_block* self);
static inline ssa_instr* ssa_get_block_terminator(const ssa_block* self);
static inline const ssa_instr* ssa_get_block_instrs_cend(const ssa_block* self);

static inline ssa_block* ssa_get_next_block(const ssa_block* self);
static inline ssa_block* ssa_get_prev_block(const ssa_block* self);

static inline void ssa_remove_block(ssa_block* self);

#define SSA_FOREACH_BLOCK_INSTR(PBLOCK, ITNAME)\
        for (ssa_instr* ITNAME = ssa_get_block_instrs_begin(PBLOCK);\
                ITNAME != ssa_get_block_instrs_cend(PBLOCK);\
                ITNAME = ssa_get_next_instr(ITNAME))

static inline ssa_value* ssa_get_block_label(ssa_block* self)
{
        return (ssa_value*)&self->_entry;
}

static inline ssa_block* ssa_get_label_block(ssa_value* self)
{
        return (ssa_block*)((char*)self - offsetof(ssa_block, _entry));
}

static inline const ssa_value* ssa_get_block_clabel(const ssa_block* self)
{
        return (const ssa_value*)&self->_entry;
}

static inline ssa_instr* ssa_get_block_instrs_begin(const ssa_block* self)
{
        return (ssa_instr*)list_begin(&self->_instr_list);
}

static inline ssa_instr* ssa_get_block_instrs_end(ssa_block* self)
{
        return (ssa_instr*)list_end(&self->_instr_list);
}

static inline ssa_instr* ssa_get_block_terminator(const ssa_block* self)
{
        return list_empty(&self->_instr_list)
                ? NULL
                : (ssa_instr*)list_last(&self->_instr_list);
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

static inline void ssa_remove_block(ssa_block* self)
{
        assert(!ssa_value_is_used(ssa_get_block_clabel(self))
                && "Cannot remove used block.");

        list_node_remove(&self->_node);
        ssa_set_value_kind(ssa_get_block_label(self), SVK_INVALID);
}

#ifdef __cplusplus
}
#endif

#endif