#include "work_stealing_queue.h"
#include "task.h"

using namespace coral::task_manager;

work_stealing_queue::work_stealing_queue(size_t size)
{
    tasks.resize(size);
}

void work_stealing_queue::push(task_t task)
{
    int64_t currentBottom = bottom.load(std::memory_order_relaxed);
    tasks[currentBottom & config::get_max_task_count_mask()] = task;

    // ensure the job is written before b+1 is published to other threads.
    // on x86/64, a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_release);

    bottom.store(currentBottom + 1, std::memory_order_relaxed);
}

task_t work_stealing_queue::pop()
{
    int64_t currentBottom = bottom.load(std::memory_order_relaxed) - 1;
    bottom.store(currentBottom, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    int64_t currentTop = top.load(std::memory_order_relaxed);

    if (currentTop < currentBottom)
    {
        // non-empty queue
        return tasks[currentBottom & config::get_max_task_count_mask()];
    }
    else if(currentTop == currentBottom)
    {
        // this is the last item in the queue
        auto task = tasks[currentBottom & config::get_max_task_count_mask()];
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

task_t work_stealing_queue::steal()
{
    int64_t currentTop = top.load(std::memory_order_relaxed);

    // ensure that top is always read before bottom.
    // loads will not be reordered with other loads on x86, so a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_seq_cst);

    int64_t currentBottom = bottom.load(std::memory_order_relaxed);
    if (currentTop < currentBottom)
    {
        // non-empty queue
        auto task = tasks[currentTop & config::get_max_task_count_mask()];

        // tmp easy way to prevent pinned task to leave this queue
        if (task->isPinned == 1)
        {
            return nullptr;
        }

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

void work_stealing_queues::clear()
{
    for (size_t i = 0; i < queues.size(); i++)
    {
        delete queues[i];
    }
    queues.clear();
}

size_t work_stealing_queues::size()
{
    return count;
}

void work_stealing_queues::init(size_t size)
{
    count = size;
    queues.resize(size);
    for (size_t i = 0; i < size; i++)
    {
        queues[i] = new work_stealing_queue(config::get_max_task_count());
    }
}

work_stealing_queue* work_stealing_queues::get(size_t index)
{
    return queues[index];
}
