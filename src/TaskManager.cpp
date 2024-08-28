#include "TaskManager.h"
#include "TaskBuffer.h"

using namespace coral::taskmanager;

void Manager::Start(int threadCount)
{
    TaskBuffer::Init(threadCount);
    Random<Xoshiro256Plus>::Init(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    WorkStealingQueues::Init(threadCount);
    PinnedTaskQueues::Init(threadCount);

    for (int i = 1; i < threadCount; i++)
    {
        threads.push_back(std::make_unique<WorkerThread>(i));
    }

    for (auto& thread : threads)
    {
        thread->Run();
    }
}

void Manager::Stop()
{
    for (auto& thread : threads)
    {
        thread->Cancel();
    }

    for (auto& thread : threads)
    {
        thread->Join();
    }

    threads.clear();
    WorkStealingQueues::Clear();
    TaskBuffer::Clear();
}
