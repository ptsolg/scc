#include "scc/core/value.h"
#include "scc/core/misc.h"
#include <math.h>
#include <stdio.h>

extern void float_init_sp(float_value* self, float v)
{
        self->precision = FP_SINGLE;
        self->as_float = v;
}

extern void float_init_dp(float_value* self, double v)
{
        self->precision = FP_DOUBLE;
        self->as_double = v;
}

extern void float_set_sign(float_value* self, bool positive)
{
        if ((positive && !float_is_positive(self))
        || (!positive && float_is_positive(self)))
                float_neg(self);
}

extern float_precision float_get_precision(const float_value* self)
{
        return self->precision;
}

extern op_result float_add(float_value* self, const float_value* rhs)
{
        if (float_is_sp(self))
                self->as_float += float_get_sp(rhs);
        else
                self->as_double += float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_sub(float_value* self, const float_value* rhs)
{
        if (float_is_sp(self))
                self->as_float -= float_get_sp(rhs);
        else
                self->as_double -= float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_mul(float_value* self, const float_value* rhs)
{
        if (float_is_sp(self))
                self->as_float *= float_get_sp(rhs);
        else
                self->as_double *= float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_div(float_value* self, const float_value* rhs)
{
        if (float_is_zero(rhs))
                return OR_DIV_BY_ZERO;

        if (float_is_sp(self))
                self->as_float /= float_get_sp(rhs);
        else
                self->as_double /= float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_neg(float_value* self)
{
        if (float_is_sp(self))
                self->as_float = -float_get_sp(self);
        else
                self->as_double = -float_get_dp(self);

        return OR_OK;
}

extern cmp_result float_cmp(const float_value* lhs, const float_value* rhs)
{
        if (float_is_sp(lhs))
        {
                float lv = float_get_sp(lhs);
                float rv = float_get_sp(rhs);
                if (lv == rv)
                        return CR_EQ;
                return lv > rv ? CR_GR : CR_LE;
        }
        else
        {
                double lv = float_get_dp(lhs);
                double rv = float_get_dp(rhs);
                if (lv == rv)
                        return CR_EQ;
                return lv > rv ? CR_GR : CR_LE;
        }
}

extern bool float_is_positive(const float_value* val)
{
        if (float_is_sp(val))
                return float_get_sp(val) >= 0.0f;
        else
                return float_get_dp(val) >= 0.0;
}

extern bool float_is_zero(const float_value* val)
{
        if (float_is_sp(val))
                return float_get_sp(val) == 0.0f;
        else
                return float_get_dp(val) == 0.0;
}

extern bool float_is_inf(const float_value* val)
{
        if (float_is_sp(val))
                return isinf(float_get_sp(val));
        else
                return isinf(float_get_dp(val));
}

extern bool float_is_nan(const float_value* val)
{
        if (float_is_sp(val))
                return isnan(float_get_sp(val));
        else
                return isnan(float_get_dp(val));
}

extern bool float_is_sp(const float_value* val)
{
        return float_get_precision(val) == FP_SINGLE;
}

extern bool float_is_dp(const float_value* val)
{
        return float_get_precision(val) == FP_DOUBLE;
}

extern void float_to_dp(float_value* val)
{
        if (float_is_sp(val))
                float_init_dp(val, float_get_dp(val));
}

extern void float_to_sp(float_value* val)
{
        if (float_is_dp(val))
                float_init_sp(val, float_get_sp(val));
}

extern int_value float_to_int(const float_value* val, uint bits)
{
        uint64_t v = float_get_u64(val);
        int_value iv;
        int_init(&iv, bits, true, v);
        return iv;
}

extern float float_get_sp(const float_value* val)
{
        if (float_is_dp(val))
                return (float)val->as_double;
        else
                return val->as_float;

}

extern double float_get_dp(const float_value* val)
{
        if (float_is_sp(val))
                return (double)val->as_float;
        else
                return val->as_double;
}

extern uint8_t float_get_u8(const float_value* val)
{
        return (uint8_t)float_get_u64(val);
}

extern uint16_t float_get_u16(const float_value* val)
{
        return (uint16_t)float_get_u64(val);
}

extern uint32_t float_get_u32(const float_value* val)
{
        return (uint32_t)float_get_u64(val);
}

extern uint64_t float_get_u64(const float_value* val)
{
        if (float_is_sp(val))
                return (uint64_t)val->as_float;
        else
                return (uint64_t)val->as_double;
}

extern int8_t float_get_i8(const float_value* val)
{
        return (int8_t)float_get_i64(val);
}

extern int16_t float_get_i16(const float_value* val)
{
        return (int16_t)float_get_i64(val);
}

extern int32_t float_get_i32(const float_value* val)
{
        return (int32_t)float_get_i64(val);
}

extern int64_t float_get_i64(const float_value* val)
{
        if (float_is_sp(val))
                return (int64_t)val->as_float;
        else
                return (int64_t)val->as_double;
}

extern int float_print(const float_value* val, char* buf, size_t count, int precision)
{
        if (float_is_sp(val))
                return snprintf(buf, count, "%.*f", precision, float_get_sp(val));
        else
                return snprintf(buf, count, "%.*lf", precision, float_get_dp(val));
}

extern int float_print_as_hex(const float_value* val, char* buf, size_t count)
{
        union
        {
                double dp;
                uint64_t u64;
        } v = { .u64 = 0ULL };

        v.dp = float_get_dp(val);
        return snprintf(buf, count, "0x%016llX", v.u64);
}

extern void int_init(int_value* self, uint bits, bool signed_, uint64_t val)
{
        self->val = val;
        int_set_signed(self, signed_);
        int_resize(self, bits);
}

extern bool int_is_signed(const int_value* self)
{
        return self->issigned;
}

extern void int_set_signed(int_value* self, bool issigned)
{
        self->issigned = issigned;
}

extern uint int_get_bits(const int_value* self)
{
        return self->bits;
}

extern op_result int_add(int_value* self, const int_value* rhs)
{
        uint64_t x = int_get_u64(self);
        uint64_t y = int_get_u64(rhs);
        self->val = mod2(x + y, int_get_bits(self));
        return self->val < x ? OR_OVERFLOW : OR_OK;
}

extern op_result int_sub(int_value* self, const int_value* rhs)
{
        uint64_t x = int_get_u64(self);
        uint64_t y = int_get_u64(rhs);
        self->val = mod2(x - y, int_get_bits(self));
        return self->val > x ? OR_UNDERFLOW : OR_OK;
}

extern op_result int_mul(int_value* self, const int_value* rhs)
{
        uint64_t x = int_get_u64(self);
        uint64_t y = int_get_u64(rhs);
        self->val = mod2(x * y, int_get_bits(self));

        return x != 0 && self->val / x != y
                ? OR_OVERFLOW : OR_OK;
}

extern op_result int_div(int_value* self, const int_value* rhs)
{
        int64_t x = int_get_i64(self);
        int64_t y = int_get_i64(rhs);
        if (y == 0)
                return OR_DIV_BY_ZERO;

        self->val = mod2((uint64_t)(x / y), int_get_bits(self));
        return OR_OK;
}

extern op_result int_mod(int_value* self, const int_value* rhs)
{
        if (int_is_signed(self))
        {
                int64_t x = int_get_i64(self);
                int64_t y = int_get_i64(rhs);
                if (y == 0)
                        return OR_DIV_BY_ZERO;

                self->val = mod2((uint64_t)(x % y), int_get_bits(self));
                return OR_OK;
        }

        uint64_t x = int_get_u64(self);
        uint64_t y = int_get_u64(rhs);
        if (y == 0)
                return OR_DIV_BY_ZERO;

        self->val = mod2(x % y, int_get_bits(self));
        return OR_OK;
}

extern op_result int_shl(int_value* self, const int_value* rhs)
{
        uint64_t x = int_get_u64(self);
        uint64_t y = int_get_u64(rhs);
        self->val = mod2(x << y, int_get_bits(self));
        return self->val >> y == x ? OR_OK : OR_OVERFLOW;
}

extern op_result int_shr(int_value* self, const int_value* rhs)
{
        uint64_t x = int_get_u64(self);
        uint64_t y = int_get_u64(rhs);
        self->val = x >> y;
        return self->val << y == x ? OR_OK : OR_UNDERFLOW;
}

extern op_result int_and(int_value* self, const int_value* rhs)
{
        self->val &= int_get_u64(rhs);
        return OR_OK;
}

extern op_result int_xor(int_value* self, const int_value* rhs)
{
        self->val ^= mod2(int_get_u64(rhs), int_get_bits(self));
        return OR_OK;
}

extern op_result int_or(int_value* self, const int_value* rhs)
{
        self->val |= mod2(int_get_u64(rhs), int_get_bits(self));
        return OR_OK;
}

extern op_result int_not(int_value* self)
{
        self->val = mod2(~int_get_u64(self), int_get_bits(self));
        return OR_OK;
}

extern op_result int_neg(int_value* self)
{
        self->val = mod2((uint64_t)(-int_get_i64(self)), int_get_bits(self));
        return OR_OK;
}

extern cmp_result int_cmp(const int_value* lhs, const int_value* rhs)
{
        if (int_is_signed(lhs))
        {
                int64_t l = int_get_i64(lhs);
                int64_t r = int_get_i64(rhs);
                if (l == r)
                        return CR_EQ;
                return l > r ? CR_GR : CR_LE;
        }

        uint64_t l = int_get_u64(lhs);
        uint64_t r = int_get_u64(rhs);
        if (l == r)
                return CR_EQ;
        return l > r ? CR_GR : CR_LE;
}

extern bool int_is_zero(const int_value* val)
{
        return int_get_u64(val) == 0;
}

extern uint8_t int_get_u8(const int_value* val)
{
        return (uint8_t)int_get_u64(val);
}

extern uint16_t int_get_u16(const int_value* val)
{
        return (uint16_t)int_get_u64(val);
}

extern uint32_t int_get_u32(const int_value* val)
{
        return (uint32_t)int_get_u64(val);
}

extern uint64_t int_get_u64(const int_value* val)
{
        return val->val;
}

extern int8_t int_get_i8(const int_value* val)
{
        return (int8_t)int_get_i64(val);
}

extern int16_t int_get_i16(const int_value* val)
{
        return (int16_t)int_get_i64(val);
}

extern int32_t int_get_i32(const int_value* val)
{
        return (int32_t)int_get_i64(val);
}

extern int64_t int_get_i64(const int_value* val)
{
        const uint64_t mask = ((uint64_t)1) << (val->bits - 1);
        uint64_t v = int_get_u64(val);
        return -(int64_t)(v & mask) + (int64_t)(v & ~mask);
}

extern uint64_t int_get_umax(const int_value* val)
{
        return ((uint64_t)-1) >> (64 - val->bits);
}

extern int64_t int_get_imin(const int_value* val)
{
        return -(int64_t)(((uint64_t)1) << (val->bits - 1));
}

extern int64_t int_get_imax(const int_value* val)
{
        return int_get_umax(val) >> 1;
}

extern float int_get_sp(const int_value* val)
{
        return int_is_signed(val)
                ? (float)int_get_i64(val)
                : (float)int_get_u64(val);
}

extern double int_get_dp(const int_value* val)
{
        return int_is_signed(val)
                ? (double)int_get_i64(val)
                : (double)int_get_u64(val);
}

extern void int_resize(int_value* val, uint bits)
{
        assert(bits && bits <= 64);
        val->bits = bits;
        val->val = mod2(val->val, bits);
}

extern float_value int_to_sp(const int_value* val)
{
        float_value fv;
        float_init_sp(&fv, int_get_sp(val));
        return fv;
}

extern float_value int_to_dp(const int_value* val)
{
        float_value fv;
        float_init_dp(&fv, int_get_dp(val));
        return fv;
}

extern int int_print(const int_value* val, char* buf, size_t count)
{
        return snprintf(buf, count,
                (int_is_signed(val) ? "%lld" : "%llu"), int_get_u64(val));
}

extern int int_print_as_hex(const int_value* val, char* buf, size_t count)
{
        return snprintf(buf, count, "0x%llX", int_get_u64(val));
}

extern void avalue_init_int(avalue* self, uint bits, bool signed_, uint64_t val)
{
        self->isint = true;
        int_init(&self->as_int, bits, signed_, val);
}

extern void avalue_init_sp(avalue* self, float v)
{
        self->isint = false;
        float_init_sp(&self->as_float, v);
}

extern void avalue_init_dp(avalue* self, double v)
{
        self->isint = false;
        float_init_dp(&self->as_float, v);
}

extern void avalue_set_signed(avalue* self, bool signed_)
{
        assert(self->isint);
        int_set_signed(&self->as_int, signed_);
}

extern op_result avalue_add(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
                return int_add(&self->as_int, &r.as_int);
        }

        avalue_to_dp(&r);
        return float_add(&self->as_float, &r.as_float);
}

extern op_result avalue_sub(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
                return int_sub(&self->as_int, &r.as_int);
        }

        avalue_to_dp(&r);
        return float_sub(&self->as_float, &r.as_float);
}

extern op_result avalue_mul(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
                return int_mul(&self->as_int, &r.as_int);
        }

        avalue_to_dp(&r);
        return float_mul(&self->as_float, &r.as_float);
}

extern op_result avalue_div(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
                return int_div(&self->as_int, &r.as_int);
        }

        avalue_to_dp(&r);
        return float_div(&self->as_float, &r.as_float);
}

