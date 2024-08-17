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

        // Cancel the thread
        // the thread will stop when the current running task has finished
        void cancel();

        // Wait until the thread is stopped
        void join();

        // Pinned tasks
        void set_execute_only_pinned_tasks(bool only_pinned_tasks = true);

        //----------------------------------------------------------------
        // Return the current thread index
        static int get_thread_index();
        
        // Return the work stealing queue of the current thread
        static work_stealing_queue* get_work_stealing_queue();

        // Return pinned task queue of the current thread
        static pinned_task_queue* get_pinned_task_queue();

        // Return the next task in the queue or null if any
        static task_t get_task();

        // Execute the given task
        static void finish(task_t task);
        static void execute(task_t task);

    private:
        // Thread index
        int index;

        // Flag to cancel the thread
        std::atomic<bool> cancelled { false };

        // The running thread
        std::unique_ptr<std::thread> thread;

        // Thread local value containing the thread index
        inline static thread_local int thread_index = 0;

        // Pinned tasks
        bool execute_only_pinned_tasks = false;
    };
}
