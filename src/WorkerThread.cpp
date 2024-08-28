#include "WorkerThread.h"
#include <cassert>

#define YIELD std::this_thread::yield();

using namespace coral::taskmanager;

WorkerThread::WorkerThread(int index) : index(index) 
{
}

void WorkerThread::Run()
{
    thread = std::make_unique<std::thread>([this]()
    {
        ThreadIndex = index;
        Random<Xoshiro256Plus>::Init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        while (!cancelled.load(std::memory_order_acquire))
        {
            TryExecuteOnTask(workStealingEnabled);
        }
    });
}

void WorkerThread::TryExecuteOnTask(bool workStealingEnabled)
{
    // Execute in priority the pinned tasks
    if (Task* task = GetPinnedTaskQueue()->Pop(!workStealingEnabled))
    {
        Execute(task);
    }
    else if (workStealingEnabled)
    {
        if (Task* task = GetOrStealTask())
        {
            Execute(task);
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

void WorkerThread::Cancel()
{
    cancelled.store(true, std::memory_order_release);
    PinnedTaskQueues::Get(index)->Cancel();
}

void WorkerThread::Join()
{
    if (thread)
    {
        thread->join();
    }
}

int WorkerThread::GetThreadIndex() 
{ 
    return ThreadIndex; 
}
        
WorkStealingQueue* WorkerThread::GetWorkStealingQueue()
{
    return WorkStealingQueues::Get(WorkerThread::GetThreadIndex());
}

PinnedTaskQueue* WorkerThread::GetPinnedTaskQueue()
{
    return PinnedTaskQueues::Get(WorkerThread::GetThreadIndex());
}

void WorkerThread::Enqueue(Task* task)
{
    // If this task can be executed by any thread, push it in the curren thread
    if (task->threadIndex == AnyThreadIndex)
    {
        WorkerThread::GetWorkStealingQueue()->Push(task);
    }
    // If it is a pinned task, we need to push it to the correct thread queue
    else
    {
        PinnedTaskQueues::Get(task->threadIndex)->Push(task);
    }
}

Task* WorkerThread::GetOrStealTask()
{
    WorkStealingQueue* queue = GetWorkStealingQueue();
    Task* task = queue->Pop();
    if (!task)
    {
        // This is not a valid job because our own queue is empty, so try stealing from some other queue
        size_t index = Random<Xoshiro256Plus>::Next() % WorkStealingQueues::Size();
        WorkStealingQueue* queueToSteal = WorkStealingQueues::Get(index);
        if (queueToSteal == queue)
        {
            // Don't try to steal from ourselves
            return nullptr;
        }

        Task* stolenTask = queueToSteal->Steal();
        if (!stolenTask)
        {
            // We couldn't steal a job from the other queue either, so we just yield our time slice for now
            return nullptr;
        }

        return stolenTask;
    }

    return task;
}

void WorkerThread::Finish(Task* task)
{
    const int remaining = task->remaining.fetch_sub(1, std::memory_order_relaxed);
    assert(remaining > 0);
    if (remaining == 1 && task->parent)
    {
        Finish(task->parent);
    }
}

void WorkerThread::Execute(Task* task)
{
    assert(task->threadIndex == AnyThreadIndex || task->threadIndex == GetThreadIndex());
    (task->function)(task, task->data);
    Finish(task);
}

void WorkerThread::SetWorkStealingEnabled(bool enableWorkStealing)
{
    workStealingEnabled = enableWorkStealing;
}

bool WorkerThread::IsWorkStealingEnabled() const
{
    return workStealingEnabled;
}