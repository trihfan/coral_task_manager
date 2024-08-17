#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include <semaphore>
#include "config.h"

namespace coral::task_manager
{
    struct task;
    using task_t = task*;

    class pinned_task_queue
    {
    public:
        pinned_task_queue(size_t size);

        void push(task_t task);
        task_t pop(bool wait_for_semaphore);

        void clear();
        void cancel() { semaphore.release(); }

    private:
        struct pinned_task_queue_leaf;
        using pinned_task_queue_leaf_t = pinned_task_queue_leaf*;

        struct pinned_task_queue_leaf
        {
            task_t task;
            std::atomic<pinned_task_queue_leaf_t> next;
        };

        std::binary_semaphore semaphore { 0 };
        
        // Free list
        std::vector<pinned_task_queue_leaf> buffer;
        std::atomic<int64_t> index { 0 };

        // Internal lock free list
        std::atomic<pinned_task_queue_leaf_t> head;
        pinned_task_queue_leaf tail;
    };

    // One pinned_task_queue per thread
    class pinned_task_queues
    {
    public:
        static void init(size_t size);
        static void clear();
        
        static size_t size();
        static pinned_task_queue* get(size_t index);

    private:
        inline static size_t count = 0;
        inline static std::vector<pinned_task_queue*> queues;
    };
}
