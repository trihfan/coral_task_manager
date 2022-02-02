#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include "work_stealing_queue.h"
#include "random.h"
#include "task.h"
#include "api.h"

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
        api worker_thread(int index);

        // Start the thread
        api void run();

        // Cancel the thread
        // the thread will stop when the current running task has finished
        api void cancel();

        // Wait until the thread is stopped
        api void join();

        //----------------------------------------------------------------
        // Return the current thread index
        api static int get_thread_index();
        
        // Return the work stealing queue of the current thread
        api static work_stealing_queue* get_work_stealing_queue();

        // Return the next task in the queue or null if any
        api static task_t get_task();

        // Execute the given task
        api static void finish(task_t task);
        api static void execute(task_t task);

    private:
        // Thread index
        int index;

        // Flag to cancel the thread
        std::atomic<bool> cancelled { false };

        // The running thread
        std::unique_ptr<std::thread> thread;

        // Thread local value containing the thread index
        inline static thread_local int thread_index = 0;
    };
}
