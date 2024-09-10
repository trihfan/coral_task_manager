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
            TryExecuteOnTask(executeOnlyPinnedTasks, executeOnlyPinnedTasks);
        }
    });
}

void WorkerThread::TryExecuteOnTask(bool executeOnlyPinnedTasks, bool useSemaphore)
{
    // Execute in priority the pinned tasks
    if (TaskHandle task = GetPinnedTaskQueue()->Pop(useSemaphore))
    {
        Execute(task);
    }
    // If we can, execute tasks from work stealing queues
    else if (!executeOnlyPinnedTasks)
    {
        if (TaskHandle task = GetOrStealTask())
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

void WorkerThread::Enqueue(TaskHandle task)
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

TaskHandle WorkerThread::GetOrStealTask()
{
    WorkStealingQueue* queue = GetWorkStealingQueue();
    TaskHandle task = queue->Pop();
    if (!task)
    {
        // This is not a valid job because our own queue is empty, so try stealing from some other queue
        size_t index = Random<Xoshiro256Plus>::Next() % WorkStealingQueues::Size();
        WorkStealingQueue* queueToSteal = WorkStealingQueues::Get(index);
        if (queueToSteal == queue)
        {
            // Don't try to steal from ourselves
            return NullTask;
        }

        TaskHandle stolenTask = queueToSteal->Steal();
        if (!stolenTask)
        {
            // We couldn't steal a job from the other queue either, so we just yield our time slice for now
            return NullTask;
        }

        return stolenTask;
    }

    return task;
}

void WorkerThread::Finish(TaskHandle task)
{
    // Finish and signal to parent
    const int remaining = task->remaining.fetch_sub(1, std::memory_order_relaxed);
    assert(remaining > 0);
    if (remaining == 1 && task->parent)
    {
        Finish(task->parent);
    }

    // Handle continuations
    const uint8_t continuationCount = task->continuationCount.load(std::memory_order_acquire);
    const uint16_t continuationInlining = task->continuationInlining.load(std::memory_order_relaxed);

    // Push non inlined continuations to queue
    for (uint8_t i = 0; i < continuationCount; i++)
    {
        if ((continuationInlining & (1 << i)) == 0)
        {
            Enqueue(task->continuationTasks[i]);
        }
    }

    // Execute inlined continuations
    for (uint8_t i = 0; i < continuationCount; i++)
    {
        if (continuationInlining & (1 << i))
        {
            Execute(task->continuationTasks[i]);
        }
    }
}

void WorkerThread::Execute(TaskHandle task)
{
    assert(task->threadIndex == AnyThreadIndex || task->threadIndex == GetThreadIndex());
    if (task->function)
    {
        task->function(task, task->userData);
    }
    Finish(task);
}

void WorkerThread::SetExecuteOnlyPinnedTasks(bool executeOnlyPinnedTasks)
{
    this->executeOnlyPinnedTasks = executeOnlyPinnedTasks;
}

bool WorkerThread::IsExecuteOnlyPinnedTasks() const
{
    return executeOnlyPinnedTasks;
}
