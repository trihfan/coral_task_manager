#include "task_manager.h"

using namespace coral::task_manager;

void TaskManager::start(int threadCount)
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

void TaskManager::stop()
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