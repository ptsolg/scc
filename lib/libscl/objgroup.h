#ifndef SOBJGROUP_H
#define SOBJGROUP_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "membuf.h"

// a dynamic array of void*
typedef struct _objgroup
{
        membuf _base;
        void** _last;
} objgroup;

extern void     objgroup_init(objgroup* self);
extern void     objgroup_init_ex(objgroup* self, allocator* alloc);
extern void     objgroup_dispose(objgroup* self);
extern void     objgroup_move(objgroup* to, objgroup* from);
extern serrcode objgroup_reserve(objgroup* self, ssize size);
extern serrcode objgroup_resize(objgroup* self, ssize size);

static inline membuf*       objgroup_base(objgroup* self);
static inline const membuf* objgroup_cbase(const objgroup* self);

static inline ssize    objgroup_size(const objgroup* self);
static inline ssize    objgroup_available(const objgroup* self);
static inline ssize    objgroup_total(const objgroup* self);
static inline serrcode objgroup_push_back(objgroup* self, void* obj);
static inline void*    objgroup_pop_back(objgroup* self);

static inline void** objgroup_begin(const objgroup* self);
static inline void** objgroup_end(const objgroup* self);
static inline void*  objgroup_first(const objgroup* self);
static inline void*  objgroup_last(const objgroup* self);
static inline void*  objgroup_nth(const objgroup* self, ssize n);

#define OBJGROUP_FOREACH(PGROUP, ITTYPE, ITNAME)                          \
        for (ITTYPE ITNAME = (ITTYPE)objgroup_begin(((objgroup*)PGROUP)); \
                ITNAME != (ITTYPE)objgroup_end(((objgroup*)PGROUP)); ITNAME++)

static inline membuf* objgroup_base(objgroup* self)
{
        return &self->_base;
}

static inline const membuf* objgroup_cbase(const objgroup* self)
{
        return &self->_base;
}

static inline ssize objgroup_size(const objgroup* self)
{
        return self->_last - (void**)membuf_begin(objgroup_cbase(self));
}

static inline ssize objgroup_available(const objgroup* self)
{
        return (void**)membuf_end(objgroup_cbase(self)) - self->_last;
}

static inline ssize objgroup_total(const objgroup* self)
{
        return objgroup_size(self) + objgroup_available(self);
}

static inline serrcode objgroup_push_back(objgroup* self, void* obj)
{
        if (!objgroup_available(self))
                if (S_FAILED(objgroup_reserve(self, objgroup_size(self) + 5)))
                        return S_ERROR;

        *self->_last++ = obj;
        return S_NO_ERROR;
}

static inline void* objgroup_pop_back(objgroup* self)
{
        S_ASSERT(objgroup_size(self));
        void* res  = objgroup_last(self);
        serrcode e = objgroup_resize(self, objgroup_size(self) - 1);
        S_ASSERT(S_SUCCEEDED(e));
        return res;
}

static inline void** objgroup_begin(const objgroup* self)
{
        return (void**)membuf_begin(objgroup_cbase(self));
}

static inline void** objgroup_end(const objgroup* self)
{
        return self->_last;
}

static inline void* objgroup_first(const objgroup* self)
{
        return objgroup_nth(self, 0);
}

static inline void* objgroup_last(const objgroup* self)
{
        S_ASSERT(objgroup_size(self));
        return *(self->_last - 1);
}

static inline void* objgroup_nth(const objgroup* self, ssize n)
{
        S_ASSERT(n < objgroup_size(self) && "Object is outside range");
        return *(objgroup_begin(self) + n);
}

#ifdef __cplusplus
}
#endif

#endif // !SOBJGROUP_H