extern op_result avalue_mod(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
        return int_mod(&self->as_int, &r.as_int);
}

extern op_result avalue_shl(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
        return int_shl(&self->as_int, &r.as_int);
}

extern op_result avalue_shr(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
        return int_shr(&self->as_int, &r.as_int);
}

extern op_result avalue_and(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
        return int_and(&self->as_int, &r.as_int);
}

extern op_result avalue_xor(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
        return int_xor(&self->as_int, &r.as_int);
}

extern op_result avalue_or(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->as_int), int_is_signed(&self->as_int));
        return int_or(&self->as_int, &r.as_int);
}

extern op_result avalue_not(avalue* self)
{
        if (avalue_is_float(self))
                return OR_INVALID;
        else
                return int_not(&self->as_int);
}

extern op_result avalue_neg(avalue* self)
{
        if (avalue_is_float(self))
                return float_neg(&self->as_float);
        else
                return int_neg(&self->as_int);
}

extern cmp_result avalue_cmp(const avalue* lhs, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_float(lhs))
        {
                avalue_to_dp(&r);
                return float_cmp(&lhs->as_float, &r.as_float);
        }

        avalue_to_int(&r, int_get_bits(&lhs->as_int), int_is_signed(&lhs->as_int));
        return int_cmp(&lhs->as_int, &r.as_int);
}

