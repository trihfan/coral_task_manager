#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <semaphore>
#include "work_stealing_queue.h"
#include "pinned_task_queue.h"
#include "random.h"
#include "task.h"

namespace coral::task_manager
{
    struct Task;

    /**
     * The worker_thread class encapsulate a std::thread used to run the tasks
     */
    class worker_thread
    {
    public:
        // Construction
        worker_thread(int index);

        // Start the thread
        void run();

        // Cancel the thread, it will stop when the current running task has finished
        void cancel();

        // Wait until the thread is stopped
        void join();

        // Set flag to only execute the tasks pinned for this thread (it will not try to steal task from other threads)
        // The thread will be in a sleep state while there is no tasks
        void set_execute_only_pinned_tasks(bool only_pinned_tasks = true);
        bool is_execute_only_pinned_tasks() const;

        //----------------------------------------------------------------
        // Return the current thread index
        static int get_thread_index();
    
        // Enqueue a task to this thread
        static void enqueue(task_t task);

        // Try to get a task to execute and execute it for the current thread
        static void try_execute_one_task(bool execute_only_pinned_tasks);

        // Execute the given task
        static void finish(task_t task);
        static void execute(task_t task);

    private:
        // Thread index
        int index;

        // If true, the thread will only execute the tasks pinned on this thread
        bool execute_only_pinned_tasks = false;

        // Flag to cancel the thread
        std::atomic<bool> cancelled { false };

        // The running thread
        std::unique_ptr<std::thread> thread;

        // Thread local value containing the thread index
        inline static thread_local int thread_index = 0;

        // Return the next task in the queue or null if any
        static task_t get_or_steal_task();

        // Return the work stealing queue of the current thread
        static work_stealing_queue* get_work_stealing_queue();

        // Return pinned task queue of the current thread
        static pinned_task_queue* get_pinned_task_queue();
    };
}
