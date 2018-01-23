#include "scc/scl/heap-debug.h"
#include "scc/scl/malloc.h"

#if S_WIN
#include <Windows.h>
#include <math.h>
#define PAGE_SIZE 4096

extern void* __cdecl hd_malloc_a(ssize size)
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

extern void __cdecl hd_free_a(void* block)
{
        VirtualFree(block, 0, MEM_RELEASE);
}

extern void* __cdecl hd_malloc_b(ssize size)
{
        ssize available = (ssize)ceil((double)size / (double)PAGE_SIZE) * PAGE_SIZE;
        ssize protected_ = PAGE_SIZE;
        DWORD old;

        char* p = VirtualAlloc(NULL, available + protected_, MEM_COMMIT, PAGE_READWRITE);
        if (!p)
                return NULL;
        VirtualProtect(p, protected_, PAGE_READONLY, &old);
        return p + protected_;
}

extern void __cdecl hd_free_b(void* block)
{
        VirtualFree(block, 0, MEM_RELEASE);
}

#else

#include "scc/scl/malloc.h"

extern void* __cdecl hd_malloc_a(size_t size)
{
        return smalloc(size);
}

extern void __cdecl hd_free_a(void* block)
{
        sfree(block);
}

extern void* __cdecl hd_malloc_b(size_t size)
{
        return smalloc(size);
}

extern void __cdecl hd_free_b(void* block)
{
        sfree(block);
}


#endif

extern void scl_enable_heap_debug(bool insert_after)
{
        if (insert_after)
                scl_override_malloc(&hd_malloc_a, &hd_free_a);
        else
                scl_override_malloc(&hd_malloc_b, &hd_free_b);
}