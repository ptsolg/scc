#include <assert.h>

#include "alloc.h"

void* memcpy(void*, const void*, size_t);

#ifndef VEC
#error VEC undefined
#endif

#ifndef VEC_T
#error VEC_VAL undefined
#endif

#ifndef VEC_F
#define __CONCAT(a, b) a ## b
#define _CONCAT(a, b) __CONCAT(a, b)
#define VEC_F(x) _CONCAT(_CONCAT(VEC, _), x)
#endif

struct VEC
{
        VEC_T* items;
        size_t size;
        size_t capacity;
};


static void VEC_F(init)(struct VEC* self)
{
        self->items = 0;
        self->capacity = 0;
        self->size = 0;
}

static struct VEC* VEC_F(new)(void)
{
        struct VEC* v = alloc(sizeof(struct VEC));
        VEC_F(init)(v);
        return v;
}

static void VEC_F(clear)(struct VEC* self)
{
        dealloc(self->items);
        VEC_F(init)(self);
}

static void VEC_F(drop)(struct VEC* self)
{
        VEC_F(clear)(self);
}

static void VEC_F(del)(struct VEC* self)
{
        VEC_F(drop)(self);
        dealloc(self);
}

static void VEC_F(copy)(struct VEC* to, const struct VEC* from)
{
        VEC_T* new_items = alloc(sizeof(VEC_T) * from->size);
        memcpy(new_items, from->items, sizeof(VEC_T) * from->size);
        VEC_F(drop)(to);
        to->items = new_items;
        to->capacity = from->size;
        to->size = from->size;
}

static inline void VEC_F(shrink)(struct VEC* self)
{
        struct VEC copy;
        VEC_F(copy)(&copy, self);
        VEC_F(drop)(self);
        *self = copy;
}

static inline void VEC_F(reserve)(struct VEC* self, const size_t new_capacity)
{
        if (self->capacity >= new_capacity)
                return;

        VEC_T* new_items = alloc(sizeof(VEC_T) * new_capacity);
        memcpy(new_items, self->items, sizeof(VEC_T) * self->size);
        dealloc(self->items);
        self->items = new_items;
        self->capacity = new_capacity;
}

static inline void VEC_F(resize)(struct VEC* self, const size_t new_size)
{
        if (self->capacity >= new_size) {
                self->size = new_size;
                return;
        }

        VEC_T* new_items = alloc(sizeof(VEC_T) * new_size);
        size_t num_elems = self->size > new_size ? new_size : self->size;
        memcpy(new_items, self->items, sizeof(VEC_T) * num_elems);
        dealloc(self->items);
        self->items = new_items;
        self->capacity = new_size;
        self->size = new_size;
}

static inline void VEC_F(push)(struct VEC* self, VEC_T v)
{
        if (self->size == self->capacity)
                VEC_F(reserve)(self, self->size * 2 + 1);
        self->items[self->size++] = v;
}

static inline VEC_T VEC_F(pop)(struct VEC* self)
{
        assert(self->size);
        return self->items[--self->size];
}

static inline VEC_T VEC_F(last)(const struct VEC* self)
{
        assert(self->size);
        return self->items[self->size - 1];
}

static inline VEC_T* VEC_F(last_ptr)(const struct VEC* self)
{
        assert(self->size);
        return self->items + self->size - 1;
}

static inline VEC_T* VEC_F(begin)(const struct VEC* self)
{
        return self->items;
}

static inline VEC_T* VEC_F(end)(const struct VEC* self)
{
        return self->items + self->size;
}

static inline VEC_T VEC_F(get)(const struct VEC* self, size_t i)
{
        assert(i < self->size);
        return self->items[i];
}

#undef VEC_F
#undef _CONCAT
#undef __CONCAT
#undef VEC_T
#undef VEC
