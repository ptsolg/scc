#ifndef SCC_CORE_HTAB_H
#define SCC_CORE_HTAB_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "hash.h"
#include "string.h"

typedef uint32_t strref;

#define STRREF_EMPTY ((strref)-1)
#define STRREF_DELETED ((strref)-2)
#define STRREF_INVALID STRREF_EMPTY

#define STRREF_IS_INVALID(R) ((R) == STRREF_EMPTY || (R) == STRREF_DELETED)

#ifndef STRREFL
#define _STRREF_HASH_SEED 391585969
#define STRREFL(S, L) ((strref)murmurhash3_86_32((S), _STRREF_HASH_SEED, (int)(L)))
#endif

#define STRREF(S) STRREFL(S, strlen(S) + 1)

#define HTAB_FN(N) strmap_##N
#define HTAB_TP    strmap
#define HTAB_ETP   strmap_entry
#define HTAB_KTP   strref
#define HTAB_EK    STRREF_EMPTY
#define HTAB_DK    STRREF_DELETED
#define HTAB_VTP   void*
#include "htab-type.h"

#define STRMAP_FOREACH(PMAP, IT) \
        HTAB_FOREACH(PMAP, strmap_entry, IT, strmap_first_entry, strmap_next_entry)

#define HTAB_FN(N) ptrset_##N
#define HTAB_TP    ptrset
#define HTAB_ETP   ptrset_entry
#define HTAB_KTP   const void*
#define HTAB_EK    ((const void*)0)
#define HTAB_DK    ((const void*)1)
#include "htab-type.h"

#define HTAB_FN(N) ptrmap_##N
#define HTAB_TP    ptrmap
#define HTAB_ETP   ptrmap_entry
#define HTAB_KTP   const void*
#define HTAB_EK    ((const void*)0)
#define HTAB_DK    ((const void*)1)
#define HTAB_VTP   void*
#include "htab-type.h"

#ifdef __cplusplus
}
#endif

#endif