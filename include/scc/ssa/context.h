#ifndef SSA_CONTEXT_H
#define SSA_CONTEXT_H

#include "scc/core/allocator.h"
#include "common.h"
#include <setjmp.h>

typedef struct _tree_target_info tree_target_info;
typedef struct _tree_context tree_context;
typedef struct _tree_type tree_type;

typedef struct _ssa_context
{
        struct stack_alloc nodes;
        tree_context* tree;
        const tree_target_info* target;
} ssa_context;

extern void ssa_init(ssa_context* self, tree_context* context, jmp_buf on_out_of_mem);
extern void ssa_dispose(ssa_context* self);
extern tree_type* ssa_get_type_for_label(ssa_context* self);

static inline void* ssa_allocate_node(ssa_context* self, size_t bytes)
{
        return stack_alloc(&self->nodes, bytes);
}

static inline const tree_target_info* ssa_get_target(const ssa_context* self)
{
        return self->target;
}

static inline tree_context* ssa_get_tree(const ssa_context* self)
{
        return self->tree;
}

static inline errcode ssa_resize_array(
        ssa_context* self,
        ssa_array* array,
        const size_t object_size,
        const size_t new_size)
{
        uint8_t* new_data = alloc(object_size * new_size);
        if (!new_data)
                return EC_ERROR;

        size_t num_objects = new_size < array->size ? new_size : array->size;
        memcpy(new_data, array->data, num_objects * object_size);
        dealloc(array->data);
        array->data = new_data;
        array->size = new_size;
        return EC_NO_ERROR;
}

static inline void* ssa_reserve_object(ssa_context* self, ssa_array* array, const size_t object_size)
{
        if (EC_FAILED(ssa_resize_array(self, array, object_size, array->size + 1)))
                return NULL;
        return array->data + (array->size - 1) * object_size;
}

static inline void ssa_dispose_array(ssa_context* self, ssa_array* array)
{
        dealloc(array->data);
        ssa_init_array(array);
}

#endif
