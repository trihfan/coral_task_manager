#pragma once
#include <cstdint>

namespace coral::taskmanager
{
    struct Task;

    // The task buffer is the buffer memory used to allocate tasks
    // It's an array of MaxTaskCount * ThreadCount of tasks
    class TaskBuffer
    {
    public:
        // Construction
        static void Init(uint32_t threadCount);
        static void Clear();

        // Allocate a task from the task buffer
        static Task* Allocate();
    };
}