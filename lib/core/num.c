#include "scc/core/num.h"
#include "scc/core/common.h"
#include <math.h>
#include <stdio.h>

#define CHECK_OP(LHS, RHS)\
        assert(LHS && RHS \
            && LHS->kind == RHS->kind \
            && LHS->num_bits == RHS->num_bits \
            && "Numbers must have the same type.")

static inline uint64_t mod2(uint64_t x, unsigned pow)
{
        assert(pow <= 64);
        if (pow == 64)
                return x;

        uint64_t y = (((uint64_t)1) << pow);
        return (x & (y - 1));
}

static void int_resize(struct num* self, unsigned num_bits)
{
        assert(num_bits && num_bits <= 64 && num_is_integral(self));
        self->num_bits = num_bits;
        self->_u64 = mod2(self->_u64, num_bits);
}

void init_int(struct num* self, long long v, unsigned num_bits)
{
        self->kind = NUM_INT;
        self->_u64 = v;
        int_resize(self, num_bits);
}

void init_uint(struct num* self, unsigned long long v, unsigned num_bits)
{
        self->kind = NUM_UINT;
        self->_u64 = v;
        int_resize(self, num_bits);
}

void init_f32(struct num* self, float v)
{
        self->kind = NUM_FLOAT;
        self->_f32 = v;
        self->num_bits = 32;
}

void init_f64(struct num* self, double v)
{
        self->kind = NUM_FLOAT;
        self->_f64 = v;
        self->num_bits = 64;
}

static inline int is_f32(const struct num* self)
{
        return self->kind == NUM_FLOAT && self->num_bits == 32;
}

static inline int is_f64(const struct num* self)
{
        return self->kind == NUM_FLOAT && self->num_bits == 64;
}

int num_cmp(const struct num* lhs, const struct num* rhs)
{
        CHECK_OP(lhs, rhs);
        if (lhs->kind == NUM_INT)
        {
                int64_t l = num_i64(lhs);
                int64_t r = num_i64(rhs);
                return l == r
                        ? 0
                        : l < r ? -1 : 1;
        }
        else if (lhs->kind == NUM_UINT)
                return lhs->_u64 == rhs->_u64
                        ? 0
                        : lhs->_u64 < rhs->_u64 ? -1 : 1;
        else if (is_f32(lhs))
                return lhs->_f32 == rhs->_f32
                        ? 0
                        : lhs->_f32 < rhs->_f32 ? -1 : 1;
        else if (is_f64(lhs))
                return lhs->_f64 == rhs->_f64
                        ? 0
                        : lhs->_f64 < rhs->_f64 ? -1 : 1;
        UNREACHABLE();
        return -666;
}

