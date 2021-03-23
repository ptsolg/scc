#ifndef HASHMAP_H
#define HASHMAP_H

#define HASHMAP_EMPTY_KEY 0x7FFFFFFFU
#define HASHMAP_DEL_KEY (HASHMAP_EMPTY_KEY - 1U)

#define HTAB hashmap
#define HTAB_K unsigned
#define HTAB_K_EMPTY HASHMAP_EMPTY_KEY
#define HTAB_K_DEL HASHMAP_DEL_KEY
#define HTAB_V void*
#include "htab.inc"

#define HASHMAP_FOREACH(HMAP, IT) \
        for (struct hashmap_iter IT = hashmap_begin(HMAP);\
                IT.pos != IT.end; hashmap_next(&IT))

#endif
