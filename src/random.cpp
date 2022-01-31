#include "random.h"

using namespace coral::task_manager;

void SplitMix64::init(uint64_t seed)
{
    x = seed;
}

uint64_t SplitMix64::next()
{
    
    uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

void Xoshiro256plus::init(uint64_t seed)
{
    Random<SplitMix64>::init(seed);
    s[0] = Random<SplitMix64>::random();
    s[1] = Random<SplitMix64>::random();
    s[2] = Random<SplitMix64>::random();
    s[3] = Random<SplitMix64>::random();
}

uint64_t Xoshiro256plus::next() 
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

uint64_t Xoshiro256plus::rotl(const uint64_t x, int k) 
{
    return (x << k) | (x >> (64 - k));
}