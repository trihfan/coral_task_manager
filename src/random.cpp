#include "random.h"

using namespace coral::task_manager;

void split_mix_64::init(uint64_t seed)
{
    x = seed;
}

uint64_t split_mix_64::next()
{
    
    uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

void xoshiro_256_plus::init(uint64_t seed)
{
    random<split_mix_64>::init(seed);
    s[0] = random<split_mix_64>::next();
    s[1] = random<split_mix_64>::next();
    s[2] = random<split_mix_64>::next();
    s[3] = random<split_mix_64>::next();
}

uint64_t xoshiro_256_plus::next() 
{
    const uint64_t result = s[0] + s[3];
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 45);

    return result;
}

uint64_t xoshiro_256_plus::rotl(const uint64_t x, int k) 
{
    return (x << k) | (x >> (64 - k));
}