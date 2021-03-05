#ifndef C_CONTEXT_H
#define C_CONTEXT_H

#include <setjmp.h>

#include "scc/c-common/lang-opts.h"
#include "scc/c-common/source.h"
#include "scc/core/memory.h"

typedef struct _tree_context tree_context;

typedef enum
{
        CES_WARNING,
        CES_ERROR,
        CES_FATAL_ERROR,
} c_error_severity;

#define C_MAX_ERROR_LEN 512

typedef struct _c_error_handler
{
        void(*on_error)(void*, c_error_severity, c_location, const char*);
} c_error_handler;

typedef struct _c_context
{
        mempool memory;
        obstack nodes;
        tree_context* tree;
        c_source_manager source_manager;
        c_lang_opts lang_opts;
        c_error_handler* error_handler;
        bool errors_disabled;
} c_context;

extern void c_context_init(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        c_error_handler* error_handler,
        jmp_buf on_bad_alloc);

extern void c_context_init_ex(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        c_error_handler* error_handler,
        jmp_buf on_bad_alloc,
        allocator* alloc);

extern void c_context_dispose(c_context* self);

extern void c_error(
        const c_context* self,
        c_error_severity severity,
        tree_location location,
        const char* format,
        ...);

static inline allocator* c_context_get_allocator(c_context* self)
{
        return mempool_to_allocator(&self->memory);
}

static inline void* c_context_allocate_node(c_context* self, size_t bytes)
{
        return obstack_allocate(&self->nodes, bytes);
}

static inline void* c_context_allocate(c_context* self, size_t bytes)
{
        return mempool_allocate(&self->memory, bytes);
}

static inline void c_context_deallocate(c_context* self, void* block)
{
        mempool_deallocate(&self->memory, block);
}

#endif
