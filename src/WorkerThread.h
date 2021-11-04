#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include "WorkStealingQueue.h"
#include "Random.h"
#include "Task.h"

#define YIELD std::this_thread::yield();

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
        WorkerThread(int index) : index(index) {}

        // Start the thread
        void run()
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

        // Cancel the thread
        // the thread will stop when the current running task has finished
        void cancel()
        {
            cancelled.store(true, std::memory_order_relaxed);
        }

        // Wait until the thread is stopped
        void join()
        {
            if (thread)
            {
                thread->join();
            }
        }

        //----------------------------------------------------------------
        // Return the current thread index
        static int getThreadIndex() { return threadIndex; }
        
        // Return the work stealing queue of the current thread
        static WorkStealingQueue* getWorkStealingQueue()
        {
            return WorkStealingQueues::get(WorkerThread::getThreadIndex());
        }

        // Return the next task in the queue or null if any
        static Task* getTask()
        {
            auto queue = getWorkStealingQueue();
            Task* task = queue->pop();
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

                Task* stolenTask = stealQueue->steal();
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

        // Execute the given task
        static void finish(Task* task)
        {
            const int remaining = task->remaining.fetch_sub(1, std::memory_order_relaxed);
            assert(remaining > 0);
            if (remaining == 1 && task->parent)
            {
                finish(task->parent);
            }
        }

        static void execute(Task* task)
        {
            (task->function)(task, task->data);
            finish(task);
        }

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
