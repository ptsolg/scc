#ifndef TREE_COMMON_H
#define TREE_COMMON_H

#include "scc/core/common.h"
#include "scc/core/strpool.h"
#include "scc/core/hash.h"
#include "scc/core/list.h"

#define TREE_INLINE inline
#define TREE_INVALID_ID HASHMAP_DEL_KEY
#define TREE_INVALID_LOC ((tree_location)UINT32_MAX)
#define TREE_MAX_LOC (TREE_INVALID_LOC - 1)
#define TREE_INVALID_XLOC ((uint64_t)UINT64_MAX)

typedef unsigned tree_id;

#define TREE_EMPTY_ID 0

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

#endif // !TREE_COMMON_H
