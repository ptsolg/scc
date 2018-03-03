#ifndef SDSEQ_INSTANCE_H
#define SDSEQ_INSTANCE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DSEQ_VALUE_TYPE void*
#define DSEQ_TYPE dseq
#define DSEQ_INIT dseq_init
#define DSEQ_INIT_ALLOC dseq_init_alloc
#define DSEQ_DISPOSE dseq_dispose
#define DSEQ_GET_SIZE dseq_size
#define DSEQ_GET_CAPACITY dseq_capacity
#define DSEQ_GET_ALLOCATOR dseq_allocator
#define DSEQ_RESERVE dseq_reserve
#define DSEQ_RESIZE dseq_resize
#define DSEQ_GET_BEGIN dseq_begin
#define DSEQ_GET_END dseq_end
#define DSEQ_GET dseq_get
#define DSEQ_SET dseq_set
#define DSEQ_APPEND dseq_append

#include "dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

#define DSEQ_VALUE_TYPE suint32
#define DSEQ_TYPE dseq_u32
#define DSEQ_INIT dseq_u32_init
#define DSEQ_INIT_ALLOC dseq_u32_init_alloc
#define DSEQ_DISPOSE dseq_u32_dispose
#define DSEQ_GET_SIZE dseq_u32_size
#define DSEQ_GET_CAPACITY dseq_u32_capacity
#define DSEQ_GET_ALLOCATOR dseq_u32_allocator
#define DSEQ_RESERVE dseq_u32_reserve
#define DSEQ_RESIZE dseq_u32_resize
#define DSEQ_GET_BEGIN dseq_u32_begin
#define DSEQ_GET_END dseq_u32_end
#define DSEQ_GET dseq_u32_get
#define DSEQ_SET dseq_u32_set
#define DSEQ_APPEND dseq_u32_append

#include "dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

#define DSEQ_VALUE_TYPE suint8
#define DSEQ_TYPE dseq_u8
#define DSEQ_INIT dseq_u8_init
#define DSEQ_INIT_ALLOC dseq_u8_init_alloc
#define DSEQ_DISPOSE dseq_u8_dispose
#define DSEQ_GET_SIZE dseq_u8_size
#define DSEQ_GET_CAPACITY dseq_u8_capacity
#define DSEQ_GET_ALLOCATOR dseq_u8_allocator
#define DSEQ_RESERVE dseq_u8_reserve
#define DSEQ_RESIZE dseq_u8_resize
#define DSEQ_GET_BEGIN dseq_u8_begin
#define DSEQ_GET_END dseq_u8_end
#define DSEQ_GET dseq_u8_get
#define DSEQ_SET dseq_u8_set
#define DSEQ_APPEND dseq_u8_append

#include "dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND 

#ifdef __cplusplus
}
#endif

#endif // !SDSEQ_INSTANCE_H
