#pragma once
#include <cstdint>

namespace coral::task_manager
{
    /**
     * Random wrapper using the given Generator
     */
    template<typename Generator>
    class Random
    {
    public:
        // Initialize the generator with the given seed
        inline static void init(uint64_t seed) { return Generator::init(seed); }

        // Return a random number
        inline static uint64_t random() { return Generator::next(); }
    };

    // SplitMix64 generator
    class SplitMix64
    {
    public:
        static void init(uint64_t seed)
        {
            x = seed;
        }

        static uint64_t next()
        {
            
            uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
            z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
            z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
            return z ^ (z >> 31);
        }

    private:
        inline static thread_local uint64_t x;
    };

    // Xoshiro256+ generator
    class Xoshiro256plus
    {
    public:
        static void init(uint64_t seed)
        {
            Random<SplitMix64>::init(seed);
            s[0] = Random<SplitMix64>::random();
            s[1] = Random<SplitMix64>::random();
            s[2] = Random<SplitMix64>::random();
            s[3] = Random<SplitMix64>::random();
        }

        static uint64_t next() 
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

    private:
        inline static thread_local uint64_t s[4];

        inline static uint64_t rotl(const uint64_t x, int k) 
        {
            return (x << k) | (x >> (64 - k));
        }
    };    
}