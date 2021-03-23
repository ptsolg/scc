#include "scc/core/alloc.h"

#include <stdlib.h>
#include <stdio.h>

#include <Windows.h>
#include <math.h>
#define PAGE_SIZE 4096

void* __cdecl hd_malloc_a(size_t size)
{
        size_t available = (size_t)ceil((double)size / (double)PAGE_SIZE) * PAGE_SIZE;
        size_t protected_ = PAGE_SIZE;
        DWORD old;

        char* p = VirtualAlloc(NULL, available + protected_, MEM_COMMIT, PAGE_READWRITE);
        if (!p)
                return NULL;
        VirtualProtect(p + available, protected_, PAGE_READONLY, &old);
        return p + available - size;
}

void __cdecl hd_free_a(void* block)
{
        VirtualFree(block, 0, MEM_RELEASE);
}

void* __cdecl hd_malloc_b(size_t size)
{
        size_t available = (size_t)ceil((double)size / (double)PAGE_SIZE) * PAGE_SIZE;
        size_t protected_ = PAGE_SIZE;
        DWORD old;

        char* p = VirtualAlloc(NULL, available + protected_, MEM_COMMIT, PAGE_READWRITE);
        if (!p)
                return NULL;
        VirtualProtect(p, protected_, PAGE_READONLY, &old);
        return p + protected_;
}

void __cdecl hd_free_b(void* block)
{
        VirtualFree(block, 0, MEM_RELEASE);
}

void* alloc(size_t size)
{
        return hd_malloc_b(size);
        // void* p = malloc(size);
        // if (!p)         {
        //         fprintf(stderr, "Out of memory.");
        //         exit(-1);
        // }
        // return p;
}

void dealloc(void* block)
{
        hd_free_b(block);
        //free(block);
}
