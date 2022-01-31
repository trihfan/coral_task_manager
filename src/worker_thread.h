#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include "work_stealing_queue.h"
#include "random.h"
#include "task.h"

namespace coral::task_manager
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
        void run();

        // Cancel the thread
        // the thread will stop when the current running task has finished
        void cancel();

        // Wait until the thread is stopped
        void join();

        //----------------------------------------------------------------
        // Return the current thread index
        static int getThreadIndex();
        
        // Return the work stealing queue of the current thread
        static WorkStealingQueue* getWorkStealingQueue();

        // Return the next task in the queue or null if any
        static task* getTask();

        // Execute the given task
        static void finish(task* task);
        static void execute(task* task);

    private:
        // Thread index
        int index;

        // Flag to cancel the thread
        std::atomic<bool> cancelled { false };

        // The running thread
        std::unique_ptr<std::thread> thread;

        // Thread local value containing the thread index
        inline static thread_local int threadIndex = 0;
    };
}
