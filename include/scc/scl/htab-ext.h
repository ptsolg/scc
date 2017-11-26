#ifndef SHTAB_EXT_H
#define SHTAB_EXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "htab.h"

#define HTAB_CHECK_TYPE(TAB, TYPE)\
        S_ASSERT(htab_obsize(TAB) == sizeof(TYPE) && "hash table type mismatch")

#define HITER_CHECK_TYPE(IT, TYPE) HTAB_CHECK_TYPE((IT)->_tab, TYPE)

#define HTAB_INIT_EX(SUFFIX) htab_init_ex_##SUFFIX
#define HTAB_GEN_INIT_EX(SUFFIX, TYPE)                                        \
        static inline void HTAB_INIT_EX(SUFFIX)(htab* self, allocator* alloc) \
        {                                                                     \
                htab_init_ex(self, sizeof(TYPE), alloc);                      \
        }

#define HTAB_INIT(SUFFIX) htab_init_##SUFFIX
#define HTAB_GEN_INIT(SUFFIX, TYPE)                          \
        static inline void HTAB_INIT(SUFFIX)(htab* self)     \
        {                                                    \
                HTAB_INIT_EX(SUFFIX)(self, STDALLOC);        \
        }

#define HTAB_INSERT(SUFFIX) htab_insert_##SUFFIX
#define HTAB_GEN_INSERT(SUFFIX, TYPE)                                              \
        static inline serrcode HTAB_INSERT(SUFFIX)(htab* self, hval key, TYPE val) \
        {                                                                          \
                HTAB_CHECK_TYPE(self, TYPE);                                       \
                return htab_insert(self, key, &val);                               \
        }

#define HITER_GET(SUFFIX) hiter_get_##SUFFIX
#define HITER_GEN_GET(SUFFIX, TYPE)                             \
        static inline TYPE HITER_GET(SUFFIX)(const hiter* self) \
        {                                                       \
                HITER_CHECK_TYPE(self, TYPE);                   \
                return *(TYPE*)hiter_get_val(self);             \
        }

#define HITER_SET(SUFFIX) hiter_set_##SUFFIX
#define HITER_GEN_SET(SUFFIX, TYPE)                                       \
        static inline void HITER_SET(SUFFIX)(const hiter* self, TYPE val) \
        {                                                                 \
                HITER_CHECK_TYPE(self, TYPE);                             \
                hiter_set_val(self, &val);                                \
        }

#define HTAB_GEN(SUFFIX, TYPE)         \
        HTAB_GEN_INIT_EX(SUFFIX, TYPE) \
        HTAB_GEN_INIT(SUFFIX, TYPE)    \
        HTAB_GEN_INSERT(SUFFIX, TYPE)  \
        HITER_GEN_GET(SUFFIX, TYPE)    \
        HITER_GEN_SET(SUFFIX, TYPE)

#define DEF_TYPE(SUFFIX, TYPE) HTAB_GEN(SUFFIX, TYPE)
#include "def-type.inc"
#ifdef HTAB_INC
#include HTAB_INC
#endif
#undef DEF_TYPE

#ifdef __cplusplus
}
#endif

#endif // !SHTAB_EXT_H
