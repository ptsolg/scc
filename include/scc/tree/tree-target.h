#ifndef TREE_TARGET_H
#define TREE_TARGET_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-common.h"
#include "tree-type.h"

typedef enum
{
        TTAK_X86_32,
        TTAK_X86_64,
} tree_target_architecture_kind;

typedef struct _tree_target_info
{
        tree_target_architecture_kind _kind;
        uint8_t _pointer_size;
        uint8_t _pointer_align;
        uint8_t _builtin_size[TBTK_SIZE];
        uint8_t _builtin_align[TBTK_SIZE];
} tree_target_info;

extern void tree_init_target_info(tree_target_info* self, tree_target_architecture_kind k);

extern tree_target_architecture_kind tree_get_target_kind(const tree_target_info* self);

extern bool tree_target_is(const tree_target_info* self, tree_target_architecture_kind k);

extern size_t tree_get_pointer_size(const tree_target_info* self);
extern size_t tree_get_pointer_align(const tree_target_info* self);
extern size_t tree_get_builtin_type_size(const tree_target_info* self, tree_builtin_type_kind k);
extern size_t tree_get_builtin_type_align(const tree_target_info* self, tree_builtin_type_kind k);

extern size_t tree_get_sizeof(const tree_target_info* info, const tree_type* t);
extern size_t tree_get_alignof(const tree_target_info* info, const tree_type* t);

#ifdef __cplusplus
}
#endif

#endif // !TREE_TARGET_H
