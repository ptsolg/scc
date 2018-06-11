#ifndef SCC_CORE_STRPOOL_H
#define SCC_CORE_STRPOOL_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "htab.h"

typedef struct _strentry
{
        size_t size;
        const uint8_t* data;
} strentry;

// a set of unique strings that can be accessed through strref
typedef struct
{
        strmap map;
        obstack string_alloc;
} strpool;

extern void strpool_init(strpool* self);
extern void strpool_init_ex(strpool* self, allocator* alloc);
extern void strpool_dispose(strpool* self);
extern bool strpool_get(const strpool* self, strref ref, strentry* result);
extern bool strpooled(const strpool* self, strref ref);
extern strref strpool_insert(strpool* self, const void* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
