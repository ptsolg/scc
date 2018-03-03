
#include "dseq-common.h"
#include <memory.h> // memcpy

#ifndef DSEQ_VALUE_TYPE
#error DSEQ_VALUE_TYPE is undefined
#endif

#ifndef DSEQ_TYPE
#error DSEQ_TYPE is undefined
#endif

#ifndef DSEQ_INIT
#error DSEQ_INIT is undefined
#endif

#ifndef DSEQ_INIT_ALLOC
#error DSEQ_INIT_ALLOC is undefined
#endif

#ifndef DSEQ_DISPOSE
#error DSEQ_DISPOSE is undefined
#endif

#ifndef DSEQ_GET_SIZE
#error DSEQ_GET_SIZE is undefined
#endif

#ifndef DSEQ_GET_CAPACITY
#error DSEQ_GET_CAPACITY is undefined
#endif

#ifndef DSEQ_GET_ALLOCATOR
#error DSEQ_GET_ALLOCATOR is undefined
#endif

#ifndef DSEQ_RESERVE
#error DSEQ_RESERVE is undefined
#endif

#ifndef DSEQ_RESIZE
#error DSEQ_RESIZE is undefined
#endif

#ifndef DSEQ_GET_BEGIN
#error DSEQ_GET_BEGIN is undefined
#endif

#ifndef DSEQ_GET_END
#error DSEQ_GET_END is undefined
#endif

#ifndef DSEQ_GET
#error DSEQ_GET is undefined
#endif

#ifndef DSEQ_SET
#error DSEQ_SET is undefined
#endif

#ifndef DSEQ_APPEND
#error DSEQ_APPEND is undefined
#endif

#ifndef DSEQ_SIZE_MULTIPLIER
#define DSEQ_SIZE_MULTIPLIER 2
#endif

#ifndef DSEQ_SIZE_ADDITION
#define DSEQ_SIZE_ADDITION 1
#endif

#if DSEQ_SIZE_ADDITION <= 0
#error Constant addition is less than zero.
#endif

#if DSEQ_SIZE_MULTIPLIER < 1
#error Multiplier is less than one.
#endif

#ifndef DSEQ_INLINE
#define DSEQ_INLINE inline
#endif

typedef struct _dseq DSEQ_TYPE;

static DSEQ_INLINE void DSEQ_INIT_ALLOC(DSEQ_TYPE* self, allocator* alloc)
{
        self->_elems = NULL;
        self->_capacity = 0;
        self->_size = 0;
        self->_alloc = alloc;
}

static DSEQ_INLINE void DSEQ_INIT(DSEQ_TYPE* self)
{
        DSEQ_INIT_ALLOC(self, STDALLOC);
}

static DSEQ_INLINE allocator* DSEQ_GET_ALLOCATOR(const DSEQ_TYPE* self)
{
        return self->_alloc;
}

static DSEQ_INLINE void DSEQ_DISPOSE(DSEQ_TYPE* self)
{
        deallocate(DSEQ_GET_ALLOCATOR(self), self->_elems);
        DSEQ_INIT_ALLOC(self, DSEQ_GET_ALLOCATOR(self));
}

static DSEQ_INLINE size_t DSEQ_GET_SIZE(const DSEQ_TYPE* self)
{
        return self->_size;
}

static DSEQ_INLINE size_t DSEQ_GET_CAPACITY(const DSEQ_TYPE* self)
{
        return self->_capacity;
}

static DSEQ_INLINE errcode DSEQ_RESERVE(DSEQ_TYPE* self, const size_t new_capacity)
{
        if (DSEQ_GET_CAPACITY(self) >= new_capacity)
                return S_NO_ERROR;

        uint8_t* new_elems = allocate(DSEQ_GET_ALLOCATOR(self), sizeof(DSEQ_VALUE_TYPE) * new_capacity);
        if (!new_elems)
                return S_ERROR;

        memcpy(new_elems, self->_elems, sizeof(DSEQ_VALUE_TYPE) * DSEQ_GET_SIZE(self));
        deallocate(DSEQ_GET_ALLOCATOR(self), self->_elems);

        self->_elems = new_elems;
        self->_capacity = new_capacity;

        return S_NO_ERROR;
}

static DSEQ_INLINE errcode DSEQ_RESIZE(DSEQ_TYPE* self, const size_t new_size)
{
        if (DSEQ_GET_CAPACITY(self) >= new_size)
        {
                self->_size = new_size;
                return S_NO_ERROR;
        }

        uint8_t* new_elems = allocate(DSEQ_GET_ALLOCATOR(self), sizeof(DSEQ_VALUE_TYPE) * new_size);
        if (!new_elems)
                return S_ERROR;

        size_t num_elems = S_MIN(DSEQ_GET_SIZE(self), new_size);
        memcpy(new_elems, self->_elems, sizeof(DSEQ_VALUE_TYPE) * num_elems);
        deallocate(DSEQ_GET_ALLOCATOR(self), self->_elems);

        self->_elems = new_elems;
        self->_capacity = 0;
        self->_size = new_size;

        return S_NO_ERROR;
}

static DSEQ_INLINE DSEQ_VALUE_TYPE* DSEQ_GET_BEGIN(const DSEQ_TYPE* self)
{
        return (DSEQ_VALUE_TYPE*)self->_elems;
}

static DSEQ_INLINE DSEQ_VALUE_TYPE* DSEQ_GET_END(const DSEQ_TYPE* self)
{
        return DSEQ_GET_BEGIN(self) + DSEQ_GET_SIZE(self);
}

static DSEQ_INLINE DSEQ_VALUE_TYPE DSEQ_GET(const DSEQ_TYPE* self, const size_t index)
{
        S_ASSERT(index < DSEQ_GET_SIZE(self) && "Index is out of range.");
        return DSEQ_GET_BEGIN(self)[index];
}

static DSEQ_INLINE void DSEQ_SET(
        const DSEQ_TYPE* self, DSEQ_VALUE_TYPE const value, const size_t index)
{
        S_ASSERT(index < DSEQ_GET_SIZE(self) && "Index is out of range.");
        DSEQ_GET_BEGIN(self)[index] = value;
}

static DSEQ_INLINE errcode DSEQ_APPEND(DSEQ_TYPE* self, DSEQ_VALUE_TYPE const value)
{
        if (DSEQ_GET_SIZE(self) == DSEQ_GET_CAPACITY(self))
        {
                size_t new_capacity = DSEQ_GET_SIZE(self) * DSEQ_SIZE_MULTIPLIER + DSEQ_SIZE_ADDITION;
                if (S_FAILED(DSEQ_RESERVE(self, new_capacity)))
                        return S_ERROR;
        }

        DSEQ_SET(self, value, self->_size++);
        return S_NO_ERROR;
}

#undef DSEQ_INLINE