// before including this header the following macro should be defined:
//      VEC_FN - vector function generator
//      VEC_TP - vector type
//      VEC_VTP - vector value type

#include "memory.h"
#include <string.h> // memcpy

#ifndef VEC_FN
#error VEC_FN is undefined
#endif

#ifndef VEC_TP
#error VEC_TP is undefined
#endif

#ifndef VEC_VTP
#error VEC_VTP is undefined
#endif

#ifndef VEC_MUL
#define VEC_MUL 2
#endif

#ifndef VEC_ADD
#define VEC_ADD 1
#endif

#if VEC_ADD <= 0
#error Constant addition is less than zero.
#endif

#if VEC_MUL < 1
#error Multiplier is less than one.
#endif

typedef struct
{
        VEC_VTP* elems;
        allocator* alloc;
        size_t size;
        size_t capacity;
} VEC_TP;

#ifndef VEC_FOREACH
#define VEC_FOREACH(PVEC, VTP, IT, END) \
        for (VTP* IT = (PVEC)->elems, *END = IT + (PVEC)->size; IT != END; IT++)
#endif

static void VEC_FN(init_ex)(VEC_TP* self, allocator* alloc)
{
        self->elems = NULL;
        self->capacity = 0;
        self->size = 0;
        self->alloc = alloc;
}

static void VEC_FN(init)(VEC_TP* self)
{
        VEC_FN(init_ex)(self, STDALLOC);
}

static VEC_TP VEC_FN(create_ex)(allocator* alloc)
{
        VEC_TP v;
        VEC_FN(init_ex)(&v, alloc);
        return v;
}

static VEC_TP VEC_FN(create)()
{
        return VEC_FN(create_ex)(STDALLOC);
}

static void VEC_FN(dispose)(VEC_TP* self)
{
        deallocate(self->alloc, self->elems);
        VEC_FN(init_ex)(self, self->alloc);
}

static errcode VEC_FN(copy)(VEC_TP* to, const VEC_TP* from)
{
        VEC_VTP* new_elems = allocate(to->alloc, sizeof(VEC_VTP) * from->size);
        if (!new_elems)
                return EC_ERROR;

        memcpy(new_elems, from->elems, sizeof(VEC_VTP) * from->size);

        VEC_FN(dispose)(to);
        to->elems = new_elems;
        to->capacity = from->size;
        to->size = from->size;

        return EC_NO_ERROR;
}

static inline errcode VEC_FN(shrink)(VEC_TP* self)
{
        VEC_TP copy;
        if (EC_FAILED(VEC_FN(copy)(&copy, self)))
                return EC_ERROR;

        VEC_FN(dispose)(self);
        *self = copy;
        return EC_NO_ERROR;
}

static inline errcode VEC_FN(reserve)(VEC_TP* self, const size_t new_capacity)
{
        if (self->capacity >= new_capacity)
                return EC_NO_ERROR;

        VEC_VTP* new_elems = allocate(self->alloc, sizeof(VEC_VTP) * new_capacity);
        if (!new_elems)
                return EC_ERROR;

        memcpy(new_elems, self->elems, sizeof(VEC_VTP) * self->size);
        deallocate(self->alloc, self->elems);

        self->elems = new_elems;
        self->capacity = new_capacity;

        return EC_NO_ERROR;
}

static inline errcode VEC_FN(resize)(VEC_TP* self, const size_t new_size)
{
        if (self->capacity >= new_size)
        {
                self->size = new_size;
                return EC_NO_ERROR;
        }

        VEC_VTP* new_elems = allocate(self->alloc, sizeof(VEC_VTP) * new_size);
        if (!new_elems)
                return EC_ERROR;

        size_t num_elems = MIN(self->size, new_size);
        memcpy(new_elems, self->elems, sizeof(VEC_VTP) * num_elems);
        deallocate(self->alloc, self->elems);

        self->elems = new_elems;
        self->capacity = new_size;
        self->size = new_size;

        return EC_NO_ERROR;
}

static inline VEC_VTP* VEC_FN(begin)(const VEC_TP* self)
{
        return self->elems;
}

static inline VEC_VTP* VEC_FN(end)(const VEC_TP* self)
{
        return VEC_FN(begin)(self) + self->size;
}

static inline VEC_VTP* VEC_FN(first_p)(const VEC_TP* self)
{
        assert(self->size);
        return VEC_FN(begin)(self);
}

static inline VEC_VTP VEC_FN(first)(const VEC_TP* self)
{
        return *VEC_FN(first_p)(self);
}

static inline VEC_VTP* VEC_FN(last_p)(const VEC_TP* self)
{
        assert(self->size);
        return VEC_FN(end)(self) - 1;
}

static inline VEC_VTP VEC_FN(last)(const VEC_TP* self)
{
        return *VEC_FN(last_p)(self);
}

static inline VEC_VTP* VEC_FN(get_p)(const VEC_TP* self, size_t i)
{
        assert(i < self->size);
        return VEC_FN(begin)(self) + i;
}

static inline VEC_VTP VEC_FN(get)(const VEC_TP* self, size_t i)
{
        return *VEC_FN(get_p)(self, i);
}

static inline void VEC_FN(set_p)(const VEC_TP* self, VEC_VTP const* p, size_t i)
{
        assert(i < self->size);
        VEC_FN(begin)(self)[i] = *p;
}

static inline void VEC_FN(set)(const VEC_TP* self, VEC_VTP v, size_t i)
{
        VEC_FN(set_p)(self, &v, i);
}

static inline VEC_VTP* VEC_FN(push_e)(VEC_TP* self)
{
        if (self->size == self->capacity)
        {
                size_t new_capacity = self->size * VEC_MUL + VEC_ADD;
                if (EC_FAILED(VEC_FN(reserve)(self, new_capacity)))
                        return NULL;
        }
        self->size++;
        return VEC_FN(last_p)(self);
}

static inline errcode VEC_FN(push_p)(VEC_TP* self, VEC_VTP const* p)
{
        VEC_VTP* last = VEC_FN(push_e)(self);
        if (!last)
                return EC_ERROR;

        *last = *p;
        return EC_NO_ERROR;
}

static inline errcode VEC_FN(push)(VEC_TP* self, VEC_VTP v)
{
        return VEC_FN(push_p)(self, &v);
}

static inline VEC_VTP* VEC_FN(pop_p)(VEC_TP* self)
{
        VEC_VTP* last = VEC_FN(last_p)(self);
        self->size--;
        return last;
}

static inline VEC_VTP VEC_FN(pop)(VEC_TP* self)
{
        return *VEC_FN(pop_p)(self);
}

#undef VEC_FN
#undef VEC_TP
#undef VEC_VTP
#undef VEC_MUL
#undef VEC_ADD
