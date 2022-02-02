#pragma once
#include <cstdint>
#include "api.h"

namespace coral::task_manager
{
    // Random wrapper using the given generator
    template<typename Generator>
    class random
    {
    public:
        // Initialize the generator with the given seed
        inline static void init(uint64_t seed) { return Generator::init(seed); }

        // Return a random number
        inline static uint64_t next() { return Generator::next(); }
    };

    // SplitMix64 generator
    class split_mix_64
    {
    public:
        api static void init(uint64_t seed);
        api static uint64_t next();

    private:
        inline static thread_local uint64_t x;
    };

    // Xoshiro256+ generator
    class xoshiro_256_plus
    {
    public:
        api static void init(uint64_t seed);
        api static uint64_t next();

    private:
        inline static thread_local uint64_t s[4];
        api static uint64_t rotl(const uint64_t x, int k);
    };    
}