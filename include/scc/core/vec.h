#ifndef VEC_H
#define VEC_H

#define VEC vec
#define VEC_T void*
#include "vec.inc"

#define VEC_FOREACH(PVEC, IT, END)\
        for (void** IT = vec_begin(PVEC), **END = vec_end(PVEC);\
                IT != END; (IT)++)

#endif
