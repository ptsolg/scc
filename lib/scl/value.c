#include "scc/scl/value.h"
#include "scc/scl/misc.h"
#include <math.h>

extern void float_init_sp(float_value* self, float v)
{
        self->_precision = FP_SINGLE;
        self->_float = v;
}

extern void float_init_dp(float_value* self, double v)
{
        self->_precision = FP_DOUBLE;
        self->_double = v;
}

extern void float_set_sign(float_value* self, bool positive)
{
        if ((positive && !float_is_positive(self))
        || (!positive && float_is_positive(self)))
                float_neg(self);
}

extern float_precision float_get_precision(const float_value* self)
{
        return self->_precision;
}

extern op_result float_add(float_value* self, const float_value* rhs)
{
        if (float_is_sp(self))
                self->_float += float_get_sp(rhs);
        else
                self->_double += float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_sub(float_value* self, const float_value* rhs)
{
        if (float_is_sp(self))
                self->_float -= float_get_sp(rhs);
        else
                self->_double -= float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_mul(float_value* self, const float_value* rhs)
{
        if (float_is_sp(self))
                self->_float *= float_get_sp(rhs);
        else
                self->_double *= float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_div(float_value* self, const float_value* rhs)
{
        if (float_is_zero(rhs))
                return OR_DIV_BY_ZERO;

        if (float_is_sp(self))
                self->_float /= float_get_sp(rhs);
        else
                self->_double /= float_get_dp(rhs);

        return OR_OK;
}

extern op_result float_neg(float_value* self)
{
        if (float_is_sp(self))
                self->_float = -float_get_sp(self);
        else
                self->_double = -float_get_dp(self);

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
        suint64 v = float_get_u64(val);
        int_value iv;
        int_init(&iv, bits, true, v);
        return iv;
}

extern float float_get_sp(const float_value* val)
{
        if (float_is_dp(val))
                return (float)val->_double;
        else
                return val->_float;

}

extern double float_get_dp(const float_value* val)
{
        if (float_is_sp(val))
                return (double)val->_float;
        else
                return val->_double;
}

extern suint8 float_get_u8(const float_value* val)
{
        return (suint8)float_get_u64(val);
}

extern suint16 float_get_u16(const float_value* val)
{
        return (suint16)float_get_u64(val);
}

extern suint32 float_get_u32(const float_value* val)
{
        return (suint32)float_get_u64(val);
}

extern suint64 float_get_u64(const float_value* val)
{
        if (float_is_sp(val))
                return (suint64)val->_float;
        else
                return (suint64)val->_double;
}

extern sint8 float_get_i8(const float_value* val)
{
        return (sint8)float_get_i64(val);
}

extern sint16 float_get_i16(const float_value* val)
{
        return (sint16)float_get_i64(val);
}

extern sint32 float_get_i32(const float_value* val)
{
        return (sint32)float_get_i64(val);
}

extern sint64 float_get_i64(const float_value* val)
{
        if (float_is_sp(val))
                return (sint64)val->_float;
        else
                return (sint64)val->_double;
}

extern void int_init(int_value* self, uint bits, bool signed_, suint64 val)
{
        self->_val = val;
        int_set_signed(self, signed_);
        int_resize(self, bits);
}

extern void int_set_signed(int_value* self, bool signed_)
{
        self->_signed = signed_;
}

extern uint int_get_bits(const int_value* self)
{
        return self->_bits;
}

extern bool int_is_signed(const int_value* self)
{
        return self->_signed;
}

extern op_result int_add(int_value* self, const int_value* rhs)
{
        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        self->_val = mod2(x + y, int_get_bits(self));
        return self->_val < x ? OR_OVERFLOW : OR_OK;
}

extern op_result int_sub(int_value* self, const int_value* rhs)
{
        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        self->_val = mod2(x - y, int_get_bits(self));
        return self->_val > x ? OR_UNDERFLOW : OR_OK;
}

extern op_result int_mul(int_value* self, const int_value* rhs)
{
        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        self->_val = mod2(x * y, int_get_bits(self));

        return x != 0 && self->_val / x != y
                ? OR_OVERFLOW : OR_OK;
}

extern op_result int_div(int_value* self, const int_value* rhs)
{
        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        if (y == 0)
                return OR_DIV_BY_ZERO;

        self->_val = mod2(x / y, int_get_bits(self));
        return OR_OK;
}

extern op_result int_mod(int_value* self, const int_value* rhs)
{
        if (int_is_signed(self))
        {
                sint64 x = int_get_i64(self);
                sint64 y = int_get_i64(rhs);
                if (y == 0)
                        return OR_DIV_BY_ZERO;

                self->_val = mod2((suint64)(x % y), int_get_bits(self));
                return OR_OK;
        }

        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        if (y == 0)
                return OR_DIV_BY_ZERO;

        self->_val = mod2(x % y, int_get_bits(self));
        return OR_OK;
}

extern op_result int_shl(int_value* self, const int_value* rhs)
{
        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        self->_val = mod2(x << y, int_get_bits(self));
        return self->_val >> y == x ? OR_OK : OR_OVERFLOW;
}

extern op_result int_shr(int_value* self, const int_value* rhs)
{
        suint64 x = int_get_u64(self);
        suint64 y = int_get_u64(rhs);
        self->_val = x >> y;
        return self->_val << y == x ? OR_OK : OR_UNDERFLOW;
}

extern op_result int_and(int_value* self, const int_value* rhs)
{
        self->_val &= int_get_u64(rhs);
        return OR_OK;
}

extern op_result int_xor(int_value* self, const int_value* rhs)
{
        self->_val ^= mod2(int_get_u64(rhs), int_get_bits(self));
        return OR_OK;
}

extern op_result int_or(int_value* self, const int_value* rhs)
{
        self->_val |= mod2(int_get_u64(rhs), int_get_bits(self));
        return OR_OK;
}

extern op_result int_not(int_value* self)
{
        self->_val = mod2(~int_get_u64(self), int_get_bits(self));
        return OR_OK;
}

extern op_result int_neg(int_value* self)
{
        self->_val = mod2(-int_get_u64(self), int_get_bits(self));
        return OR_OK;
}

extern cmp_result int_cmp(const int_value* lhs, const int_value* rhs)
{
        if (int_is_signed(lhs))
        {
                sint64 l = int_get_i64(lhs);
                sint64 r = int_get_i64(rhs);
                if (l == r)
                        return CR_EQ;
                return l > r ? CR_GR : CR_LE;
        }

        suint64 l = int_get_u64(lhs);
        suint64 r = int_get_u64(rhs);
        if (l == r)
                return CR_EQ;
        return l > r ? CR_GR : CR_LE;
}

extern bool int_is_zero(const int_value* val)
{
        return int_get_u64(val) == 0;
}

extern suint8 int_get_u8(const int_value* val)
{
        return (suint8)int_get_u64(val);
}

extern suint16 int_get_u16(const int_value* val)
{
        return (suint16)int_get_u64(val);
}

extern suint32 int_get_u32(const int_value* val)
{
        return (suint32)int_get_u64(val);
}

extern suint64 int_get_u64(const int_value* val)
{
        return val->_val;
}

extern sint8 int_get_i8(const int_value* val)
{
        return (sint8)int_get_i64(val);
}

extern sint16 int_get_i16(const int_value* val)
{
        return (sint16)int_get_i64(val);
}

extern sint32 int_get_i32(const int_value* val)
{
        return (sint32)int_get_i64(val);
}

extern sint64 int_get_i64(const int_value* val)
{
        const suint64 mask = ((suint64)1) << (val->_bits - 1);
        suint64 v = int_get_u64(val);
        return -(sint64)(v & mask) + (sint64)(v & ~mask);
}

extern suint64 int_get_umax(const int_value* val)
{
        return ((suint64)-1) >> (64 - val->_bits);
}

extern sint64 int_get_imin(const int_value* val)
{
        return -(sint64)(((suint64)1) << (val->_bits - 1));
}

extern sint64 int_get_imax(const int_value* val)
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
        S_ASSERT(bits && bits <= 64);
        val->_bits = bits;
        val->_val = mod2(val->_val, bits);
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

extern void avalue_init_int(avalue* self, uint bits, bool signed_, suint64 val)
{
        self->_integer = true;
        int_init(&self->_int, bits, signed_, val);
}

extern void avalue_init_sp(avalue* self, float v)
{
        self->_integer = false;
        float_init_sp(&self->_float, v);
}

extern void avalue_init_dp(avalue* self, double v)
{
        self->_integer = false;
        float_init_dp(&self->_float, v);
}

extern op_result avalue_add(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
                return int_add(&self->_int, &r._int);
        }

        avalue_to_dp(&r);
        return float_add(&self->_float, &r._float);
}

extern op_result avalue_sub(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
                return int_sub(&self->_int, &r._int);
        }

        avalue_to_dp(&r);
        return float_sub(&self->_float, &r._float);
}

extern op_result avalue_mul(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
                return int_mul(&self->_int, &r._int);
        }

        avalue_to_dp(&r);
        return float_mul(&self->_float, &r._float);
}

