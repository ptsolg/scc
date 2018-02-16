#ifndef TREE_COMMON_H
#define TREE_COMMON_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/scl/common.h"
#include "scc/scl/strpool.h"
#include "scc/scl/list.h"

#define TREE_INLINE inline

#define TREE_INVALID_ID STRREF_INVALID
#define TREE_INVALID_LOC ((tree_location)-1)
#define TREE_INVALID_XLOC ((suint64)-1)

typedef strref tree_id;

#define TREE_EMPTY_ID (STRREF(""))

typedef suint32 tree_location;

typedef struct _tree_xlocation
{
        union
        {
                suint64 val;
                struct
                {
                        tree_location begin;
                        tree_location end;
                };
        };
} tree_xlocation;

static tree_xlocation tree_create_xloc(tree_location begin, tree_location end)
{
        tree_xlocation x = { .begin = begin, .end = end };
        return x;
}

typedef struct _tree_array
{
        suint8* data;
        ssize size;
} tree_array;

static TREE_INLINE void tree_init_array(tree_array* self)
{
        self->data = NULL;
        self->size = 0;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_COMMON_H
