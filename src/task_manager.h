#pragma once
#include <thread>
#include <vector>
#include <memory>
#include "worker_thread.h"
#include "random.h"
#include "task.h"

namespace coral::task_manager
{
    // The actual task manager
    class manager
    {
    public:
        static void start(int threadCount = std::thread::hardware_concurrency() - 1);
        static void stop();

    private:
        inline static std::vector<std::unique_ptr<worker_thread>> threads;
    };

    /************************************************************/
    inline bool is_finished(const task_t task)
    {
        return task->remaining.load(std::memory_order_relaxed) <= 0;
    }

    inline void start(int threadCount = std::thread::hardware_concurrency() - 1)
    {
        manager::start(threadCount);        
    }

    inline void stop()
    {
        manager::stop();        
    }

    inline void run(task_t task)
    {
        work_stealing_queue* queue = worker_thread::get_work_stealing_queue();
        queue->push(task);
    }

    namespace internal
    {
        inline void wait() {}

        template <typename... Tasks>
        inline void wait(task_t task, Tasks&&... tasks)
        {
            while (!is_finished(task))
            {
                auto nextTask = worker_thread::get_task();
                if (nextTask)
                {
                    worker_thread::execute(nextTask);
                }
            }
            internal::wait(std::forward<Tasks>(tasks)...);
        }
    }

    inline void wait(std::initializer_list<task_t> tasks)
    {
        for (auto task : tasks)
        {
            internal::wait(task);
        }
    }

    template <typename... Tasks>
    inline void wait(task_t task, Tasks&&... tasks)
    {
        internal::wait(task, std::forward<Tasks>(tasks)...);
    }

    inline void wait(std::function<bool()>&& condition)
    {
        while (!condition())
        {
            auto nextTask = worker_thread::get_task();
            if (nextTask)
            {
                worker_thread::execute(nextTask);
            }
        }
    }

    /*template <typename Type, typename Splitter>
    task_t parallel_for(Type* data, size_t count, void (*function)(T*, size_t), const Splitter& splitter)
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
    void parallel_for_task(task_t task, const void* taskData)
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
