#ifndef TREE_CONTEXT_H
#define TREE_CONTEXT_H

#include "scc/core/allocator.h"
#include "common.h"
#include "type.h"

typedef struct _tree_target_info tree_target_info;

typedef struct _tree_context
{
        struct stack_alloc nodes;
        struct strpool strings;
        tree_target_info* target;
        tree_type builtin_types[TBTK_SIZE];
} tree_context;

extern void tree_init(tree_context* self, tree_target_info* target);
extern void tree_dispose(tree_context* self);

extern tree_type* tree_get_builtin_type(tree_context* self, tree_builtin_type_kind k);
extern tree_type* tree_get_size_type(tree_context* self);
extern tree_type* tree_get_ptrdiff_type(tree_context* self);

static TREE_INLINE void* tree_allocate_node(tree_context* self, size_t bytes)
{
        return stack_alloc(&self->nodes, bytes);
}

static TREE_INLINE struct strentry* tree_get_id_strentry(
        const tree_context* self, tree_id id)
{
        return strpool_lookup(&self->strings, id);
}

static TREE_INLINE const char* tree_get_id_string(const tree_context* self, tree_id id)
{
        struct strentry* e = tree_get_id_strentry(self, id);
        return e ? e->data : 0;
}

static TREE_INLINE tree_id tree_get_id_for_string_s(
        tree_context* self, const char* string, size_t size)
{
        return strpool_insert(&self->strings, string, size);
}

static TREE_INLINE tree_id tree_get_id_for_string(tree_context* self, const char* string)
{
        size_t strlen(const char*);
        return tree_get_id_for_string_s(self, string, strlen(string) + 1);
}

static void tree_resize_array(
        tree_context* self,
        tree_array* array,
        const size_t object_size,
        const size_t new_size)
{
        uint8_t* new_data = alloc(object_size * new_size);
        size_t num_objects = MIN(new_size, array->size);
        memcpy(new_data, array->data, array->size * object_size);
        dealloc(array->data);
        array->data = new_data;
        array->size = new_size;
}

static void tree_array_append_ptr(
        tree_context* self,
        tree_array* array,
        void* object)
{
        tree_resize_array(self, array, sizeof(void*), array->size + 1);
        *((void**)array->data + array->size - 1) = object;
}

#endif // !TREE_CONTEXT_H
