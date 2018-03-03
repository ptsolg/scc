#ifndef SSA_FUNCTION_H
#define SSA_FUNCTION_H

#ifdef HAS_PRAGMA
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
        list_node _node;
        tree_decl* _function;
        list_head _blocks;
        ssa_array _params;
} ssa_function;

extern ssa_function* ssa_new_function(ssa_context* context, tree_decl* func);
extern void ssa_add_function_block(ssa_function* self, ssa_block* block);
extern void ssa_add_function_param(ssa_function* self, ssa_context* context, ssa_value* param);
extern bool ssa_function_returns_void(const ssa_function* self);

extern void ssa_fix_function_content_uids(ssa_function* self);

static inline ssa_function* ssa_get_next_function(const ssa_function* self);
static inline tree_type* ssa_get_function_result_type(const ssa_function* self);
static inline tree_decl* ssa_get_function_entity(const ssa_function* self);
static inline ssa_block* ssa_get_function_blocks_begin(const ssa_function* self);
static inline ssa_block* ssa_get_function_blocks_end(ssa_function* self);
static inline ssa_block* ssa_get_function_blocks_cend(const ssa_function* self);
static inline ssa_value** ssa_get_function_params_begin(const ssa_function* self);
static inline ssa_value** ssa_get_function_params_end(const ssa_function* self);

#define SSA_FOREACH_FUNCTION_BLOCK(PFUNC, ITNAME)\
        for (ssa_block* ITNAME = ssa_get_function_blocks_begin(PFUNC);\
                ITNAME != ssa_get_function_blocks_cend(PFUNC);\
                ITNAME = ssa_get_next_block(ITNAME))

#define SSA_FOREACH_FUNCTION_PARAM(PFUNC, ITNAME) \
        for (ssa_value** ITNAME = ssa_get_function_params_begin(PFUNC); \
                ITNAME != ssa_get_function_params_end(PFUNC); ITNAME++)

static inline ssa_function* ssa_get_next_function(const ssa_function* self)
{
        return (ssa_function*)list_node_next(&self->_node);
}

static inline tree_type* ssa_get_function_result_type(const ssa_function* self)
{
        return tree_get_function_type_result(
                tree_get_decl_type(ssa_get_function_entity(self)));
}

static inline tree_decl* ssa_get_function_entity(const ssa_function* self)
{
        return self->_function;
}

static inline ssa_block* ssa_get_function_blocks_begin(const ssa_function* self)
{
        return (ssa_block*)list_begin(&self->_blocks);
}

static inline ssa_block* ssa_get_function_blocks_end(ssa_function* self)
{
        return (ssa_block*)list_end(&self->_blocks);
}

static inline ssa_block* ssa_get_function_blocks_cend(const ssa_function* self)
{
        return (ssa_block*)list_cend(&self->_blocks);
}

static inline ssa_value** ssa_get_function_params_begin(const ssa_function* self)
{
        return (ssa_value**)self->_params.data;
}

static inline ssa_value** ssa_get_function_params_end(const ssa_function* self)
{
        return ssa_get_function_params_begin(self) + self->_params.size;
}

static inline void ssa_set_function_entity(ssa_function* self, tree_decl* func)
{
        self->_function = func;
}

#ifdef __cplusplus
}
#endif

#endif