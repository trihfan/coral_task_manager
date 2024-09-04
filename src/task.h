#pragma once
#include <atomic>
#include <array>
#include <type_traits>
#include <cstdint>
#include <functional>
#include "Config.h"
#include "ArgumentPacker.h"
#include "FunctionTraits.h"
#include "TaskBuffer.h"

namespace coral::taskmanager
{
    static constexpr uint8_t AnyThreadIndex = std::numeric_limits<uint8_t>::max();

    struct Task;
    static constexpr int DataSize = config::TaskSizeBytes - sizeof(std::function<void(Task*, void*)>) - sizeof(Task*) - sizeof(std::atomic<int32_t>) - sizeof(uint8_t);

    // The task struct, contains the function and the data
    struct Task
    {
        Task* parent = nullptr;                     // Optional task parent
        std::function<void(Task*, void*)> function; // The task function
        std::atomic<int32_t> remaining = 0;         // Remaining work for the task (current + children)
        uint8_t threadIndex = AnyThreadIndex;       // Thread index to execute the task
        char data[DataSize];                        // Task data
    };

    // asserts
    static_assert(sizeof(Task) == config::TaskSizeBytes, "Wrong task size");
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");
}