op_result num_add(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                self->_u64 = mod2(x + y, self->num_bits);
                return self->_u64 < x ? OR_OVERFLOW : OR_OK;
        }
        else if (is_f32(self))
        {
                self->_f32 += rhs->_f32;
                return OR_OK;
        }
        else if (is_f64(self))
        {
                self->_f64 += rhs->_f64;
                return OR_OK;
        }
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_sub(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                self->_u64 = mod2(x - y, self->num_bits);
                return self->_u64 < x ? OR_OVERFLOW : OR_OK;
        }
        else if (is_f32(self))
        {
                self->_f32 -= rhs->_f32;
                return OR_OK;
        }
        else if (is_f64(self))
        {
                self->_f64 -= rhs->_f64;
                return OR_OK;
        }
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_mul(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                self->_u64 = mod2(x * y, self->num_bits);
                return x != 0 && self->_u64 / x != y 
                        ? OR_OVERFLOW : OR_OK;
        }
        else if (is_f32(self))
        {
                self->_f32 *= rhs->_f32;
                return OR_OK;
        }
        else if (is_f64(self))
        {
                self->_f64 *= rhs->_f64;
                return OR_OK;
        }
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_div(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                if (y == 0)
                        return OR_DIV_BY_ZERO;
                self->_u64 = mod2((uint64_t)(x / y), self->num_bits);
                return OR_OK;
        }
        else if (is_f32(self))
        {
                if (rhs->_f32 == 0.0f)
                        return OR_DIV_BY_ZERO;
                self->_f32 /= rhs->_f32;
                return OR_OK;
        }
        else if (is_f64(self))
        {
                if (rhs->_f64 == 0.0)
                        return OR_DIV_BY_ZERO;
                self->_f64 /= rhs->_f64;
                return OR_OK;
        }
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_mod(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (self->kind == NUM_INT)
        {
                int64_t x = num_i64(self);
                int64_t y = num_i64(rhs);
                if (y == 0)
                        return OR_DIV_BY_ZERO;
                self->_u64 = mod2((uint64_t)(x % y), self->num_bits);
                return OR_OK;
        }
        else if (self->kind == NUM_UINT)
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                if (y == 0)
                        return OR_DIV_BY_ZERO;
                self->_u64 = mod2(x % y, self->num_bits);
                return OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_bit_shl(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                self->_u64 = mod2(x << y, self->num_bits);
                return self->_u64 >> y != x ? OR_OVERFLOW : OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_bit_shr(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                uint64_t x = self->_u64;
                uint64_t y = rhs->_u64;
                self->_u64 = mod2(x >> y, self->num_bits);
                return self->_u64 << y != x ? OR_UNDERFLOW : OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_bit_and(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                self->_u64 &= rhs->_u64;
                return OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_bit_xor(struct num* self, const struct num* rhs)
{
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                self->_u64 ^= mod2(rhs->_u64, rhs->num_bits);
                return OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_bit_or(struct num* self,  const struct num* rhs)
{
        CHECK_OP(self, rhs);
        CHECK_OP(self, rhs);
        if (num_is_integral(self))
        {
                self->_u64 |= mod2(rhs->_u64, rhs->num_bits);
                return OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_bit_neg(struct num* self)
{
        if (num_is_integral(self))
        {
                self->_u64 = mod2(~self->_u64, self->num_bits);
                return OR_OK;
        }
        else if (self->kind == NUM_FLOAT)
                return OR_INVALID;
        UNREACHABLE();
        return OR_INVALID;
}

op_result num_neg(struct num* self)
{
        if (self->kind == NUM_INT)
        {
                self->_u64 = mod2((uint64_t)-num_i64(self), self->num_bits);
                return OR_OK;
        }
        else if (self->kind == NUM_UINT)
        {
                self->_u64 = mod2(-self->_u64, self->num_bits);
                return OR_OK;
        }
        else if (is_f32(self))
        {
                self->_f32 = -self->_f32;
                return OR_OK;
        }
        else if (is_f64(self))
        {
                self->_f64 = -self->_f64;
                return OR_OK;
        }
        UNREACHABLE();
        return OR_INVALID;
}

void num_to_int(struct num* self, unsigned num_bits)
{
        if (num_is_integral(self))
        {
                self->kind = NUM_INT;
                self->num_bits = num_bits;
                self->_u64 = mod2(self->_u64, num_bits);         
        }
        else if (is_f32(self))
                init_int(self, (int64_t)self->_f32, num_bits);
        else if (is_f64(self))
                init_int(self, (int64_t)self->_f64, num_bits);
        else
                UNREACHABLE();
}

void num_to_uint(struct num* self, unsigned num_bits)
{
        if (num_is_integral(self))
        {
                self->kind = NUM_UINT;
                self->num_bits = num_bits;
                self->_u64 = mod2(self->_u64, num_bits);
        }
        else if (is_f32(self))
                init_uint(self, (uint64_t)self->_f32, num_bits);
        else if (is_f64(self))
                init_uint(self, (uint64_t)self->_f64, num_bits);
        else
                UNREACHABLE();
}

void num_to_f32(struct num* self)
{
        if (self->kind == NUM_INT)
                init_f32(self, (float)num_i64(self));
        else if (self->kind == NUM_UINT)
                init_f32(self, (float)self->_u64);   
        else if (is_f32(self))
                return;
        else if (is_f64(self))
                init_f32(self, (float)self->_f64);
        else
                UNREACHABLE();
}

void num_to_f64(struct num* self)
{
        if (self->kind == NUM_INT)
                init_f64(self, (double)num_i64(self));
        else if (self->kind == NUM_UINT)
                init_f64(self, (double)self->_u64);
        else if (is_f32(self))
                init_f64(self, (double)self->_f32);
        else if (is_f64(self))
                return;
        else
                UNREACHABLE();
}

int num_is_zero(const struct num* self)
{
        if (num_is_integral(self))
                return self->_u64 == 0;
        else if (self->kind == NUM_FLOAT)
        {
                assert(self->num_bits == 32 || self->num_bits == 64);
                return self->num_bits == 32
                        ? self->_f32 == 0.0f
                        : self->_f64 == 0.0;
        }
        UNREACHABLE();
        return 0;
}

int num_is_integral(const struct num* self)
{
        return self->kind == NUM_INT || self->kind == NUM_UINT;
}

void num_to_str(const struct num* self, char* buf, size_t len)
{
        if (self->kind == NUM_INT)
                snprintf(buf, len, "%lld", num_i64(self));
        else if (self->kind == NUM_UINT)
                snprintf(buf, len, "%llu", self->_u64);
        else if (self->kind == NUM_FLOAT)
                snprintf(buf, len, "%lf", num_f64(self));
        else
                UNREACHABLE();
}

void num_to_hex(const struct num* self, char* buf, size_t len)
{
        union 
        {
                uint64_t x;
                double y;
        } z = { .x = self->_u64 };
        if (is_f32(self))
                z.y = self->_f32;

        snprintf(buf, len, "0x%016llX", z.x);
}

long long num_i64(const struct num* self)
{
        assert(self->kind == NUM_INT);
        uint64_t mask = ((uint64_t)1) << (self->num_bits - 1);
        return -(int64_t)(self->_u64 & mask) + (int64_t)(self->_u64 & ~mask);
}

unsigned long long num_u64(const struct num* self)
{
        assert(self->kind == NUM_UINT);
        return self->_u64;
}

unsigned long long num_as_u64(const struct num* self)
{
        struct num x = *self;
        num_to_uint(&x, self->num_bits);
        return x._u64;
}

float num_f32(const struct num* self)
{
        assert(self->kind == NUM_FLOAT);
        return self->num_bits == 32 ? self->_f32 : self->_f64;
}

double num_f64(const struct num* self)
{
        assert(self->kind == NUM_FLOAT);
        return self->num_bits == 32 ? self->_f32 : self->_f64;
}
