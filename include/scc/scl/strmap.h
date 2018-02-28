#ifndef SSTRMAP_H
#define SSTRMAP_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "hash.h"
#include "string.h"

typedef suint32 strref;

#define STRREF_INVALID ((strref)-1)

#ifndef STRREFL
#define _STRREF_HASH_SEED 391585969
#define STRREFL(S, L) ((strref)murmurhash3_86_32((S), _STRREF_HASH_SEED, (int)(L)))
#endif

#define STRREF(S) STRREFL(S, strlen(S) + 1)

#define STRMAP_EMPTY STRREF_INVALID
#define STRMAP_DELETED ((strref)-2)

#define STRMAP_FOREACH(PMAP, ITNAME) \
        for (strmap_iter ITNAME = strmap_iter_create(PMAP); \
                strmap_iter_valid(&ITNAME); strmap_iter_advance(&ITNAME))

#define HTAB_TYPE strmap
#define HTAB_IMPL_FN_GENERATOR(NAME) _strmap_##NAME
#define HTAB_KEY_TYPE suint32
#define HTAB_DELETED_KEY STRMAP_DELETED
#define HTAB_EMPTY_KEY STRMAP_EMPTY
#define HTAB_VALUE_TYPE void*
#define HTAB_INIT strmap_init
#define HTAB_INIT_ALLOC strmap_init_alloc
#define HTAB_DISPOSE strmap_dispose
#define HTAB_GET_SIZE strmap_size
#define HTAB_GET_ALLOCATOR strmap_alloc
#define HTAB_RESERVE strmap_reserve
#define HTAB_CLEAR strmap_clear
#define HTAB_ERASE strmap_erase
#define HTAB_GROW strmap_grow
#define HTAB_INSERT strmap_insert
#define HTAB_FIND strmap_find

#define HTAB_ITERATOR_TYPE strmap_iter
#define HTAB_ITERATOR_GET_KEY strmap_iter_key
#define HTAB_ITERATOR_ADVANCE strmap_iter_advance
#define HTAB_ITERATOR_INIT strmap_iter_init
#define HTAB_ITERATOR_CREATE strmap_iter_create
#define HTAB_ITERATOR_IS_VALID strmap_iter_valid
#define HTAB_ITERATOR_GET_VALUE strmap_iter_value

#include "htab.h"

#undef HTAB_TYPE
#undef HTAB_IMPL_FN_GENERATOR
#undef HTAB_KEY_TYPE 
#undef HTAB_DELETED_KEY
#undef HTAB_EMPTY_KEY
#undef HTAB_VALUE_TYPE
#undef HTAB_INIT
#undef HTAB_INIT_ALLOC
#undef HTAB_DISPOSE
#undef HTAB_GET_SIZE
#undef HTAB_GET_ALLOCATOR
#undef HTAB_RESERVE
#undef HTAB_CLEAR
#undef HTAB_ERASE
#undef HTAB_GROW
#undef HTAB_INSERT
#undef HTAB_FIND
#undef HTAB_ITERATOR_TYPE
#undef HTAB_ITERATOR_GET_KEY
#undef HTAB_ITERATOR_ADVANCE
#undef HTAB_ITERATOR_INIT
#undef HTAB_ITERATOR_CREATE
#undef HTAB_ITERATOR_IS_VALID
#undef HTAB_ITERATOR_GET_VALUE

#ifdef __cplusplus
}
#endif

#endif