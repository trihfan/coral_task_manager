#pragma once
#include <cstdint>
#include "Task.h"

namespace coral::taskmanager
{
    // The task buffer is the buffer memory used to allocate tasks
    // It's an array of MaxTaskCount * ThreadCount of tasks
    class TaskBuffer
    {
    public:
        // Construction
        static void Init(size_t threadCount);
        static void Clear();

        // Allocate a task from the task buffer
        static TaskHandle Allocate();
        static TaskData* Get(TaskId id);
    };
}