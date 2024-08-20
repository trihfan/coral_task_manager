#include "worker_thread.h"
#include <cassert>

#define YIELD std::this_thread::yield();

using namespace coral::task_manager;

worker_thread::worker_thread(int index) : index(index) 
{
}

void worker_thread::run()
{
    thread = std::make_unique<std::thread>([this]()
    {
        random<xoshiro_256_plus>::init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        thread_index = index;
        while (!cancelled.load(std::memory_order_acquire))
        {
            try_execute_one_task(execute_only_pinned_tasks);
        }
    });
}

void worker_thread::try_execute_one_task(bool execute_only_pinned_tasks)
{
    // Execute in priority the pinned tasks
    if (auto task = get_pinned_task_queue()->pop(execute_only_pinned_tasks))
    {
        execute(task);
    }
    else if (!execute_only_pinned_tasks)
    {
        if (auto task = get_or_steal_task())
        {
            execute(task);
        }
        else 
        {
            YIELD
        }
    }
    else 
    {
        YIELD
    }
}

void worker_thread::cancel()
{
    cancelled.store(true, std::memory_order_release);
    pinned_task_queues::get(index)->cancel();
}

void worker_thread::join()
{
    if (thread)
    {
        thread->join();
    }
}

int worker_thread::get_thread_index() 
{ 
    return thread_index; 
}
        
work_stealing_queue* worker_thread::get_work_stealing_queue()
{
    return work_stealing_queues::get(worker_thread::get_thread_index());
}

pinned_task_queue* worker_thread::get_pinned_task_queue()
{
    return pinned_task_queues::get(worker_thread::get_thread_index());
}

void worker_thread::enqueue(task_t task)
{
    if (task->threadIndex == ANY_THREAD_INDEX)
    {
        work_stealing_queue* queue = worker_thread::get_work_stealing_queue();
        queue->push(task);
    }
    else
    {
        pinned_task_queue* queue = pinned_task_queues::get(task->threadIndex);
        queue->push(task);
    }
}

task_t worker_thread::get_or_steal_task()
{
    auto queue = get_work_stealing_queue();
    auto task = queue->pop();
    if (!task)
    {
        // this is not a valid job because our own queue is empty, so try stealing from some other queue
        auto index = random<task_manager::xoshiro_256_plus>::next() % work_stealing_queues::size();
        auto stealQueue = work_stealing_queues::get(index);
        if (stealQueue == queue)
        {
            // don't try to steal from ourselves
            return nullptr;
        }

        auto stolenTask = stealQueue->steal();
        if (!stolenTask)
        {
            // we couldn't steal a job from the other queue either, so we just yield our time slice for now
            return nullptr;
        }

        return stolenTask;
    }

    return task;
}

void worker_thread::finish(task_t task)
{
    const int remaining = task->remaining.fetch_sub(1, std::memory_order_relaxed);
    assert(remaining > 0);
    if (remaining == 1 && task->parent)
    {
        finish(task->parent);
    }
}

void worker_thread::execute(task_t task)
{
    assert(task->threadIndex == ANY_THREAD_INDEX || task->threadIndex == get_thread_index());
    (task->function)(task, task->data);
    finish(task);
}

void worker_thread::set_execute_only_pinned_tasks(bool only_pinned_tasks)
{
    execute_only_pinned_tasks = only_pinned_tasks;
}

bool worker_thread::is_execute_only_pinned_tasks() const
{
    return execute_only_pinned_tasks;
}