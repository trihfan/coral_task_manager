#pragma once
#include <cstdint>

namespace coral::taskmanager
{
    // Random wrapper using the given generator
    template<typename Generator>
    class Random
    {
    public:
        // Initialize the generator with the given seed
        inline static void Init(uint64_t seed) { return Generator::Init(seed); }

        // Return a random number
        inline static uint64_t Next() { return Generator::Next(); }
    };

    // SplitMix64 generator
    class SplitMix64
    {
    public:
        static void Init(uint64_t seed);
        static uint64_t Next();

    private:
        inline static thread_local uint64_t x;
    };

    // Xoshiro256+ generator
    class Xoshiro256Plus
    {
    public:
        static void Init(uint64_t seed);
        static uint64_t Next();

    private:
        inline static thread_local uint64_t s[4];
        static uint64_t Rotl(const uint64_t x, int k);
    };    
}