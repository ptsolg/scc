#ifndef TREE_TARGET_H
#define TREE_TARGET_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-common.h"
#include "tree-type.h"

typedef enum
{
        TTARGET_X32,
        TTARGET_X64,
} tree_target_kind;

typedef struct _tree_target_info
{
        tree_target_kind _kind;
        suint8 _pointer_size;
        suint8 _pointer_align;
        suint8 _builtin_size[TBTK_SIZE];
        suint8 _builtin_align[TBTK_SIZE];
} tree_target_info;

extern void tree_init_target_info(tree_target_info* self, tree_target_kind k);

extern tree_target_kind tree_get_target_kind(const tree_target_info* self);

extern bool tree_target_is(const tree_target_info* self, tree_target_kind k);

extern ssize tree_get_pointer_size(const tree_target_info* self);
extern ssize tree_get_pointer_align(const tree_target_info* self);
extern ssize tree_get_builtin_type_size(const tree_target_info* self, tree_builtin_type_kind k);
extern ssize tree_get_builtin_type_align(const tree_target_info* self, tree_builtin_type_kind k);

extern ssize tree_get_sizeof(const tree_target_info* info, const tree_type* t);
extern ssize tree_get_alignof(const tree_target_info* info, const tree_type* t);

#ifdef __cplusplus
}
#endif

#endif // !TREE_TARGET_H
