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
        static void init(uint64_t seed);
        static uint64_t next();

    private:
        inline static thread_local uint64_t x;
    };

    // Xoshiro256+ generator
    class Xoshiro256plus
    {
    public:
        static void init(uint64_t seed);
        static uint64_t next();

    private:
        inline static thread_local uint64_t s[4];
        static uint64_t rotl(const uint64_t x, int k);
    };    
}