extern bool avalue_is_signed(const avalue* val)
{
        return avalue_is_float(val) ? true : int_is_signed(&val->as_int);
}

extern bool avalue_is_zero(const avalue* val)
{
        if (avalue_is_float(val))
                return float_is_zero(&val->as_float);
        else
                return int_is_zero(&val->as_int);
}

extern bool avalue_is_float(const avalue* val)
{
        return !val->isint;
}

extern bool avalue_is_int(const avalue* val)
{
        return !avalue_is_float(val);
}

extern uint8_t avalue_get_u8(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u8(&val->as_float);
        else
                return int_get_u8(&val->as_int);
}

extern uint16_t avalue_get_u16(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u16(&val->as_float);
        else
                return int_get_u16(&val->as_int);
}

extern uint32_t avalue_get_u32(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u32(&val->as_float);
        else
                return int_get_u32(&val->as_int);
}

extern uint64_t avalue_get_u64(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u64(&val->as_float);
        else
                return int_get_u64(&val->as_int);
}

extern int8_t avalue_get_i8(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i8(&val->as_float);
        else
                return int_get_i8(&val->as_int);
}

extern int16_t avalue_get_i16(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i16(&val->as_float);
        else
                return int_get_i16(&val->as_int);
}

extern int32_t avalue_get_i32(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i32(&val->as_float);
        else
                return int_get_i32(&val->as_int);
}

