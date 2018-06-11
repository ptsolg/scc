#ifndef SCC_CORE_MALLOC_H
#define SCC_CORE_MALLOC_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef void*(*malloc_fn)(size_t);
typedef void (*free_fn)(void*);

extern malloc_fn core_malloc;
extern free_fn core_free;

extern void override_malloc(malloc_fn malloc, free_fn free);
      
#ifdef __cplusplus
}
#endif

#endif
