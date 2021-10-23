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
        // Push a task to the queue
        void push(Task* task)
        {
            int64_t currentBottom = bottom.load(std::memory_order_relaxed);
            tasks[currentBottom & MASK] = task;
        
            // ensure the job is written before b+1 is published to other threads.
            // on x86/64, a compiler barrier is enough.
            std::atomic_signal_fence(std::memory_order_seq_cst);
        
            bottom.store(currentBottom + 1, std::memory_order_relaxed);
        }

        // Pop a task from the queue
        Task* pop()
        {
            int64_t currentBottom = bottom - 1;
            bottom.store(currentBottom, std::memory_order_seq_cst);

            int64_t currentTop = top.load(std::memory_order_relaxed);
            if (currentTop <= currentBottom)
            {
                // non-empty queue
                Task* task = tasks[currentBottom & MASK];
                if (currentTop != currentBottom)
                {
                    // there's still more than one item left in the queue
                    return task;
                }
        
                // this is the last item in the queue
                if (!top.compare_exchange_strong(currentTop, currentTop + 1, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    // failed race against steal operation
                    task = nullptr;
                }
        
                bottom.store(currentTop + 1, std::memory_order_relaxed);
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
            std::atomic_signal_fence(std::memory_order_seq_cst);

            int64_t currentBottom = bottom.load(std::memory_order_relaxed);

            // Is there somtehing in the queue
            if (currentTop < currentBottom)
            {
                Task* task = tasks[currentTop & MASK];
        
                if (!top.compare_exchange_strong(currentTop, currentTop + 1, std::memory_order_seq_cst, std::memory_order_seq_cst))
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
        static constexpr uint32_t NUMBER_OF_TASKS = 4096u;
        static constexpr uint32_t MASK = NUMBER_OF_TASKS - 1u;
        std::array<Task*, NUMBER_OF_TASKS> tasks;
        std::atomic<int64_t> top { 0 };
        std::atomic<int64_t> bottom { 0 };
    };

    // One WorkStealingQueue per thread
    class WorkStealingQueues
    {
    public:
        static void clear()
        {
            queues.clear();
        }

        static size_t size()
        {
            return queues.size();
        }

        static void resize(size_t size)
        {
            // Fill missing
            for (size_t i = queues.size(); i < size; i++)
            {
                queues.push_back(std::make_unique<WorkStealingQueue>());
            }
            // Limit to size
            queues.resize(size);
        }

        static WorkStealingQueue* get(size_t index)
        {
            return queues[index].get();
        }

    private:
        inline static std::vector<std::unique_ptr<WorkStealingQueue>> queues;
    };
}
