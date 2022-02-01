#pragma once
#include <cstdint>

// config
namespace coral::task_manager
{
    struct config
    {
        // Const parameters
        static constexpr uint32_t task_size_bytes = 64;

        // Maxmimum task count in the task manager
        static void set_max_task_count(uint32_t count)
        {
            // max_task_count must be a power of two
            assert(count % 2 == 0);
            max_task_count = count;
            max_task_count_mask = max_task_count - 1u;
        }

        // Return the max task count value
        static uint32_t get_max_task_count()
        {
            return max_task_count;
        }

        // Return the max task count mask
        static uint32_t get_max_task_count_mask()
        {
            return max_task_count_mask;
        }

    private:
        inline static uint32_t max_task_count = 4096u;
        inline static uint32_t max_task_count_mask = max_task_count - 1u;
    };
}
