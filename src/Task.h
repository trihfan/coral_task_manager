#pragma once
#include <atomic>
#include <array>
#include <type_traits>
#include <cstdint>
#include <functional>
#include "Config.h"

namespace coral::taskmanager
{
    struct TaskData;
    using TaskId = uint16_t;

    // Handle for task data, accessed using the task id
    struct TaskHandle
    {
        // Task id
        TaskId id = 0;

        // Return true if the task is not null
        operator bool() const;

        // Access task data
        TaskData* operator->();
        TaskData* operator*();

    #ifndef NDEBUG
        TaskData* data = nullptr;
    #endif
    };
    static constexpr size_t TaskSize = sizeof(TaskHandle);

    static constexpr TaskHandle NullTask;
    static constexpr uint8_t AnyThreadIndex = std::numeric_limits<uint8_t>::max();

    // The task struct, contains the function and the data
    struct alignas(std::hardware_destructive_interference_size) TaskData
    {
        std::function<void(TaskHandle, void*)> function;    // The task function 
        TaskHandle parent;                                  // Optional task parent
        std::atomic<int16_t> remaining;                     // Remaining work for the task (current + children)
        uint8_t threadIndex = AnyThreadIndex;               // Thread index to execute the task
        std::atomic<uint8_t> continuationCount;             // Number of tasks to start after this one
        std::array<TaskHandle, 16> continuationTasks;       // Tasks to start after this one 
        void* userData;                                     // User defined task data   
        std::atomic<uint8_t> continuationInlining;          // Flag to inline a continuation task (-> the task will be executed by the same thread without going to the task queue)
        char padding[15];                                                 
    };

    // asserts
#ifdef NDEBUG
    static constexpr size_t TaskDataSize = sizeof(TaskData);
    static_assert(TaskDataSize == config::TaskSizeBytes, "Wrong task size");
#endif
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");
}
