#pragma once
#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include "config.h"

namespace coral::task_manager
{
    struct task;
    
    /**
     * WorkStealingQueue is a queue of task allowing work stealing
     * Only the thread owning the queue can push or pop, 
     * any other thread can steal from it
     */
    class WorkStealingQueue
    {
    public:
        // Push a task to the queue
        void push(task* task);

        // Pop a task from the queue
        task* pop();

        // Steal a task from the queue
        task* steal();

    private:
        std::array<task*, config::maxTaskCount> tasks;
        std::atomic<int64_t> top { 0 };
        std::atomic<int64_t> bottom { 0 };
    };

    // One WorkStealingQueue per thread
    class WorkStealingQueues
    {
    public:
        static void clear();
        static size_t size();
        static void init(size_t size);
        static WorkStealingQueue* get(size_t index);

    private:
        inline static size_t count = 0;
        inline static WorkStealingQueue* queues = nullptr;
    };
}
