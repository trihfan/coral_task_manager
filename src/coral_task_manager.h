#pragma once

// config
#include "config.h"

// includes
#include "task.h"
#include "task_manager.h"

// Usage
namespace coral::task_manager
{
    template <typename... Args> task* create_task(task_function function, Args&&... args);
    task* create_task();
    task* create_child_task(Task* parent, task_function function);
    template <typename... Args> task* create_child_task(task* parent, task_function function, Args&&... args);
    template <typename... Args> task* create_and_run_ask(Args&&... args);
}

namespace coral::task_manager
{
    void start(int threadCount);
    void stop();


    bool isFinished(const task* task);

    void run(task* task);
    void wait(std::initializer_list<task*> tasks);
    template <typename... Tasks> void wait(task* task, Tasks&&... tasks);
}
