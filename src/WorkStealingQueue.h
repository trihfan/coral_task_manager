#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include "Config.h"

namespace coral::taskmanager
{
    struct Task;

    /**
     * WorkStealingQueue is a queue of task allowing work stealing
     * Only the thread owning the queue can push or pop, 
     * any other thread can steal from it
     */
    class WorkStealingQueue
    {
    public:
        WorkStealingQueue(size_t bufferSize);

        // Push a task to the queue
        void Push(TaskHandle task);

        // Pop a task from the queue
        TaskHandle Pop();

        // Steal a task from the queue
        TaskHandle Steal();

    private:
        std::vector<TaskHandle> tasks;
        std::atomic<int64_t> top { 0 };
        std::atomic<int64_t> bottom { 0 };
    };

    // Container to encapsulate a WorkStealingQueue per thread
    class WorkStealingQueues
    {
    public:
        // Initialization
        static void Init(size_t size);
        static void Clear();
        
        static size_t Size();
        static WorkStealingQueue* Get(size_t index);

    private:
        inline static size_t count = 0;
        inline static std::vector<WorkStealingQueue*> queues;
    };
}
