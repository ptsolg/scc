#ifndef SHEAP_DEBUG_H
#define SHEAP_DEBUG_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// inserts read-only page of memory after each allocated block of memory
extern void* __cdecl hd_malloc(size_t size);
extern void __cdecl hd_free(void* block);
extern void scl_enable_heap_debug();

#ifdef __cplusplus
}
#endif

#endif // !SHEAP_DEBUG_H
