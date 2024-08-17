#pragma once
#include <atomic>
#include <array>
#include <type_traits>
#include <cstdint>
#include "config.h"
#include "argument_packer.h"
#include "function_traits.h"
#include "task_buffer.h"

#include <iostream>

namespace coral::task_manager
{
    struct task;
    typedef void (*task_function)(task*, const void*);
    static constexpr int data_size = config::task_size_bytes - sizeof(task_function) - sizeof(task*) - sizeof(std::atomic<int32_t>) - sizeof(uint8_t);

    static constexpr uint8_t ANY_THREAD_INDEX = std::numeric_limits<uint8_t>::max();

    // The task struct, contains the function and the data
    struct task
    {
        task_function function;                 // task function to execute
        task* parent = nullptr;                 // optional task parent
        std::atomic<int32_t> remaining = 0;     // remaining work for the task (current + children)
        char data[data_size];                   // task data
        uint8_t threadIndex = ANY_THREAD_INDEX;                
    };
    using task_t = task*;

    // asserts
    static_assert(sizeof(task) == config::task_size_bytes, "Wrong task size");
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");

    /************************************************************/
    // Call the function getting the arguments from the data buffer
    template <typename Function, std::size_t... IndexSequence>
    inline void apply(const Function function, const void* data, std::index_sequence<IndexSequence...>)
    {
        using traits = function_traits<Function>;
        function(task_manager::items::get<IndexSequence, std::tuple_element_t<IndexSequence, typename traits::arguments>...>(data)...);
    }

    // Create task executing a function with the given arguments
    template <typename Function, typename... Args>
    task_t create_task(const Function function, Args... args)
    {
        static_assert(sizeof(decltype(function)) == 1, "A task function can't contains captured values");
        static const thread_local auto copy = function;
        auto task = task_buffer::allocate();
        task->function = [](auto task, auto data) { apply(copy, data, std::make_index_sequence<sizeof...(Args)>{}); };
        task->parent = nullptr;
        task->remaining.store(1, std::memory_order_relaxed);
        items::copy(task->data, std::forward<Args>(args)...);
        return task;
    }

    // Helper to create batch
    inline task_t create_task() 
    { 
        return create_task([](){}); 
    } 

    // Create child task
    template <typename Function, typename... Args>
    task_t create_child_task(task_t parent, const Function function, Args&&... args)
    {
        auto task = create_task(function, std::forward<Args>(args)...);
        task->parent = parent;
        parent->remaining.fetch_add(1, std::memory_order_relaxed);
        return task;
    }
}
