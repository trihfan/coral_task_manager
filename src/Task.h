#pragma once
#include <atomic>
#include <functional>
#include <array>
#include "config.h"
#include "ArgumentPacker.h"

namespace coral::task_manager
{
    struct Task;
    typedef void (*task_function)(const void*);
    static constexpr int DATA_SIZE = taskSizeBytes - sizeof(task_function) - sizeof(Task*) - sizeof(std::atomic<int32_t>);

    /**
     * The task struct, contains the function and the data
     */
    struct Task
    {
        task_function function;             // task function to execute
        Task* parent;                       // optional task parent
        std::atomic<int32_t> remaining;     // remaining work for the task (current + children)
        char data[DATA_SIZE];               // task data
    };

    // asserts
    static_assert(sizeof(Task) == taskSizeBytes, "Wrong task size");
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");
    
    // task buffer
    struct TaskInternal
    {
        static Task* allocateTask() 
        {
            const uint32_t index = currentIndex++;
            return &taskBuffer[(index - 1u) & (maxTaskCount - 1u)];
        }

        inline static thread_local std::array<Task, maxTaskCount> taskBuffer;
        inline static thread_local uint32_t currentIndex = 0;
    };

    inline bool isFinished(const Task* task)
    {
        return task->remaining.load(std::memory_order_relaxed) == 0;
    }

    // Create an empty task
    template <typename... Args>
    Task* createTask(task_function function, Args&&... args)
    {
        Task* task = TaskInternal::allocateTask();
        task->function = function;
        task->parent = nullptr;
        task->remaining.store(1, std::memory_order_relaxed);
        Items<Args...>::copy(task->data, args...);
        return task;
    }

    inline Task* createTask() 
    { 
        return createTask([](auto){}); 
    } 

    inline Task* createChildTask(Task* parent, task_function function)
    {
        auto task = createTask(function);
        task->parent = parent;
        parent->remaining.fetch_add(1, std::memory_order::memory_order_relaxed);
        return task;
    }

    template <typename... Args>
    Task* createChildTask(Task* parent, task_function function, Args&&... args)
    {
        auto task = createChildTask(parent, function);
        Items<Args...>::copy(task->data, args...);
        return task;
    }

}
