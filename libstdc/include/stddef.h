#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void*)0)

#if _M32
typedef unsigned size_t;
typedef int ptrdiff_t;
#else
typedef unsigned long long size_t;
typedef long long ptrdiff_t;
#endif

#define offsetof(type, member) __offsetof(type, member)

#endif
