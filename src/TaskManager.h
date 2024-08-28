#pragma once
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include "WorkerThread.h"
#include "Random.h"
#include "Task.h"

namespace coral::taskmanager
{
    // The actual task manager
    class Manager
    {
    public:
        // Start the manager, this will create (threadCount - 1) threads (the first one is the main thread)
        static void Start(int threadCount = std::thread::hardware_concurrency());
        static void Stop();

        inline static bool workStealingEnabledMainThread = true;
        inline static std::vector<std::unique_ptr<WorkerThread>> threads;   
    };

    /************************************************************/
    
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
