#pragma once
#include <atomic>
#include <array>
#include <type_traits>
#include <cstdint>
#include <functional>
#include "Config.h"
#include "TaskBuffer.h"

namespace coral::taskmanager
{
    static constexpr uint8_t AnyThreadIndex = std::numeric_limits<uint8_t>::max();
    struct Task;

    // The task struct, contains the function and the data
    struct Task
    {
        std::function<void(Task*, void*)> function;     // The task function
        Task* parent = nullptr;                         // Optional task parent
        std::atomic<int32_t> remaining = 0;             // Remaining work for the task (current + children)
        uint8_t threadIndex = AnyThreadIndex;           // Thread index to execute the task
        std::atomic<uint8_t> continuationCount = 0;     // Number of tasks to start after this one
        std::array<Task*, 5> continuationTasks;         // Tasks to start after this one
        void* data;                                     // Task data
    };

    // asserts
    static_assert(sizeof(Task) == config::TaskSizeBytes, "Wrong task size");
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");
}
