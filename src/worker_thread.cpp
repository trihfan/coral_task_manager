#include "worker_thread.h"

#define YIELD std::this_thread::yield();

using namespace coral::task_manager;

WorkerThread::WorkerThread(int index) : index(index) 
{
}

void WorkerThread::run()
{
    thread = std::make_unique<std::thread>([this]()
    {
        Random<Xoshiro256plus>::init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        threadIndex = index;
        while (!cancelled.load(std::memory_order_relaxed))
        {
            auto task = getTask();
            if (task)
            {
                execute(task);
            }
        }
    });
}

void WorkerThread::cancel()
{
    cancelled.store(true, std::memory_order_relaxed);
}

void WorkerThread::join()
{
    if (thread)
    {
        thread->join();
    }
}

int WorkerThread::getThreadIndex() 
{ 
    return threadIndex; 
}
        
WorkStealingQueue* WorkerThread::getWorkStealingQueue()
{
    return WorkStealingQueues::get(WorkerThread::getThreadIndex());
}

task* WorkerThread::getTask()
{
    auto queue = getWorkStealingQueue();
    auto task = queue->pop();
    if (!task)
    {
        // this is not a valid job because our own queue is empty, so try stealing from some other queue
        auto index = Random<task_manager::Xoshiro256plus>::random() % WorkStealingQueues::size();
        WorkStealingQueue* stealQueue = WorkStealingQueues::get(index);
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

void WorkerThread::finish(task* task)
{
    const int remaining = task->remaining.fetch_sub(1, std::memory_order_relaxed);
    assert(remaining > 0);
    if (remaining == 1 && task->parent)
    {
        finish(task->parent);
    }
}

void WorkerThread::execute(task* task)
{
    (task->function)(task, task->data);
    finish(task);
}