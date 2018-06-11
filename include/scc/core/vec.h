#ifndef SCC_CORE_VEC_H
#define SCC_CORE_VEC_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define VEC_FN(N) ptrvec_##N
#define VEC_TP    ptrvec
#define VEC_VTP   void*
#include "vec-type.h"
#define PTRVEC_FOREACH(PVEC, IT, END) VEC_FOREACH(PVEC, void*, IT, END)

#define VEC_FN(N) u32vec_##N
#define VEC_TP    u32vec
#define VEC_VTP   uint32_t
#include "vec-type.h"
#define U32VEC_FOREACH(PVEC, IT, END) VEC_FOREACH(PVEC, uint32_t, IT, END)

#define VEC_FN(N) u8vec_##N
#define VEC_TP    u8vec
#define VEC_VTP   uint8_t
#include "vec-type.h"
#define U8VEC_FOREACH(PVEC, IT, END) VEC_FOREACH(PVEC, uint8_t, IT, END)

#ifdef __cplusplus
}
#endif

#endif