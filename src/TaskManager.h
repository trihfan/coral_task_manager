#pragma once
#include <thread>
#include <vector>
#include <memory>
#include "WorkerThread.h"
#include "Random.h"

namespace coral::task_manager
{
    struct Task;
    class WorkerThread;

    class TaskManager
    {
    public:
        static void start(int threadCount = std::thread::hardware_concurrency() - 1)
        {
            Random<Xoshiro256plus>::init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            WorkStealingQueues::init(threadCount + 1);
            
            for (int i = 0; i < threadCount; i++)
            {
                threads.push_back(std::make_unique<WorkerThread>(i + 1));
            }

            for (auto& thread : threads)
            {
                thread->run();
            }
        }

        static void stop()
        {
            for (auto& thread : threads)
            {
                thread->cancel();
            }

            for (auto& thread : threads)
            {
                thread->join();
            }

            threads.clear();
            WorkStealingQueues::clear();
        }

    private:
        inline static std::vector<std::unique_ptr<WorkerThread>> threads;
    };

    inline void start(int threadCount = std::thread::hardware_concurrency() - 1)
    {
        TaskManager::start(threadCount);        
    }

    inline void stop()
    {
        TaskManager::stop();        
    }

    inline void run(Task* task)
    {
        WorkStealingQueue* queue = WorkerThread::getWorkStealingQueue();
        queue->push(task);
    }

    namespace internal
    {
        inline void wait() 
        {
        }

        template <typename... Tasks>
        inline void wait(Task* task, Tasks&&... tasks)
        {
            while (!isFinished(task))
            {
                Task* nextTask = WorkerThread::getTask();
                if (nextTask)
                {
                    WorkerThread::execute(nextTask);
                }
            }
            internal::wait(std::forward<Tasks>(tasks)...);
        }
    }

    inline void wait(std::initializer_list<Task*> tasks)
    {
        for (auto task : tasks)
        {
            internal::wait(task);
        }
    }

    template <typename... Tasks>
    inline void wait(Task* task, Tasks&&... tasks)
    {
        internal::wait(task, std::forward<Tasks>(tasks)...);
    }

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
