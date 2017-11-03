#ifndef TREE_COMMON_H
#define TREE_COMMON_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libscl/common.h>
#include <libscl/sstring.h>
#include <libscl/list.h>
#include <libscl/objgroup.h>

#define TREE_INVALID_ID STRREF_INVALID
#define TREE_INVALID_LOC ((tree_location)-1)
#define TREE_INVALID_XLOC ((tree_xlocation)-1)

typedef strref tree_id;
typedef suint32 tree_location;
// extended location wich stores begin and end
typedef suint64 tree_xlocation;
typedef struct _tree_context tree_context;

extern tree_xlocation tree_init_xloc(tree_location begin, tree_location end);

typedef union
{
        tree_xlocation val;
        struct
        {
                tree_location begin;
                tree_location end;
        };
} _tree_xlocation;

static inline tree_location tree_get_xloc_begin(tree_xlocation self)
{
        _tree_xlocation l = { self };
        return l.begin;
}

static inline tree_location tree_get_xloc_end(tree_xlocation self)
{
        _tree_xlocation l = { self };
        return l.end;
}

static inline tree_xlocation tree_set_xloc_begin(tree_xlocation self, tree_location begin)
{
        _tree_xlocation l = { self };
        l.begin = begin;
        return l.val;
}
static inline tree_xlocation tree_set_xloc_end(tree_xlocation self, tree_location end)
{
        _tree_xlocation l = { self };
        l.end = end;
        return l.val;
}

extern tree_id tree_get_empty_id();
extern bool tree_id_is_empty(tree_id id);

#ifdef __cplusplus
}
#endif

#endif // !TREE_COMMON_H
