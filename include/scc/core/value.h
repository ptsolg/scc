#ifndef SCC_CORE_VALUE_H
#define SCC_CORE_VALUE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "misc.h"

typedef enum
{
        OR_OK,
        OR_DIV_BY_ZERO,
        OR_OVERFLOW,
        OR_UNDERFLOW,
        OR_INVALID,
} op_result;

typedef struct _int_value int_value;

typedef enum
{
        FP_SINGLE,
        FP_DOUBLE,
} float_precision;

typedef struct _float_value
{
        float_precision precision;
        union
        {
                float as_float;
                double as_double;
        };
} float_value;

extern void float_init_sp(float_value* self, float v);
extern void float_init_dp(float_value* self, double v);
extern void float_set_sign(float_value* self, bool positive);

extern float_precision float_get_precision(const float_value* self);

extern op_result float_add(float_value* self, const float_value* rhs);
extern op_result float_sub(float_value* self, const float_value* rhs);
extern op_result float_mul(float_value* self, const float_value* rhs);
extern op_result float_div(float_value* self, const float_value* rhs);
extern op_result float_neg(float_value* self);
extern cmp_result float_cmp(const float_value* lhs, const float_value* rhs);

extern bool float_is_positive(const float_value* self);
extern bool float_is_zero(const float_value* val);
extern bool float_is_inf(const float_value* val);
extern bool float_is_nan(const float_value* val);
extern bool float_is_sp(const float_value* val);
extern bool float_is_dp(const float_value* val);
                  
extern void float_to_dp(float_value* val);
extern void float_to_sp(float_value* val);
extern int_value float_to_int(const float_value* val, uint bits);

extern float float_get_sp(const float_value* val);
extern double float_get_dp(const float_value* val);
extern uint8_t float_get_u8(const float_value* val);
extern uint16_t float_get_u16(const float_value* val);
extern uint32_t float_get_u32(const float_value* val);
extern uint64_t float_get_u64(const float_value* val);
extern int8_t float_get_i8(const float_value* val);
extern int16_t float_get_i16(const float_value* val);
extern int32_t float_get_i32(const float_value* val);
extern int64_t float_get_i64(const float_value* val);

extern int float_print(const float_value* val, char* buf, size_t count, int precision);
extern int float_print_as_hex(const float_value* val, char* buf, size_t count);

typedef struct _int_value
{
        uint64_t val;
        uint bits;
        bool issigned;
} int_value;

extern void int_init(int_value* self, uint bits, bool signed_, uint64_t val);

extern bool int_is_signed(const int_value* self);
extern void int_set_signed(int_value* self, bool issigned);
extern uint int_get_bits(const int_value* self);

extern op_result int_add(int_value* self, const int_value* rhs);
extern op_result int_sub(int_value* self, const int_value* rhs);
extern op_result int_mul(int_value* self, const int_value* rhs);
extern op_result int_div(int_value* self, const int_value* rhs);
extern op_result int_mod(int_value* self, const int_value* rhs);
extern op_result int_shl(int_value* self, const int_value* rhs);
extern op_result int_shr(int_value* self, const int_value* rhs);
extern op_result int_and(int_value* self, const int_value* rhs);
extern op_result int_xor(int_value* self, const int_value* rhs);
extern op_result int_or(int_value* self, const int_value* rhs);
extern op_result int_not(int_value* self);
extern op_result int_neg(int_value* self);

extern cmp_result int_cmp(const int_value* lhs, const int_value* rhs);

extern bool int_is_zero(const int_value* val);

extern uint8_t int_get_u8(const int_value* val);
extern uint16_t int_get_u16(const int_value* val);
extern uint32_t int_get_u32(const int_value* val);
extern uint64_t int_get_u64(const int_value* val);
extern int8_t int_get_i8(const int_value* val);
extern int16_t int_get_i16(const int_value* val);
extern int32_t int_get_i32(const int_value* val);
extern int64_t int_get_i64(const int_value* val);
extern uint64_t int_get_umax(const int_value* val);
extern int64_t int_get_imin(const int_value* val);
extern int64_t int_get_imax(const int_value* val);
extern float int_get_sp(const int_value* val);
extern double int_get_dp(const int_value* val);

extern void int_resize(int_value* val, uint bits);
extern float_value int_to_sp(const int_value* val);
extern float_value int_to_dp(const int_value* val);

extern int int_print(const int_value* val, char* buf, size_t count);
extern int int_print_as_hex(const int_value* val, char* buf, size_t count);

typedef struct _avalue
{
        bool isint;
        union
        {
                float_value as_float;
                int_value as_int;
        };
} avalue;

extern void avalue_init_int(avalue* self, uint bits, bool signed_, uint64_t val);
extern void avalue_init_sp(avalue* self, float v);
extern void avalue_init_dp(avalue* self, double v);

extern void avalue_set_signed(avalue* self, bool signed_);

extern op_result avalue_add(avalue* self, const avalue* rhs);
extern op_result avalue_sub(avalue* self, const avalue* rhs);
extern op_result avalue_mul(avalue* self, const avalue* rhs);
extern op_result avalue_div(avalue* self, const avalue* rhs);
extern op_result avalue_mod(avalue* self, const avalue* rhs);
extern op_result avalue_shl(avalue* self, const avalue* rhs);
extern op_result avalue_shr(avalue* self, const avalue* rhs);
extern op_result avalue_and(avalue* self, const avalue* rhs);
extern op_result avalue_xor(avalue* self, const avalue* rhs);
extern op_result avalue_or(avalue* self, const avalue* rhs);
extern op_result avalue_not(avalue* self);
extern op_result avalue_neg(avalue* self);

extern cmp_result avalue_cmp(const avalue* lhs, const avalue* rhs);

extern bool avalue_is_signed(const avalue* val);
extern bool avalue_is_zero(const avalue* val);
extern bool avalue_is_float(const avalue* val);
extern bool avalue_is_int(const avalue* val);

extern uint8_t avalue_get_u8(const avalue* val);
extern uint16_t avalue_get_u16(const avalue* val);
extern uint32_t avalue_get_u32(const avalue* val);
extern uint64_t avalue_get_u64(const avalue* val);
extern int8_t avalue_get_i8(const avalue* val);
extern int16_t avalue_get_i16(const avalue* val);
extern int32_t avalue_get_i32(const avalue* val);
extern int64_t avalue_get_i64(const avalue* val);
extern float avalue_get_sp(const avalue* val);
extern double avalue_get_dp(const avalue* val);
extern int_value avalue_get_int(const avalue* val);
extern float_value avalue_get_float(const avalue* val);

extern void avalue_to_int(avalue* val, uint bits, bool signed_);
extern void avalue_to_sp(avalue* val);
extern void avalue_to_dp(avalue* val);

extern int avalue_print(const avalue* self, char* buf, size_t count, int precision);
extern int avalue_print_as_hex(const avalue* val, char* buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif
