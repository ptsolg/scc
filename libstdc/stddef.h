#ifndef STDDEF_H
#define STDDEF_H

#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif

#endif