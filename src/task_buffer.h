#pragma once
#include <cstdint>
#include "api.h"

namespace coral::task_manager
{
    struct task;
    using task_t = task*;

    class task_buffer
    {
    public:
        // Construction
        api static void init(uint32_t thread_count);
        api static void clear();

        // Allocate a task from the task buffer
        api static task_t allocate();
    };
}