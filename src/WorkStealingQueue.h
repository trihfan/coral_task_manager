#pragma once
#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace coral::task_manager
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

        /*Task* allocate()
        {
            int64_t currentBottom = bottom.load(std::memory_order_relaxed);
	        return tasks[currentBottom & MASK];
        }

        void commit()
        {
            int64_t currentBottom = bottom.load(std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_release);
            bottom.store(currentBottom + 1, std::memory_order_relaxed);
        }*/

        // Push a task to the queue
        void push(Task* task)
        {
            int64_t currentBottom = bottom.load(std::memory_order_relaxed);
            tasks[currentBottom & maxTaskCountMask] = task;
        
            // ensure the job is written before b+1 is published to other threads.
            // on x86/64, a compiler barrier is enough.
            std::atomic_thread_fence(std::memory_order_release);
        
            bottom.store(currentBottom + 1, std::memory_order_relaxed);
        }

        // Pop a task from the queue
        Task* pop()
        {
            int64_t currentBottom = bottom.load(std::memory_order_relaxed) - 1;
            bottom.store(currentBottom, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_seq_cst);
            int64_t currentTop = top.load(std::memory_order_relaxed);

            if (currentTop < currentBottom)
            {
                // non-empty queue
                return tasks[currentBottom & maxTaskCountMask];
            }
            else if(currentTop == currentBottom)
            {
                // this is the last item in the queue
                Task* task = tasks[currentBottom & maxTaskCountMask];
                const int64_t desired = currentTop + 1;
                if (!top.compare_exchange_strong(currentTop, desired, std::memory_order_seq_cst, std::memory_order_relaxed))
                {
                    // failed race against steal operation
                    task = nullptr;
                }

                bottom.store(desired, std::memory_order_relaxed);
                return task;
            }
            else
            {
                // deque was already empty
                bottom.store(currentTop, std::memory_order_relaxed);
                return nullptr;
            }
        }

        // Steal a task from the queue
        Task* steal()
        {
            int64_t currentTop = top.load(std::memory_order_relaxed);
 
            // ensure that top is always read before bottom.
            // loads will not be reordered with other loads on x86, so a compiler barrier is enough.
            std::atomic_thread_fence(std::memory_order_seq_cst);
        
            int64_t currentBottom = bottom.load(std::memory_order_relaxed);
            if (currentTop < currentBottom)
            {
                // non-empty queue
                Task* task = tasks[currentTop & maxTaskCountMask];
        
                // the interlocked function serves as a compiler barrier, and guarantees that the read happens before the CAS.
                if (!top.compare_exchange_strong(currentTop, currentTop + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
                {
                    // a concurrent steal or pop operation removed an element from the deque in the meantime.
                    return nullptr;
                }
        
                return task;
            }
            else
            {
                // empty queue
                return nullptr;
            }
        }

    private:
        std::array<Task*, maxTaskCount> tasks;
        std::atomic<int64_t> top { 0 };
        std::atomic<int64_t> bottom { 0 };
    };

    // One WorkStealingQueue per thread
    class WorkStealingQueues
    {
    public:
        static void clear()
        {
            delete[] queues;
            queues = nullptr;
        }

        static size_t size()
        {
            return count;
        }

        static void init(size_t size)
        {
            count = size;
            queues = new WorkStealingQueue[size];
        }

        static WorkStealingQueue* get(size_t index)
        {
            return &queues[index];
        }

    private:
        inline static size_t count = 0;
        inline static WorkStealingQueue* queues = nullptr;
    };
}
