#pragma once
#include <cstdint>

namespace coral::task_manager
{
    struct task;
    using task_t = task*;

    class task_buffer
    {
    public:
        // Construction
        static void init(uint32_t thread_count);
        static void clear();

        // Allocate a task from the task buffer
        static task_t allocate();
    };
}