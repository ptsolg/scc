#ifndef NUM_H
#define NUM_H

#include "common.h"

typedef enum
{
        OR_OK,
        OR_DIV_BY_ZERO,
        OR_OVERFLOW,
        OR_UNDERFLOW,
        OR_INVALID,
} op_result;

typedef enum
{
        NUM_INT,
        NUM_UINT,
        NUM_FLOAT,
} num_kind;

struct num
{
        unsigned kind : 2;
        unsigned num_bits : 7;
        union
        {
                unsigned long long _u64;
                float _f32;
                double _f64;
        };
};

void init_int(struct num* self, long long v, unsigned num_bits);
void init_uint(struct num* self, unsigned long long v, unsigned num_bits);
void init_f32(struct num* self, float v);
void init_f64(struct num* self, double v);
int num_cmp(const struct num* lhs, const struct num* rhs);
op_result num_add(struct num* self, const struct num* rhs);
op_result num_sub(struct num* self, const struct num* rhs);
op_result num_mul(struct num* self, const struct num* rhs);
op_result num_div(struct num* self, const struct num* rhs);
op_result num_mod(struct num* self, const struct num* rhs);
op_result num_bit_shl(struct num* self, const struct num* rhs);
op_result num_bit_shr(struct num* self, const struct num* rhs);
op_result num_bit_and(struct num* self, const struct num* rhs);
op_result num_bit_xor(struct num* self, const struct num* rhs);
op_result num_bit_or(struct num* self,  const struct num* rhs);
op_result num_bit_neg(struct num* self);
op_result num_neg(struct num* self);
void num_to_int(struct num* self, unsigned num_bits);
void num_to_uint(struct num* self, unsigned num_bits);
void num_to_f32(struct num* self);
void num_to_f64(struct num* self);
int num_is_zero(const struct num* self);
int num_is_integral(const struct num* self);
void num_to_str(const struct num* self, char* buf, size_t len);
void num_to_hex(const struct num* self, char* buf, size_t len);
long long num_i64(const struct num* self);
unsigned long long num_u64(const struct num* self);
unsigned long long num_as_u64(const struct num* self);
float num_f32(const struct num* self);
double num_f64(const struct num* self);

#endif
