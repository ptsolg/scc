#include "scc/core/misc.h"
#include <string.h> // memcpy

typedef struct
{
        uint8_t* first;
        uint8_t* last;
} memrange;

static inline memrange memrange_create(uint8_t* first, uint8_t* last)
{
        memrange r = { first, last };
        return r;
}

static inline memrange _partition(uint8_t* first, uint8_t* last, size_t obsize,
        cmp_result(*const cmp_fn)(void*, const void*, const void*), void* ex_data)
{
        uint8_t mid[SORT_MAX_OBJECT_SIZE];
        uint8_t buffer[SORT_MAX_OBJECT_SIZE];
        size_t n = (last - first) / obsize;
        memcpy(mid, first + (n / 2) * obsize, obsize);

        while (first <= last)
        {
                while (cmp_fn(ex_data, first, mid) == CR_LE)
                        first += obsize;
                while (cmp_fn(ex_data, last, mid) == CR_GR)
                        last -= obsize;
                if (first <= last)
                {
                        memcpy(buffer, first, obsize);
                        memcpy(first, last, obsize);
                        memcpy(last, buffer, obsize);
                        first += obsize;
                        last -= obsize;
                }
        }
        return memrange_create(first, last);
}

extern void sort(void* data, size_t n, size_t obsize,
        cmp_result(*const cmp_fn)(void*, const void*, const void*), void* ex_data)
{
        assert(data && n && cmp_fn && obsize < SORT_MAX_OBJECT_SIZE);

        memrange stack[128];
        size_t sp = 0;
        stack[sp++] = memrange_create((uint8_t*)data, (uint8_t*)data + (n - 1) * obsize);

        while (sp)
        {
                memrange r = stack[--sp];
                if (r.first >= r.last)
                        continue;

                memrange p = _partition(r.first, r.last, obsize, cmp_fn, ex_data);
                stack[sp++] = memrange_create(r.first, p.last);
                stack[sp++] = memrange_create(p.first, r.last);
        }

}