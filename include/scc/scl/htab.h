
#include "htab-common.h"

#ifndef HTAB_TYPE
#error HTAB_TYPE is undefined
#endif

#ifndef HTAB_IMPL_FN_GENERATOR
#error HTAB_IMPL_FN_GENERATOR is undefined
#endif

#ifndef HTAB_KEY_TYPE
#error HTAB_KEY_TYPE is undefined
#endif

#ifndef HTAB_DELETED_KEY
#error HTAB_DELETED_KEY is undefined
#endif

#ifndef HTAB_EMPTY_KEY
#error HTAB_EMPTY_KEY is undefined
#endif

#ifndef HTAB_VALUE_TYPE
#error HTAB_VALUE_TYPE is undefined
#endif

#ifndef HTAB_INIT
#error HTAB_INIT is undefined
#endif

#ifndef HTAB_INIT_ALLOC
#error HTAB_INIT_ALLOC is undefined
#endif

#ifndef HTAB_DISPOSE
#error HTAB_DISPOSE is undefined
#endif

#ifndef HTAB_GET_SIZE
#error HTAB_GET_SIZE is undefined
#endif

#ifndef HTAB_GET_ALLOCATOR
#error HTAB_GET_ALLOCATOR is undefined
#endif

#ifndef HTAB_RESERVE
#error HTAB_RESERVE is undefined
#endif

#ifndef HTAB_ITERATOR_TYPE
#error HTAB_ITERATOR_TYPE is undefined
#endif

#ifndef HTAB_ITERATOR_GET_KEY
#error HTAB_ITERATOR_GET_KEY is undefined
#endif

#ifndef HTAB_ITERATOR_ADVANCE
#error HTAB_ITERATOR_ADVANCE is undefined
#endif

#ifndef HTAB_ITERATOR_INIT
#error HTAB_ITERATOR_INIT is undefined
#endif

#ifndef HTAB_ITERATOR_CREATE
#error HTAB_ITERATOR_CREATE is undefined
#endif

#ifndef HTAB_ITERATOR_IS_VALID
#error HTAB_ITERATOR_IS_VALID is undefined
#endif

#ifndef HTAB_ITERATOR_GET_VALUE
#error HTAB_ITERATOR_GET_VALUE is undefined
#endif

#ifndef HTAB_CLEAR
#error HTAB_CLEAR is undefined
#endif

#ifndef HTAB_GROW
#error HTAB_GROW is undefined
#endif

#ifndef HTAB_INSERT
#error HTAB_INSERT is undefined
#endif

#ifndef HTAB_FIND
#error HTAB_FIND is undefined
#endif

typedef struct _htab HTAB_TYPE;
typedef struct _hiter HTAB_ITERATOR_TYPE;

#define HTAB_FIND_IMPL HTAB_IMPL_FN_GENERATOR(find_impl)
#define HTAB_RESIZE_IMPL HTAB_IMPL_FN_GENERATOR(resize_impl)
#define HTAB_BUCKET_SIZE (sizeof(HTAB_KEY_TYPE) + sizeof(HTAB_VALUE_TYPE))

#ifndef HTAB_INLINE
#define HTAB_INLINE inline
#endif

static HTAB_INLINE void HTAB_INIT_ALLOC(HTAB_TYPE* self, allocator* alloc)
{
        self->_alloc = alloc;
        self->_buckets = NULL;
        self->_num_buckets = 0;
        self->_size = 0;
}

static HTAB_INLINE void HTAB_INIT(HTAB_TYPE* self)
{
        HTAB_INIT_ALLOC(self, STDALLOC);
}

static HTAB_INLINE allocator* HTAB_GET_ALLOCATOR(const HTAB_TYPE* self)
{
        return self->_alloc;
}

static HTAB_INLINE void HTAB_DISPOSE(HTAB_TYPE* self)
{
        deallocate(HTAB_GET_ALLOCATOR(self), self->_buckets);
        HTAB_INIT_ALLOC(self, HTAB_GET_ALLOCATOR(self));
}

static HTAB_INLINE ssize HTAB_GET_SIZE(const HTAB_TYPE* self)
{
        return self->_size;
}

static HTAB_INLINE HTAB_KEY_TYPE HTAB_ITERATOR_GET_KEY(const HTAB_ITERATOR_TYPE* self)
{
        return *(HTAB_KEY_TYPE*)self->_bucket;
}

static HTAB_INLINE void HTAB_ITERATOR_ADVANCE(HTAB_ITERATOR_TYPE* self)
{
        self->_bucket += HTAB_BUCKET_SIZE;
}

static HTAB_INLINE void HTAB_ITERATOR_INIT(HTAB_ITERATOR_TYPE* self, const HTAB_TYPE* tab)
{
        self->_bucket = tab->_buckets;
        self->_end = tab->_buckets + HTAB_BUCKET_SIZE * tab->_num_buckets;
}

static HTAB_INLINE HTAB_ITERATOR_TYPE HTAB_ITERATOR_CREATE(const HTAB_TYPE* tab)
{
        HTAB_ITERATOR_TYPE it;
        HTAB_ITERATOR_INIT(&it, tab);
        return it;
}

static HTAB_INLINE bool HTAB_ITERATOR_IS_VALID(const HTAB_ITERATOR_TYPE* self)
{
        if (self->_bucket == self->_end)
                return false;

        HTAB_KEY_TYPE k = HTAB_ITERATOR_GET_KEY(self);
        return k != HTAB_EMPTY_KEY && k != HTAB_DELETED_KEY;
}

