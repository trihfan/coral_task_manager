#include "random.h"

using namespace coral::taskmanager;

void SplitMix64::Init(uint64_t seed)
{
    x = seed;
}

uint64_t SplitMix64::Next()
{
    
    uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

void Xoshiro256Plus::Init(uint64_t seed)
{
    Random<SplitMix64>::Init(seed);
    s[0] = Random<SplitMix64>::Next();
    s[1] = Random<SplitMix64>::Next();
    s[2] = Random<SplitMix64>::Next();
    s[3] = Random<SplitMix64>::Next();
}

uint64_t Xoshiro256Plus::Next() 
{
    const uint64_t result = s[0] + s[3];
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = Rotl(s[3], 45);

    return result;
}

uint64_t Xoshiro256Plus::Rotl(const uint64_t x, int k) 
{
    return (x << k) | (x >> (64 - k));
}