#include "work_stealing_queue.h"

using namespace coral::task_manager;

void WorkStealingQueue::push(task* task)
{
    int64_t currentBottom = bottom.load(std::memory_order_relaxed);
    tasks[currentBottom & config::maxTaskCountMask] = task;

    // ensure the job is written before b+1 is published to other threads.
    // on x86/64, a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_release);

    bottom.store(currentBottom + 1, std::memory_order_relaxed);
}

task* WorkStealingQueue::pop()
{
    int64_t currentBottom = bottom.load(std::memory_order_relaxed) - 1;
    bottom.store(currentBottom, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    int64_t currentTop = top.load(std::memory_order_relaxed);

    if (currentTop < currentBottom)
    {
        // non-empty queue
        return tasks[currentBottom & config::maxTaskCountMask];
    }
    else if(currentTop == currentBottom)
    {
        // this is the last item in the queue
        auto task = tasks[currentBottom & config::maxTaskCountMask];
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

task* WorkStealingQueue::steal()
{
    int64_t currentTop = top.load(std::memory_order_relaxed);

    // ensure that top is always read before bottom.
    // loads will not be reordered with other loads on x86, so a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_seq_cst);

    int64_t currentBottom = bottom.load(std::memory_order_relaxed);
    if (currentTop < currentBottom)
    {
        // non-empty queue
        auto task = tasks[currentTop & config::maxTaskCountMask];

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

void WorkStealingQueues::clear()
{
    delete[] queues;
    queues = nullptr;
}

size_t WorkStealingQueues::size()
{
    return count;
}

void WorkStealingQueues::init(size_t size)
{
    count = size;
    queues = new WorkStealingQueue[size];
}

WorkStealingQueue* WorkStealingQueues::get(size_t index)
{
    return &queues[index];
}