extern op_result avalue_div(avalue* self, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_int(self))
        {
                avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
                return int_div(&self->_int, &r._int);
        }

        avalue_to_dp(&r);
        return float_div(&self->_float, &r._float);
}

extern op_result avalue_mod(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
        return int_mod(&self->_int, &r._int);
}

extern op_result avalue_shl(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
        return int_shl(&self->_int, &r._int);
}

extern op_result avalue_shr(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
        return int_shr(&self->_int, &r._int);
}

extern op_result avalue_and(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
        return int_and(&self->_int, &r._int);
}

extern op_result avalue_xor(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
        return int_xor(&self->_int, &r._int);
}

extern op_result avalue_or(avalue* self, const avalue* rhs)
{
        if (avalue_is_float(rhs))
                return OR_INVALID;

        avalue r = *rhs;
        avalue_to_int(&r, int_get_bits(&self->_int), int_is_signed(&self->_int));
        return int_or(&self->_int, &r._int);
}

extern op_result avalue_not(avalue* self)
{
        if (avalue_is_float(self))
                return OR_INVALID;
        else
                return int_not(&self->_int);
}

extern op_result avalue_neg(avalue* self)
{
        if (avalue_is_float(self))
                return float_neg(&self->_float);
        else
                return int_neg(&self->_int);
}

