#pragma once

// config
#include "Config.h"

// includes
#include "Task.h"
#include "TaskManager.h"

// Usage
/*
 *  coral::taskmanager::Start();
 *
 *  Task* task = coral::taskmanager::CreateTask([](){ ... });
 *  coral::taskmanager::Run(task);
 *  coral::taskmanager::Wait(task);
 * 
 *  coral::taskmanager::Stop();
 */
namespace coral::taskmanager
{
/*** Task manager ***/ 
inline void Start(int threadCount = std::thread::hardware_concurrency())
{
    Manager::Start(threadCount);        
}

inline void Stop()
{
    Manager::Stop();        
}

inline void SetExecuteOnlyPinnedTasks(uint8_t threadId, bool executeOnlyPinnedTasks)
{
    if (threadId == 0)
    {
        Manager::executeOnlyPinnedTasksMainThread = executeOnlyPinnedTasks;
    }
    else
    {
        Manager::threads[threadId - 1]->SetExecuteOnlyPinnedTasks(executeOnlyPinnedTasks);
    }
}

inline bool IsExecuteOnlyPinnedTasks(uint8_t threadId)
{
    if (threadId == 0)
    {
        return Manager::executeOnlyPinnedTasksMainThread;
    }
    else
    {
        return Manager::threads[threadId - 1]->IsExecuteOnlyPinnedTasks();
    }
}

/*** Create tasks ***/ 
template <typename... Args>
inline Task* CreateTask(std::function<void(Task*, void*)>&& function, Args... args)
{
    Task* task = TaskBuffer::Allocate();
    task->function = std::move(function);
    task->parent = nullptr;
    task->remaining.store(1, std::memory_order_relaxed);
    items::copy(task->data, std::forward<Args>(args)...);
    return task;
}

// Create an empty task
inline Task* CreateTask() { return CreateTask(nullptr); } 

inline void SetParentTask(Task* parent, Task* child)
{
    child->parent = parent;
    parent->remaining.fetch_add(1, std::memory_order_relaxed);
    //parent->continuation.
}

// Create a task as a child of the given parent
template <typename... Args>
Task* CreateChildTask(Task* parent, std::function<void(Task*, void*)>&& function, Args&&... args)
{
    Task* task = CreateTask(std::forward<std::function<void(Task*, void*)>>(function), std::forward<Args>(args)...);
    SetParentTask(parent, task);
    return task;
}

/*** Run tasks ***/ 
inline void Run(Task* task)
{
    WorkerThread::Enqueue(task);
}

inline void Run(Task* task, uint8_t threadIndex)
{
    task->threadIndex = threadIndex;
    WorkerThread::Enqueue(task);
}

/*** Wait for tasks to finished ***/ 
inline bool IsFinished(const Task* task)
{
    return task->remaining.load(std::memory_order_relaxed) <= 0;
}

namespace internal
{      
    inline void Wait() {}

    template <typename... Tasks>
    inline void Wait(Task* task, Tasks&&... tasks)
    {
        while (!IsFinished(task))
        {
            WorkerThread::TryExecuteOnTask(IsExecuteOnlyPinnedTasks(WorkerThread::GetThreadIndex()));
        }
        internal::Wait(std::forward<Tasks>(tasks)...);
    }
}

inline void Wait(std::initializer_list<Task*> tasks)
{
    for (Task* task : tasks)
    {
        internal::Wait(task);
    }
}

template <typename... Tasks>
inline void Wait(Task* task, Tasks&&... tasks)
{
    internal::Wait(task, std::forward<Tasks>(tasks)...);
}

inline void Wait(std::function<bool()>&& condition)
{
    while (!condition())
    {
        WorkerThread::TryExecuteOnTask(IsExecuteOnlyPinnedTasks(WorkerThread::GetThreadIndex()));
    }
}
}