#ifndef SSTRING_H
#define SSTRING_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "hash.h"
#include "htab-ext.h"
#include <string.h>

typedef hval strref;

// a set of unique strings that can be accessed through strref
typedef struct
{
        htab _strings;
        bump_ptr_allocator _alloc;
} strpool;

#define STRREF_INVALID ((strref)-1)

#ifndef STRREFL
#define _STRREF_HASH_SEED 391585969
#define STRREFL(S, L) ((strref)murmurhash3_86_32((S), _STRREF_HASH_SEED, (int)(L)))
#endif

#define STRREF(S) STRREFL(S, strlen(S))

extern void strpool_init(strpool* self);
extern void strpool_init_ex(strpool* self, allocator* alloc);
extern void strpool_dispose(strpool* self);
extern const char* strpool_get(const strpool* self, strref ref);
extern bool strpooled(const strpool* self, strref ref);
extern strref strpool_insert(strpool* self, const char* string);
extern strref strpool_insertl(strpool* self, const char* string, ssize len);

extern ssize sstrlen(const char* string);
extern bool sstreq(const char* a, const char* b, const char* ignore);
extern char* sstrprecat(char* string, const char* other);
extern char* sstrcatn(char* string, ssize n, ...);
extern char* sstrprecatn(char* string, ssize n, ...);
extern char* sstrwrap(const char* prefix, char* string, const char* suffix);
extern char* sstrend(char* string);
extern const char* scstrend(const char* string);
extern char* sstrfill(char* string, int v, ssize n);

#ifdef __cplusplus
}
#endif

#endif // !SSTRING_H
