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

inline void SetWorkStealingEnabled(uint8_t threadId, bool enableWorkStealing)
{
    if (threadId == 0)
    {
        Manager::workStealingEnabledMainThread = enableWorkStealing;
    }
    else
    {
        Manager::threads[threadId - 1]->SetWorkStealingEnabled(enableWorkStealing);
    }
}

inline bool IsWorkStealingEnabled(uint8_t threadId)
{
    if (threadId == 0)
    {
        return Manager::workStealingEnabledMainThread;
    }
    else
    {
        return Manager::threads[threadId - 1]->IsWorkStealingEnabled();
    }
}

/*** Create tasks ***/ 
template <typename Function, typename... Args>
Task* CreateTask(const Function function, Args... args)
{
    static_assert(sizeof(decltype(function)) == 1, "A task function can't contains captured values");
    static const thread_local auto copy = function;
    Task* task = TaskBuffer::Allocate();
    task->function = [](auto task, auto data) { Apply(copy, data, std::make_index_sequence<sizeof...(Args)>{}); };
    task->parent = nullptr;
    task->remaining.store(1, std::memory_order_relaxed);
    items::copy(task->data, std::forward<Args>(args)...);
    return task;
}

// Helper to create an empty task (for example to create a parent task)
inline Task* CreateTask() { return CreateTask([](){}); } 

//inline Task* CreateTask()

/*
    template <typename... Args> using Function = void (*)(Args...);

    // Function constructor
    Connection(const Signal<Args...>& signal, Function<Args...> function) : signal(signal), type(ConnectionType::function), function(reinterpret_cast<void*>(function)) { }

    // Methods constructor
    template <typename Method, typename Object>
    Connection(const Signal<Args...>& signal, Method method, Object* object) : signal(signal), type(ConnectionType::method), object(reinterpret_cast<void*>(object)), function(*reinterpret_cast<void**>(&method)) { }

    // Lambda constructor
    template <typename Lambda, typename std::enable_if<std::is_convertible<Lambda, std::function<void(Args...)>>::value, bool>::type = true>
    Connection(const Signal<Args...>& signal, Lambda&& lambda) : signal(signal), type(ConnectionType::lambda), function(reinterpret_cast<void*>(new LambdaContainer<Args...>{ lambda })) { }

    // Lambda with no argument constructor for simlpification
    template <typename Lambda, typename std::enable_if<std::is_convertible<Lambda, std::function<void()>>::value && !std::is_same<std::function<void()>, std::function<void(Args...)>>::value, bool>::type = true>
    Connection(const Signal<Args...>& signal, Lambda&& lambda) : signal(signal), type(ConnectionType::lambda), function(reinterpret_cast<void*>(new LambdaContainer<Args...>{[lambda](Args...){ lambda(); }})) { }

*/
inline void SetParentTask(Task* parent, Task* child)
{
    child->parent = parent;
    parent->remaining.fetch_add(1, std::memory_order_relaxed);
    //parent->continuation.
}

// Create a task as a child of the given parent
template <typename Function, typename... Args>
Task* CreateChildTask(Task* parent, const Function function, Args&&... args)
{
    Task* task = CreateTask(function, std::forward<Args>(args)...);
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
            WorkerThread::TryExecuteOnTask(!Manager::IsWorkStealingEnabled(WorkerThread::GetThreadIndex()));
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
        WorkerThread::TryExecuteOnTask(!IsWorkStealingEnabled(WorkerThread::GetThreadIndex()));
    }
}
}