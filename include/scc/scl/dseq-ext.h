#ifndef SDSEQ_EXT_H
#define SDSEQ_EXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "dseq.h"

#define DSEQ_CHECK_TYPE(SEQ, TYPE)\
        S_ASSERT(dseq_obsize(self) == sizeof(TYPE) && "dynamic sequence type mismatch")

#define DSEQ_CHECK_TYPE_AND_INDEX(SEQ, TYPE, INDEX) \
        do { DSEQ_CHECK_TYPE(SEQ, TYPE); DSEQ_CHECK_INDEX(SEQ, INDEX); } while(0)

#define DSEQ_CHECK_HAS_ELEM(SEQ)\
        S_ASSERT(dseq_size(self) && "dynamic sequence is empty")

#define DSEQ_INIT_EX(SUFFIX) dseq_init_ex_##SUFFIX
#define DSEQ_GEN_INIT_EX(SUFFIX, TYPE)                                        \
        static inline void DSEQ_INIT_EX(SUFFIX)(dseq* self, allocator* alloc) \
        {                                                                     \
                dseq_init_ex(self, sizeof(TYPE), alloc);                      \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_INIT_EX(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_INIT(SUFFIX) dseq_init_##SUFFIX
#define DSEQ_GEN_INIT(SUFFIX, TYPE)                          \
        static inline void DSEQ_INIT(SUFFIX)(dseq* self)     \
        {                                                    \
                DSEQ_INIT_EX(SUFFIX)(self, get_std_alloc()); \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_INIT(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_BEGIN(SUFFIX) dseq_begin_##SUFFIX
#define DSEQ_GEN_BEGIN(SUFFIX, TYPE) \
        static inline TYPE* DSEQ_BEGIN(SUFFIX)(const dseq* self) \
        {                                                        \
                DSEQ_CHECK_TYPE(self, TYPE);                     \
                return ((TYPE*)membuf_begin(dseq_cbase(self)));  \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_BEGIN(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_END(SUFFIX) dseq_end_##SUFFIX
#define DSEQ_GEN_END(SUFFIX, TYPE)                                  \
        static inline TYPE* DSEQ_END(SUFFIX)(const dseq* self)      \
        {                                                           \
                DSEQ_CHECK_TYPE(self, TYPE);                        \
                return DSEQ_BEGIN(SUFFIX)(self) + dseq_size(self);  \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_END(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_FIRST(SUFFIX) dseq_first_##SUFFIX
#define DSEQ_GEN_FIRST(SUFFIX, TYPE)                            \
        static inline TYPE DSEQ_FIRST(SUFFIX)(const dseq* self) \
        {                                                       \
                DSEQ_CHECK_HAS_ELEM(self);                      \
                return *DSEQ_BEGIN(SUFFIX)(self);               \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_FIRST(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_LAST(SUFFIX) dseq_last_##SUFFIX
#define DSEQ_GEN_LAST(SUFFIX, TYPE)                            \
        static inline TYPE DSEQ_LAST(SUFFIX)(const dseq* self) \
        {                                                      \
                DSEQ_CHECK_HAS_ELEM(self);                     \
                return *(DSEQ_END(SUFFIX)(self) - 1);          \
        }


#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_LAST(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_GEN_APPEND(SUFFIX, TYPE)                                     \
        static inline serrcode dseq_append_##SUFFIX(dseq* self, TYPE val) \
        {                                                                 \
                DSEQ_CHECK_TYPE(self, TYPE);                              \
                if (!_dseq_maybe_grow(self))                              \
                        return S_ERROR;                                   \
                self->_size++;                                            \
                *(TYPE*)dseq_last(self) = val;                            \
                return S_NO_ERROR;                                        \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_APPEND(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_GEN_GET(SUFFIX, TYPE)                                      \
        static inline TYPE dseq_get_##SUFFIX(const dseq* self, ssize i) \
        {                                                               \
                DSEQ_CHECK_TYPE_AND_INDEX(self, TYPE, i);               \
                return (DSEQ_BEGIN(SUFFIX)(self))[i];                   \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_GET(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_GEN_SET(SUFFIX, TYPE)                                          \
        static inline TYPE dseq_set_##SUFFIX(dseq* self, ssize i, TYPE val) \
        {                                                                   \
                DSEQ_CHECK_TYPE_AND_INDEX(self, TYPE, i);                   \
                (DSEQ_BEGIN(SUFFIX)(self))[i] = val;                        \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_SET(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#define DSEQ_GEN_POP(SUFFIX, TYPE)\
        static inline TYPE dseq_pop_##SUFFIX(dseq* self) \
        {                                                \
                DSEQ_CHECK_HAS_ELEM(self);               \
                self->_size--;                           \
                return *DSEQ_END(SUFFIX)(self);          \
        }

#define DSEQ_TYPE(SUFFIX, TYPE) DSEQ_GEN_POP(SUFFIX, TYPE)
#include "dseq-types.inc"
#undef DSEQ_TYPE

#ifdef __cplusplus
}
#endif

#endif // !SDSEQ_EXT_H
