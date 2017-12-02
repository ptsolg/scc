#include "scc/scl/misc.h"
#include <string.h> // memcpy

typedef struct
{
        suint8* first;
        suint8* last;
} memrange;

static inline memrange memrange_create(suint8* first, suint8* last)
{
        memrange r = { first, last };
        return r;
}

static inline memrange _partition(suint8* first, suint8* last, ssize obsize,
        cmp_result(*const cmp_fn)(void*, const void*, const void*), void* ex_data)
{
        suint8 mid[SSORT_MAX_OBJECT_SIZE];
        suint8 buffer[SSORT_MAX_OBJECT_SIZE];
        ssize n = (last - first) / obsize;
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

extern void ssort(void* data, ssize n, ssize obsize,
        cmp_result(*const cmp_fn)(void*, const void*, const void*), void* ex_data)
{
        S_ASSERT(data && n && cmp_fn && obsize < SSORT_MAX_OBJECT_SIZE);

        memrange stack[128];
        ssize sp = 0;
        stack[sp++] = memrange_create((suint8*)data, (suint8*)data + (n - 1) * obsize);

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