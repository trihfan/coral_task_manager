#include "task_manager.h"
#include "task_buffer.h"

using namespace coral::task_manager;

void manager::start(int threadCount)
{
    task_buffer::init(threadCount);
    random<xoshiro_256_plus>::init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    work_stealing_queues::init(threadCount);
    pinned_task_queues::init(threadCount);

    for (int i = 1; i < threadCount; i++)
    {
        threads.push_back(std::make_unique<worker_thread>(i));
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

void manager::set_execute_only_pinned_tasks(uint8_t thread_id, bool only_pinned_tasks)
{
    threads[thread_id]->set_execute_only_pinned_tasks(only_pinned_tasks);
}
