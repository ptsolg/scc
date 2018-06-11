#ifndef SSA_BLOCK_H
#define SSA_BLOCK_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-value.h"
#include "ssa-instr.h" // struct _ssa_instr_node

typedef struct _ssa_context ssa_context;

typedef struct _ssa_block
{
        list_node node;
        struct _ssa_label entry;
        struct _ssa_instr_node instr_node;
        bool atomic;
} ssa_block;

extern ssa_block* ssa_new_block(ssa_context* context, bool atomic);
extern void ssa_move_block_instrs(ssa_block* from, ssa_block* to);
extern void ssa_add_block_after(ssa_block* block, ssa_block* pos);
extern void ssa_add_block_before(ssa_block* block, ssa_block* pos);

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
static inline bool ssa_block_is_atomic(ssa_block* self);

#define SSA_FOREACH_BLOCK_INSTR(PBLOCK, ITNAME)\
        for (ssa_instr* ITNAME = ssa_get_block_instrs_begin(PBLOCK);\
                ITNAME != ssa_get_block_instrs_cend(PBLOCK);\
                ITNAME = ssa_get_next_instr(ITNAME))

#define SSA_FOREACH_BLOCK_INSTR_SAFE(PBLOCK, ITNAME, NEXTNAME)\
        for (ssa_instr* ITNAME = ssa_get_block_instrs_begin(PBLOCK), *NEXTNAME;\
                (NEXTNAME = ssa_get_next_instr(ITNAME)), ITNAME != ssa_get_block_instrs_cend(PBLOCK);\
                ITNAME = NEXTNAME)

static inline ssa_value* ssa_get_block_label(ssa_block* self)
{
        return (ssa_value*)&self->entry;
}

static inline ssa_block* ssa_get_label_block(ssa_value* self)
{
        return (ssa_block*)((char*)self - offsetof(ssa_block, entry));
}

static inline const ssa_value* ssa_get_block_clabel(const ssa_block* self)
{
        return (const ssa_value*)&self->entry;
}

static inline ssa_instr* ssa_get_block_instrs_begin(const ssa_block* self)
{
        return (ssa_instr*)self->instr_node.list.head;
}

static inline ssa_instr* ssa_get_block_instrs_end(ssa_block* self)
{
        return (ssa_instr*)list_end(&self->instr_node.list);
}

static inline ssa_instr* ssa_get_block_terminator(const ssa_block* self)
{
        return list_empty(&self->instr_node.list)
                ? NULL
                : (ssa_instr*)self->instr_node.list.tail;
}

static inline const ssa_instr* ssa_get_block_instrs_cend(const ssa_block* self)
{
        return (const ssa_instr*)list_end_c(&self->instr_node.list);
}

static inline ssa_block* ssa_get_next_block(const ssa_block* self)
{
        return (ssa_block*)self->node.next;
}

static inline ssa_block* ssa_get_prev_block(const ssa_block* self)
{
        return (ssa_block*)self->node.prev;
}

static inline void ssa_remove_block(ssa_block* self)
{
        assert(!ssa_value_is_used(ssa_get_block_clabel(self))
                && "Cannot remove used block.");

        SSA_FOREACH_BLOCK_INSTR(self, instr)
                SSA_FOREACH_INSTR_OPERAND(instr, it, end)
                        _ssa_remove_value_use(it);

        SSA_FOREACH_BLOCK_INSTR(self, instr)
                ssa_remove_instr(instr); // if we fail here then one of the block' instructions is used in the other block

        list_node_remove(&self->node);
        ssa_set_value_kind(ssa_get_block_label(self), SVK_INVALID);
}

static inline bool ssa_block_is_atomic(ssa_block* self)
{
        return self->atomic;
}

#ifdef __cplusplus
}
#endif

#endif