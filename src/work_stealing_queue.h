#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include "config.h"
#include "api.h"

namespace coral::task_manager
{
    struct task;
    
    /**
     * work_stealing_queue is a queue of task allowing work stealing
     * Only the thread owning the queue can push or pop, 
     * any other thread can steal from it
     */
    class work_stealing_queue
    {
    public:
        api work_stealing_queue(size_t size);

        // Push a task to the queue
        api void push(task_t task);

        // Pop a task from the queue
        api task_t pop();

        // Steal a task from the queue
        api task_t steal();

    private:
        std::vector<task_t> tasks;
        std::atomic<int64_t> top { 0 };
        std::atomic<int64_t> bottom { 0 };
    };

    // One work_stealing_queue per thread
    class work_stealing_queues
    {
    public:
        api static void init(size_t size);
        api static void clear();
        
        api static size_t size();
        api static work_stealing_queue* get(size_t index);

    private:
        inline static size_t count = 0;
        inline static std::vector<work_stealing_queue*> queues;
    };
}