extern int64_t avalue_get_i64(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i64(&val->as_float);
        else
                return int_get_i64(&val->as_int);
}

extern float avalue_get_sp(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_sp(&val->as_float);
        else
                return int_get_sp(&val->as_int);
}

extern double avalue_get_dp(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_dp(&val->as_float);
        else
                return int_get_dp(&val->as_int);
}

extern int_value avalue_get_int(const avalue* val)
{
        assert(avalue_is_int(val));
        return val->as_int;
}

extern float_value avalue_get_float(const avalue* val)
{
        assert(avalue_is_float(val));
        return val->as_float;
}

extern void avalue_to_int(avalue* val, uint bits, bool signed_)
{
        if (avalue_is_float(val))
        {
                val->isint = true;
                val->as_int = float_to_int(&val->as_float, bits);
        }
        else
        {
                int_set_signed(&val->as_int, signed_);
                int_resize(&val->as_int, bits);
        }
}

extern void avalue_to_sp(avalue* val)
{
        if (avalue_is_float(val))
                float_to_sp(&val->as_float);
        else
        {
                val->isint = false;
                val->as_float = int_to_sp(&val->as_int);
        }
}

extern void avalue_to_dp(avalue* val)
{
        if (avalue_is_float(val))
                float_to_dp(&val->as_float);
        else
        {
                val->isint = false;
                val->as_float = int_to_dp(&val->as_int);
        }
}

extern int avalue_print(const avalue* self, char* buf, size_t count, int precision)
{
        if (avalue_is_float(self))
                return float_print(&self->as_float, buf, count, precision);
        else
                return int_print(&self->as_int, buf, count);
}

extern int avalue_print_as_hex(const avalue* val, char* buf, size_t count)
{
        if (avalue_is_float(val))
                return float_print_as_hex(&val->as_float, buf, count);
        else
                return int_print_as_hex(&val->as_int, buf, count);
}