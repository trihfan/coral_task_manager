#pragma once
#include <atomic>
#include <array>
#include "config.h"
#include "argument_packer.h"
#include "function_traits.h"

namespace coral::task_manager
{
    struct task;
    typedef void (*task_function)(task*, const void*);
    static constexpr int data_size = config::taskSizeBytes - sizeof(task_function) - sizeof(task*) - sizeof(std::atomic<int32_t>);

    /**
     * The task struct, contains the function and the data
     */
    struct task
    {
        task_function function;             // task function to execute
        task* parent;                       // optional task parent
        std::atomic<int32_t> remaining;     // remaining work for the task (current + children)
        char data[data_size];               // task data
    };

    // asserts
    static_assert(sizeof(task) == config::taskSizeBytes, "Wrong task size");
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");
    
    // task buffer
    struct task_buffer
    {
        static task* allocate() 
        {
            const uint32_t i = index++;
            assert(buffer[i & config::maxTaskCountMask].remaining == 0);
            return &buffer[i & config::maxTaskCountMask];
        }

        inline static thread_local std::array<task, config::maxTaskCount> buffer;
        inline static thread_local uint32_t index = 0;
    };

    /************************************************************/
    // Call the function getting the arguments from the data buffer
    template <typename Function, std::size_t... IndexSequence>
    inline void apply(const Function function, const void* data, std::index_sequence<IndexSequence...>)
    {
        using traits = function_traits<Function>;
        function(task_manager::items::get<IndexSequence, std::tuple_element_t<IndexSequence, typename traits::arguments>...>(data)...);
    }

    // Create task
    template <typename Function, typename... Args>
    task_manager::task* create_task(const Function function, Args... args)
    {
        static const thread_local auto copy = function;
        task* task = task_buffer::allocate();
        task->function = [](auto task, auto data) { apply(copy, data, std::make_index_sequence<sizeof...(Args)>{}); };
        task->parent = nullptr;
        task->remaining.store(1, std::memory_order_relaxed);
        items::copy(task->data, std::forward<Args>(args)...);
        return task;
    }

    // Helper to create batch
    inline task* create_task() 
    { 
        return create_task([](){}); 
    } 

    // Create child task
    template <typename Function, typename... Args>
    task* create_child_task(task* parent, const Function function, Args&&... args)
    {
        auto task = create_task(function, std::forward<Args>(args)...);
        task->parent = parent;
        parent->remaining.fetch_add(1, std::memory_order::memory_order_relaxed);
        return task;
    }

    // Create and run
    template <typename... Args>
    task* create_and_run_task(Args&&... args)
    {
        auto task = create_task(std::forward<Args>(args)...);
        run(task);
        return task;
    }
}