static HTAB_INLINE HTAB_VALUE_TYPE* HTAB_ITERATOR_GET_VALUE(const HTAB_ITERATOR_TYPE* self)
{
        return (HTAB_VALUE_TYPE*)(self->_bucket + sizeof(HTAB_KEY_TYPE));
}

static HTAB_INLINE void HTAB_CLEAR(HTAB_TYPE* self)
{
        for (HTAB_ITERATOR_TYPE it = HTAB_ITERATOR_CREATE(self);
                it._bucket != it._end; HTAB_ITERATOR_ADVANCE(&it))
        {
                *(HTAB_KEY_TYPE*)it._bucket = HTAB_EMPTY_KEY;
        }

        self->_size = 0;
}

static HTAB_INLINE serrcode HTAB_INSERT(HTAB_TYPE*, HTAB_KEY_TYPE const, HTAB_VALUE_TYPE const);
static HTAB_INLINE serrcode HTAB_RESIZE_IMPL(HTAB_TYPE* self, const ssize new_size)
{
        S_ASSERT(IS_POWEROF2(new_size));
        suint8* new_buckets = allocate(HTAB_GET_ALLOCATOR(self), new_size * HTAB_BUCKET_SIZE);
        if (!new_buckets)
                return S_ERROR;

        HTAB_TYPE new_tab;
        new_tab._buckets = new_buckets;
        new_tab._num_buckets = new_size;
        new_tab._size = 0;
        new_tab._alloc = self->_alloc;
        HTAB_CLEAR(&new_tab);

        for (HTAB_ITERATOR_TYPE it = HTAB_ITERATOR_CREATE(self);
                it._bucket != it._end; HTAB_ITERATOR_ADVANCE(&it))
        {
                if (HTAB_ITERATOR_IS_VALID(&it))
                        HTAB_INSERT(&new_tab, HTAB_ITERATOR_GET_KEY(&it), *HTAB_ITERATOR_GET_VALUE(&it));
        }

        HTAB_DISPOSE(self);
        *self = new_tab;

        return S_NO_ERROR;
}

static HTAB_INLINE serrcode HTAB_RESERVE(HTAB_TYPE* self, const ssize at_least)
{
        if (at_least <= self->_num_buckets - self->_size)
                return S_NO_ERROR;

        ssize new_size = self->_num_buckets;
        while (at_least > new_size - self->_size)
                new_size *= 2;
        return HTAB_RESIZE_IMPL(self, new_size);
}

static HTAB_INLINE suint8* HTAB_FIND_IMPL(const HTAB_TYPE* self, HTAB_KEY_TYPE const key)
{
        if (!self->_buckets)
                return NULL;

        const ssize size = self->_num_buckets;

        ssize i = 1;
        ssize start = key & (size - 1);
        ssize bucket_no = start;

        while (1)
        {
                suint8* bucket = self->_buckets + HTAB_BUCKET_SIZE * bucket_no;

                HTAB_KEY_TYPE k = *(HTAB_KEY_TYPE*)bucket;

                if (k == key || k == HTAB_EMPTY_KEY)
                        return bucket;

                bucket_no += i++;
                bucket_no &= (size - 1);

                if (bucket_no == start)
                {
                        // if we are here, then hash table has 0 free slots
                        return NULL;
                }
        }
}

static HTAB_INLINE serrcode HTAB_GROW(HTAB_TYPE* self)
{
        return HTAB_RESIZE_IMPL(self, self->_num_buckets ? self->_num_buckets << 1 : 2);
}

static HTAB_INLINE serrcode HTAB_INSERT(
        HTAB_TYPE* self, HTAB_KEY_TYPE const key, HTAB_VALUE_TYPE const value)
{
        S_ASSERT(key != HTAB_DELETED_KEY && key != HTAB_EMPTY_KEY
                && "Deleted/Empty keys should not be used.");

        ssize critical = self->_num_buckets >> 1;
        if (self->_size >= critical)
        {
                bool has_free_buckets = self->_size != self->_num_buckets;
                if (S_FAILED(HTAB_GROW(self)) && !has_free_buckets)
                        return S_ERROR;
        }

        suint8* bucket = HTAB_FIND_IMPL(self, key);
        if (!bucket)
                return S_ERROR;

        if (*(HTAB_KEY_TYPE*)bucket == HTAB_EMPTY_KEY)
                self->_size++;

        *(HTAB_KEY_TYPE*)bucket = key;
        *(HTAB_VALUE_TYPE*)(bucket + sizeof(HTAB_KEY_TYPE)) = value;
        return S_NO_ERROR;
}

static HTAB_INLINE bool HTAB_FIND(
        const HTAB_TYPE* self, HTAB_KEY_TYPE const key, HTAB_ITERATOR_TYPE* result)
{
        suint8* bucket = HTAB_FIND_IMPL(self, key);
        if (!bucket || *(HTAB_KEY_TYPE*)bucket != key)
        {
                result->_bucket = NULL;
                result->_end = NULL;
                return false;
        }

        HTAB_ITERATOR_INIT(result, self);
        result->_bucket = bucket;

        return true;
}

#undef HTAB_INLINE
#undef HTAB_FIND_IMPL
#undef HTAB_BUCKET_SIZE
#undef HTAB_RESIZE_IMPL