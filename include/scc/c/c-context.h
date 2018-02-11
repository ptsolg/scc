#ifndef CCONTEXT_H
#define CCONTEXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree.h"
#include <stdio.h>
#include <setjmp.h>

typedef struct _csource csource;
typedef struct _file_entry file_entry;

typedef struct _ccontext
{
        base_allocator base_alloc;
        bump_ptr_allocator node_alloc;
        list_head sources;
        bool use_tags;
        tree_context* tree;
} ccontext;

extern void cinit(ccontext* self, tree_context* tree, jmp_buf on_bad_alloc);
extern void cinit_ex(ccontext* self,
        tree_context* tree, jmp_buf on_bad_alloc, allocator* alloc);

extern void cdispose(ccontext* self);

extern csource* csource_new(ccontext* context, file_entry* entry);
extern void csource_delete(ccontext* context, csource* source);

extern hval ctree_id_to_key(const ccontext* self, tree_id id, bool is_tag);
extern hval cget_decl_key(const ccontext* self, const tree_decl* decl);

static inline tree_context* cget_tree(const ccontext* self)
{
        return self->tree;
}

static inline allocator* cget_alloc(ccontext* self)
{
        return base_allocator_base(&self->base_alloc);
}

static inline void* callocate(ccontext* self, ssize bytes)
{
        return bump_ptr_allocate(&self->node_alloc, bytes);
}

#ifdef __cplusplus
}
#endif

#endif // !CCONTEXT_H
