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

    // Helper for user data
    template <typename... Items>
    struct UserData
    {
        template <size_t Index>
        static auto& Get(void* data) 
        { 
            auto tuple = reinterpret_cast<std::tuple<Items...>*>(data);
            return std::get<Index>(*tuple);
        }
    };
    
    // Constants
    static constexpr TaskHandle NullTask;
    static constexpr uint8_t AnyThreadIndex = std::numeric_limits<uint8_t>::max();

    // The task struct, contains the function and the data
    struct alignas(64) TaskData
    {
        std::function<void(TaskHandle, void*)> function;    // The task function 
        TaskHandle parent;                                  // Optional task parent
        std::atomic<int16_t> remaining;                     // Remaining work for the task (current + children)
        uint8_t threadIndex = AnyThreadIndex;               // Thread index to execute the task
        std::atomic<uint8_t> continuationCount;             // Number of tasks to start after this one
        std::array<TaskHandle, 16> continuationTasks;       // Tasks to start after this one 
        std::atomic<uint8_t> continuationInlining;          // Flag to inline a continuation task (-> the task will be executed by the same thread without going to the task queue)
        char userData[Config::TaskUserDataSizeBytes];    // User defined task data                                        
    };

    // asserts
#ifdef NDEBUG
    static constexpr size_t TaskHandleSize = sizeof(TaskHandle);
    static_assert(TaskHandleSize == 2, "Wrong task handle size");

    static constexpr size_t TaskDataSize = sizeof(TaskData);
    static_assert(TaskDataSize == Config::TaskSizeBytes, "Wrong task size");
#endif
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");
}
