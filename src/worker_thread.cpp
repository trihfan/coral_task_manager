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
        while (!cancelled.load(std::memory_order_relaxed))
        {
            auto task = get_task();
            if (task)
            {
                execute(task);
            }
        }
    });
}

void worker_thread::cancel()
{
    cancelled.store(true, std::memory_order_relaxed);
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

task_t worker_thread::get_task()
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
            YIELD
            return nullptr;
        }

        auto stolenTask = stealQueue->steal();
        if (!stolenTask)
        {
            // we couldn't steal a job from the other queue either, so we just yield our time slice for now
            YIELD
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
    (task->function)(task, task->data);
    finish(task);
}