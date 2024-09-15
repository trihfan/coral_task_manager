#pragma once
#include <cstdint>
#include <cassert>

// config
namespace coral::taskmanager
{
    struct Config
    {
        // Const parameters
        static constexpr uint32_t TaskSizeBytes = 64 * 2;
        static constexpr uint32_t TaskUserDataSizeBytes = 23;

        // Maxmimum task count in the task manager
        static void SetMaxTaskCount(uint32_t count)
        {
            // maxTaskCount must be a power of two so we can use it like a modulo
            assert(count % 2 == 0);
            maxTaskCount = count;
            maxTaskCountMask = maxTaskCount - 1u;
        }

        // Return the max task count value
        static uint32_t GetMaxTaskCount()
        {
            return maxTaskCount;
        }

        // Return the mask to iterate a task buffer (buffer[index & maxTaskCount])
        static uint32_t GetMaxTaskCountMask()
        {
            return maxTaskCountMask;
        }

    private:
        inline static uint32_t maxTaskCount = 4096u;
        inline static uint32_t maxTaskCountMask = maxTaskCount - 1u;
    };
}
