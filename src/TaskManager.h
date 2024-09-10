#pragma once
#ifndef __CORAL_TASK_MANAGER_H__
#define __CORAL_TASK_MANAGER_H__
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include "WorkerThread.h"
#include "Random.h"
#include "Task.h"

namespace coral::taskmanager
{
    /*** Task manager ***/ 
    void Start(int threadCount = std::thread::hardware_concurrency());
    void Stop();

    void SetExecuteOnlyPinnedTasks(uint8_t threadId, bool executeOnlyPinnedTasks);
    bool IsExecuteOnlyPinnedTasks(uint8_t threadId);

    /*** Create tasks ***/ 
    TaskHandle CreateTask(std::function<void(TaskHandle, void*)>&& function = nullptr);
    TaskHandle CreateChildTask(TaskHandle parent, std::function<void(TaskHandle, void*)>&& function);

    // Dependencies
    void SetParentTask(TaskHandle parent, TaskHandle child);
    void AddContinuation(TaskHandle task, TaskHandle continuation, bool inlineTask = false); // An inlined task won't goes to the queue but will be immediately executed by the thread

    /*** Wait for tasks to finished ***/ 
    bool IsFinished(TaskHandle task);

    template <typename... Tasks>
    void Wait(TaskHandle task, Tasks&&... tasks);
    void Wait(std::function<bool()>&& condition);
    void Wait(std::initializer_list<TaskHandle> tasks);

    /*** Run tasks ***/ 
    void Run(TaskHandle task);
    void Run(TaskHandle task, uint8_t threadIndex);

    void RunAndWait(TaskHandle task);
    void RunAndWait(TaskHandle task, uint8_t threadIndex);

    /*** Algorithms ***/ 
    template <typename Type>
    TaskHandle ParallelFor(Type* data, size_t count, std::function<void(TaskHandle, size_t)>&& function, size_t splitSize)
    {
        TaskHandle parentTask = CreateTask();

        for (size_t i = 0; i < count; i += splitSize)
        {
            Run(CreateChildTask(parentTask, [data = (data + i), count = std::min(splitSize, count - i), function]() { function(data, count); }));
        }

        Run(parentTask);
        return parentTask;
    }
}

#endif