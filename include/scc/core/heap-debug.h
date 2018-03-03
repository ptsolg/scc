#ifndef SCC_CORE_HEAP_DEBUG_H
#define SCC_CORE_HEAP_DEBUG_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// inserts read-only page of memory after each allocated block of memory
extern void* __cdecl hd_malloc_a(size_t size);
extern void __cdecl hd_free_a(void* block);

// inserts read-only page of memory before each allocated block of memory
extern void* __cdecl hd_malloc_b(size_t size);
extern void __cdecl hd_free_b(void* block);

extern void scl_enable_heap_debug(bool insert_after);

#ifdef __cplusplus
}
#endif

#endif
