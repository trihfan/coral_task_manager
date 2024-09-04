#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <semaphore>
#include "WorkStealingQueue.h"
#include "PinnedTaskQueue.h"
#include "Random.h"
#include "Task.h"

namespace coral::taskmanager
{
    struct Task;

    /**
     * The WorkerThread class encapsulate a std::thread used to run the tasks
     */
    class WorkerThread
    {
    public:
        // Construction
        WorkerThread(int index);

        // Start the thread
        void Run();

        // Cancel the thread, it will stop when the current running task has finished
        void Cancel();

        // Wait until the thread is stopped
        void Join();

        // Force the thread to execute only the pinned task queue
        void SetExecuteOnlyPinnedTasks(bool executeOnlyPinnedTasks);
        bool IsExecuteOnlyPinnedTasks() const;

        //----------------------------------------------------------------
        // Return the current thread index
        static int GetThreadIndex();
    
        // Enqueue a task to this thread
        static void Enqueue(Task* task);

        // Execute one task if we can find one
        // executeOnlyPinnedTasks: only get task from pinned queue
        // useSemaphore: if pinned queue is empty, wait for the semaphore to be signaled so the thread will sleep
        static void TryExecuteOnTask(bool executeOnlyPinnedTasks, bool useSemaphore = false);

        // Execute the given task
        static void Finish(Task* task);
        static void Execute(Task* task);

    private:
        // Thread index
        int index;

        // Force the thread to execute only the pinned task queue
        bool executeOnlyPinnedTasks = false;

        // Flag to cancel the thread
        std::atomic<bool> cancelled { false };

        // The running thread
        std::unique_ptr<std::thread> thread;

        // Thread local value containing the thread index
        inline static thread_local int ThreadIndex = 0;

        // Return the next task in the queue or null if any
        static Task* GetOrStealTask();

        // Return the work stealing queue of the current thread
        static WorkStealingQueue* GetWorkStealingQueue();

        // Return pinned task queue of the current thread
        static PinnedTaskQueue* GetPinnedTaskQueue();
    };
}
