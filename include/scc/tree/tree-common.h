#ifndef TREE_COMMON_H
#define TREE_COMMON_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/common.h"
#include "scc/core/strpool.h"
#include "scc/core/list.h"

#define TREE_INLINE inline

#define TREE_INVALID_ID STRREF_INVALID
#define TREE_INVALID_LOC ((tree_location)-1)
#define TREE_INVALID_XLOC ((uint64_t)-1)

typedef strref tree_id;

#define TREE_EMPTY_ID (STRREF(""))

typedef uint32_t tree_location;

typedef struct _tree_xlocation
{
        union
        {
                uint64_t val;
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
        uint8_t* data;
        size_t size;
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
