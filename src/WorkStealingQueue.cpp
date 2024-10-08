#include "WorkStealingQueue.h"
#include "Task.h"

using namespace coral::taskmanager;

WorkStealingQueue::WorkStealingQueue(size_t size)
{
    tasks.resize(size);
}

void WorkStealingQueue::Push(TaskHandle task)
{
    int64_t currentBottom = bottom.load(std::memory_order_relaxed);
    tasks[currentBottom & Config::GetMaxTaskCountMask()] = task;

    // ensure the job is written before b+1 is published to other threads.
    // on x86/64, a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_release);

    bottom.store(currentBottom + 1, std::memory_order_relaxed);
}

TaskHandle WorkStealingQueue::Pop()
{
    int64_t currentBottom = bottom.load(std::memory_order_relaxed) - 1;
    bottom.store(currentBottom, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    int64_t currentTop = top.load(std::memory_order_relaxed);

    if (currentTop < currentBottom)
    {
        // non-empty queue
        return tasks[currentBottom & Config::GetMaxTaskCountMask()];
    }
    else if (currentTop == currentBottom)
    {
        // this is the last item in the queue
        TaskHandle task = tasks[currentBottom & Config::GetMaxTaskCountMask()];
        const int64_t desired = currentTop + 1;
        if (!top.compare_exchange_strong(currentTop, desired, std::memory_order_seq_cst, std::memory_order_relaxed))
        {
            // failed race against steal operation
            task = NullTask;
        }

        bottom.store(desired, std::memory_order_relaxed);
        return task;
    }
    else
    {
        // deque was already empty
        bottom.store(currentTop, std::memory_order_relaxed);
        return NullTask;
    }
}

TaskHandle WorkStealingQueue::Steal()
{
    int64_t currentTop = top.load(std::memory_order_relaxed);

    // ensure that top is always read before bottom.
    // loads will not be reordered with other loads on x86, so a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_seq_cst);

    int64_t currentBottom = bottom.load(std::memory_order_relaxed);
    if (currentTop < currentBottom)
    {
        // non-empty queue
        TaskHandle task = tasks[currentTop & Config::GetMaxTaskCountMask()];

        // the interlocked function serves as a compiler barrier, and guarantees that the read happens before the CAS.
        if (!top.compare_exchange_strong(currentTop, currentTop + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
        {
            // a concurrent steal or pop operation removed an element from the deque in the meantime.
            return NullTask;
        }

        return task;
    }
    else
    {
        // empty queue
        return NullTask;
    }
}

void WorkStealingQueues::Clear()
{
    for (size_t i = 0; i < queues.size(); i++)
    {
        delete queues[i];
    }
    queues.clear();
}

size_t WorkStealingQueues::Size()
{
    return count;
}

void WorkStealingQueues::Init(size_t size)
{
    count = size;
    queues.resize(size);
    for (size_t i = 0; i < size; i++)
    {
        queues[i] = new WorkStealingQueue(Config::GetMaxTaskCount());
    }
}

WorkStealingQueue* WorkStealingQueues::Get(size_t index)
{
    return queues[index];
}
