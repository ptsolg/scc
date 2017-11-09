#include "scc/scl/heap-debug.h"
#include "scc/scl/malloc.h"

#if S_WIN
#include <Windows.h>
#include <math.h>
#define PAGE_SIZE 4096

extern void* __cdecl hd_malloc(ssize size)
{
        ssize available = (ssize)ceil((double)size / (double)PAGE_SIZE) * PAGE_SIZE;
        ssize protected_ = PAGE_SIZE;
        DWORD old;

        char* p = VirtualAlloc(NULL, available + protected_, MEM_COMMIT, PAGE_READWRITE);
        if (!p)
                return NULL;
        VirtualProtect(p + available, protected_, PAGE_READONLY, &old);
        return p + available - size;
}

extern void __cdecl hd_free(void* block)
{
        VirtualFree(block, 0, MEM_RELEASE);
}

#else

#include "malloc.h"

extern void* __cdecl hd_malloc(size_t size)
{
        return smalloc(size);
}

extern void __cdecl hd_free(void* block)
{
        sfree(block);
}

#endif

extern void scl_enable_heap_debug()
{
        scl_override_malloc(&hd_malloc, &hd_free);
}