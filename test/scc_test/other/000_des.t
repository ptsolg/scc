typedef unsigned uint32_t;
typedef unsigned long long uint64_t;
typedef int bool;
typedef unsigned size_t;

static inline uint32_t set_bit32(uint32_t v, uint32_t n)
{
        return v | (1 << n);
}

static inline uint64_t set_bit64(uint64_t v, uint32_t n)
{
        return v | (1LL << n);
}

static inline bool check_bit32(uint32_t v, uint32_t n)
{
        return v & (1 << n);
}

static inline bool check_bit64(uint64_t v, uint32_t n)
{
        return v & (1LL << n);
}

static inline uint32_t upper64(uint64_t v)
{
        return (uint32_t)(v >> 32);
}

static inline uint32_t lower64(uint64_t v)
{
        return (uint32_t)(v << 32 >> 32);
}

static inline uint64_t rotl_64(uint64_t v, uint32_t n)
{
        return (v << n) | (v >> (64 - n));
}

static inline uint64_t rotl28_64(uint64_t v, uint32_t n)
{
        return ((v << n) | (v >> (28 - n))) & 0xFFFFFFF;
}

static inline uint64_t assemble64(uint32_t l, uint32_t r)
{
        return ((uint64_t)l << 32) | r;
}
