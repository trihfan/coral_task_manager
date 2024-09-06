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
    Task* CreateTask(std::function<void(Task*, void*)>&& function = nullptr);
    Task* CreateChildTask(Task* parent, std::function<void(Task*, void*)>&& function);

    // Dependencies
    void SetParentTask(Task* parent, Task* child);
    void AddContinuation(Task* task, Task* continuation);

    /*** Wait for tasks to finished ***/ 
    bool IsFinished(const Task* task);

    template <typename... Tasks>
    void Wait(Task* task, Tasks&&... tasks);
    void Wait(std::function<bool()>&& condition);
    void Wait(std::initializer_list<Task*> tasks);

    /*** Run tasks ***/ 
    void Run(Task* task);
    void Run(Task* task, uint8_t threadIndex);

    void RunAndWait(Task* task);
    void RunAndWait(Task* task, uint8_t threadIndex);

    /*template <typename Type, typename Splitter>
    Task* parallel_for(Type* data, size_t count, void (*function)(T*, size_t), const Splitter& splitter)
    {
        typedef parallel_for_job_data<Type, Splitter> TaskData;
        const TaskData taskData(data, count, function, splitter);
        return createTask(&parallel_for_task<TaskData>, taskData);
    }

    template <typename Type, typename Splitter>
    struct parallel_for_task_data
    {
        typedef Type DataType;
        typedef Splitter SplitterType;

        parallel_for_task_data(DataType* data, size_t count, void (*function)(DataType*, size_t), const SplitterType& splitter)
        : data(data)  count(count) , function(function) , splitter(splitter) { }

        DataType* data;
        size_t count;
        void (*function)(DataType*, size_t);
        SplitterType splitter;
    };

    template <typename TaskData>
    void parallel_for_task(Task* task, const void* taskData)
    {
        const TaskData* data = static_cast<const TaskData*>(jobData);
        const TaskData::SplitterType& splitter = data->splitter;

        if (splitter.Split<TaskData::DataType>(data->count))
        {
            // split in two
            const unsigned int leftCount = data->count / 2u;
            const TaskData leftData(data->data, leftCount, data->function, splitter);
            Job* left = jobSystem::CreateJobAsChild(job, &jobs::parallel_for_job<TaskData>, leftData);
            jobSystem::Run(left);

            const unsigned int rightCount = data->count - leftCount;
            const TaskData rightData(data->data + leftCount, rightCount, data->function, splitter);
            Job* right = jobSystem::CreateJobAsChild(job, &jobs::parallel_for_job<TaskData>, rightData);
            jobSystem::Run(right);
        }
        else
        {
            // execute the function on the range of data
            (data->function)(data->data, data->count);
        }
    }*/
}

#endif