extern cmp_result avalue_cmp(const avalue* lhs, const avalue* rhs)
{
        avalue r = *rhs;
        if (avalue_is_float(lhs))
        {
                avalue_to_dp(&r);
                return float_cmp(&lhs->_float, &r._float);
        }

        avalue_to_int(&r, int_get_bits(&lhs->_int), int_is_signed(&lhs->_int));
        return int_cmp(&lhs->_int, &r._int);
}

extern bool avalue_is_signed(const avalue* val)
{
        return avalue_is_float(val) ? true : int_is_signed(&val->_int);
}

extern bool avalue_is_zero(const avalue* val)
{
        if (avalue_is_float(val))
                return float_is_zero(&val->_float);
        else
                return int_is_zero(&val->_int);
}

extern bool avalue_is_float(const avalue* val)
{
        return !val->_integer;
}

extern bool avalue_is_int(const avalue* val)
{
        return !avalue_is_float(val);
}

extern suint8 avalue_get_u8(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u8(&val->_float);
        else
                return int_get_u8(&val->_int);
}

extern suint16 avalue_get_u16(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u16(&val->_float);
        else
                return int_get_u16(&val->_int);
}

extern suint32 avalue_get_u32(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u32(&val->_float);
        else
                return int_get_u32(&val->_int);
}

extern suint64 avalue_get_u64(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_u64(&val->_float);
        else
                return int_get_u64(&val->_int);
}

extern sint8 avalue_get_i8(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i8(&val->_float);
        else
                return int_get_i8(&val->_int);
}

extern sint16 avalue_get_i16(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i16(&val->_float);
        else
                return int_get_i16(&val->_int);
}

extern sint32 avalue_get_i32(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i32(&val->_float);
        else
                return int_get_i32(&val->_int);
}

extern sint64 avalue_get_i64(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_i64(&val->_float);
        else
                return int_get_i64(&val->_int);
}

extern float avalue_get_sp(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_sp(&val->_float);
        else
                return int_get_sp(&val->_int);
}

extern double avalue_get_dp(const avalue* val)
{
        if (avalue_is_float(val))
                return float_get_dp(&val->_float);
        else
                return int_get_dp(&val->_int);
}
extern int_value avalue_get_int(const avalue* val)
{
        S_ASSERT(avalue_is_int(val));
        return val->_int;
}

extern float_value avalue_get_float(const avalue* val)
{
        S_ASSERT(avalue_is_float(val));
        return val->_float;
}

extern void avalue_to_int(avalue* val, uint bits, bool signed_)
{
        if (avalue_is_float(val))
        {
                val->_integer = true;
                val->_int = float_to_int(&val->_float, bits);
        }
        else
        {
                int_set_signed(&val->_int, signed_);
                int_resize(&val->_int, bits);
        }
}

extern void avalue_to_sp(avalue* val)
{
        if (avalue_is_float(val))
                float_to_sp(&val->_float);
        else
        {
                val->_integer = false;
                val->_float = int_to_sp(&val->_int);
        }
}

extern void avalue_to_dp(avalue* val)
{
        if (avalue_is_float(val))
                float_to_dp(&val->_float);
        else
        {
                val->_integer = false;
                val->_float = int_to_dp(&val->_int);
        }
}