#ifndef SCC_CORE_MALLOC_H
#define SCC_CORE_MALLOC_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef void*(*malloc_fn)(size_t);
typedef void (*free_fn)(void*);

extern malloc_fn smalloc;
extern free_fn sfree;

extern void scl_override_malloc(malloc_fn malloc, free_fn free);
      
#ifdef __cplusplus
}
#endif

#endif
