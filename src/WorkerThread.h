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

        // Set flag to allows the thread to steal work from other threads
        // if it is false, thre thread will only execute the pinned task and sleep when there is no task to execute
        void SetWorkStealingEnabled(bool enableWorkStealing);
        bool IsWorkStealingEnabled() const;

        //----------------------------------------------------------------
        // Return the current thread index
        static int GetThreadIndex();
    
        // Enqueue a task to this thread
        static void Enqueue(Task* task);

        // Try to get a task to execute and execute it for the current thread
        static void TryExecuteOnTask(bool workStealingEnabled);

        // Execute the given task
        static void Finish(Task* task);
        static void Execute(Task* task);

    private:
        // Thread index
        int index;

        // If true, the thread can steal work from other thread
        // if it is false, it will execute only the pinned task and sleep when there is no task to execute
        bool workStealingEnabled = true;

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
