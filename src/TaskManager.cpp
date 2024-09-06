#include "TaskManager.h"
#include "TaskBuffer.h"

namespace coral::taskmanager
{
static bool executeOnlyPinnedTasksMainThread = false;
static std::vector<std::unique_ptr<WorkerThread>> threads;   

void Start(int threadCount)
{
    Random<Xoshiro256Plus>::Init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    TaskBuffer::Init(threadCount);
    WorkStealingQueues::Init(threadCount);
    PinnedTaskQueues::Init(threadCount);

    for (int i = 1; i < threadCount; i++)
    {
        threads.push_back(std::make_unique<WorkerThread>(i));
    }

    for (auto& thread : threads)
    {
        thread->Run();
    }
}

void Stop()
{
    for (auto& thread : threads)
    {
        thread->Cancel();
    }

    for (auto& thread : threads)
    {
        thread->Join();
    }

    threads.clear();
    WorkStealingQueues::Clear();
    PinnedTaskQueues::Clear();
    TaskBuffer::Clear();
}

void SetExecuteOnlyPinnedTasks(uint8_t threadId, bool executeOnlyPinnedTasks)
{
    if (threadId == 0)
    {
        executeOnlyPinnedTasksMainThread = executeOnlyPinnedTasks;
    }
    else
    {
        threads[threadId - 1]->SetExecuteOnlyPinnedTasks(executeOnlyPinnedTasks);
    }
}

bool IsExecuteOnlyPinnedTasks(uint8_t threadId)
{
    if (threadId == 0)
    {
        return executeOnlyPinnedTasksMainThread;
    }
    else
    {
        return threads[threadId - 1]->IsExecuteOnlyPinnedTasks();
    }
}

void Run(Task* task)
{
    WorkerThread::Enqueue(task);
}

void Run(Task* task, uint8_t threadIndex)
{
    task->threadIndex = threadIndex;
    WorkerThread::Enqueue(task);
}

void RunAndWait(Task* task)
{
    Run(task);
    Wait(task);
}

void RunAndWait(Task* task, uint8_t threadIndex)
{
    Run(task, threadIndex);
    Wait(task);
}

Task* CreateTask(std::function<void(Task*, void*)>&& function)
{
    Task* task = TaskBuffer::Allocate();
    task->function = std::move(function);
    task->parent = nullptr;
    task->remaining.store(1, std::memory_order_relaxed);
    return task;
}

void SetParentTask(Task* parent, Task* child)
{
    child->parent = parent;
    parent->remaining.fetch_add(1, std::memory_order_relaxed);
}

// Create a task as a child of the given parent
Task* CreateChildTask(Task* parent, std::function<void(Task*, void*)>&& function)
{
    Task* task = CreateTask(std::forward<std::function<void(Task*, void*)>>(function));
    SetParentTask(parent, task);
    return task;
}

void AddContinuation(Task* task, Task* continuation)
{
    assert(!IsFinished(task));
    uint8_t index = task->continuationCount.fetch_add(1);
    assert(index < std::numeric_limits<uint8_t>::max());
    task->continuationTasks[index] = task;
}

bool IsFinished(const Task* task)
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

void Wait(std::initializer_list<Task*> tasks)
{
    for (Task* task : tasks)
    {
        internal::Wait(task);
    }
}

template <typename... Tasks>
void Wait(Task* task, Tasks&&... tasks)
{
    internal::Wait(task, std::forward<Tasks>(tasks)...);
}

void Wait(std::function<bool()>&& condition)
{
    while (!condition())
    {
        WorkerThread::TryExecuteOnTask(IsExecuteOnlyPinnedTasks(WorkerThread::GetThreadIndex()));
    }
}

}