#include "task_manager.h"
#include "task_buffer.h"

using namespace coral::task_manager;

void manager::start(int threadCount)
{
    task_buffer::init(threadCount);
    random<xoshiro_256_plus>::init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    work_stealing_queues::init(threadCount + 1);
    
    for (int i = 0; i < threadCount; i++)
    {
        threads.push_back(std::make_unique<worker_thread>(i + 1));
    }

    for (auto& thread : threads)
    {
        thread->run();
    }
}

void manager::stop()
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
    work_stealing_queues::clear();
    task_buffer::clear();
}