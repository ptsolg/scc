#ifndef SSTRING_H
#define SSTRING_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "strmap.h"

typedef struct _strentry
{
        ssize size;
        const suint8* data;
} strentry;

// a set of unique strings that can be accessed through strref
typedef struct
{
        strmap _map;
        obstack _string_alloc;
} strpool;

extern void strpool_init(strpool* self);
extern void strpool_init_ex(strpool* self, allocator* alloc);
extern void strpool_dispose(strpool* self);
extern bool strpool_get(const strpool* self, strref ref, strentry* result);
extern bool strpooled(const strpool* self, strref ref);
extern strref strpool_insert(strpool* self, const void* data, ssize size);

#ifdef __cplusplus
}
#endif

#endif // !SSTRING_H
