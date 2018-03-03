#ifndef TREE_CONTEXT_H
#define TREE_CONTEXT_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/memory.h"
#include "tree-common.h"
#include "tree-type.h"

typedef struct _tree_target_info tree_target_info;

typedef struct _tree_context
{
        obstack nodes;
        allocator* alloc;
        strpool strings;
        tree_target_info* target;
        tree_type builtin_types[TBTK_SIZE];
} tree_context;

extern void tree_init(tree_context* self, tree_target_info* target);
extern void tree_init_ex(tree_context* self, tree_target_info* target, allocator* alloc);
extern void tree_dispose(tree_context* self);

extern tree_type* tree_get_builtin_type(tree_context* self, tree_builtin_type_kind k);
extern tree_type* tree_get_size_type(tree_context* self);

static TREE_INLINE void* tree_allocate(tree_context* self, size_t bytes)
{
        return allocate(self->alloc, bytes);
}

static TREE_INLINE void tree_deallocate(tree_context* self, void* block)
{
        deallocate(self->alloc, block);
}

static TREE_INLINE void* tree_allocate_node(tree_context* self, size_t bytes)
{
        return obstack_allocate(&self->nodes, bytes);
}

static TREE_INLINE bool tree_get_id_strentry(
        const tree_context* self, tree_id id, strentry* result)
{
        return strpool_get(&self->strings, id, result);
}

static TREE_INLINE const char* tree_get_id_string(const tree_context* self, tree_id id)
{
        strentry entry;
        return tree_get_id_strentry(self, id, &entry)
                ? (const char*)entry.data
                : NULL;
}

static TREE_INLINE tree_id tree_get_id_for_string(
        tree_context* self, const char* string, size_t len)
{
        return strpool_insert(&self->strings, string, len);
}

static TREE_INLINE errcode tree_resize_array(
        tree_context* self,
        tree_array* array,
        const size_t object_size,
        const size_t new_size)
{
        uint8_t* new_data = tree_allocate(self, object_size * new_size);
        if (!new_data)
                return EC_ERROR;

        size_t num_objects = MIN(new_size, array->size);
        memcpy(new_data, array->data, array->size * object_size);
        tree_deallocate(self, array->data);
        array->data = new_data;
        array->size = new_size;
        return EC_NO_ERROR;
}

static TREE_INLINE errcode tree_array_append_ptr(
        tree_context* self,
        tree_array* array,
        void* object)
{
        if (EC_FAILED(tree_resize_array(self, array, sizeof(void*), array->size + 1)))
                return EC_ERROR;

        *((void**)array->data + array->size - 1) = object;
        return EC_NO_ERROR;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_CONTEXT_H
