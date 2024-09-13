#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <chrono>
#include <iostream>
#include <future>
#include <atomic>
#include "CoralTaskManager.h"
#include "WorkerThread.h"

using namespace std::chrono;
using namespace coral;

class TestFixture 
{
public:
    TestFixture() { taskmanager::Start(); }
    ~TestFixture() { taskmanager::Stop(); }
};

TEST_CASE("StartAndStop") 
{
    taskmanager::Start(1);
    taskmanager::Stop();

    taskmanager::Start(4);
    taskmanager::Stop();

    taskmanager::Start(8);
    taskmanager::Stop();
}

TEST_CASE_FIXTURE(TestFixture, "SequencedTasks")
{
    std::array<int, 4000> values { 0 };
    for (size_t i = 0; i < values.size(); i++)
    {
        auto task = taskmanager::CreateTask([&values, i](auto, auto)
        {
            values[i] = 2;
        });
        taskmanager::Run(task);
        taskmanager::Wait(task);
    }

    for (auto value : values)
    {
        CHECK(value == 2);
    }
}

TEST_CASE_FIXTURE(TestFixture, "BatchTasks")
{
    auto parent = taskmanager::CreateTask();
    std::array<int, 4000> values { 0 };
    for (size_t i = 0; i < values.size(); i++)
    {
        auto task = taskmanager::CreateChildTask(parent, [&values, i](auto, auto)
        {
            values[i] = 2;
        });
        taskmanager::Run(task);
    }

    taskmanager::Run(parent);
    taskmanager::Wait(parent);

    for (auto value : values)
    {
        CHECK(value == 2);
    }
}

TEST_CASE_FIXTURE(TestFixture, "LambdaTest1")
{
    auto parent = taskmanager::CreateTask();
    auto last = parent;
    std::atomic<int> total = 2;
    std::vector<taskmanager::TaskHandle> tasks;
    for (int i = 0; i < 1000; i++)
    {
        last = taskmanager::CreateChildTask(last, [&total, i](auto, auto) 
        {
            total += i;
        });
        tasks.push_back(last);
    }

    for (auto task : tasks)
    {
        taskmanager::Run(task);
    }

    taskmanager::Run(parent);
    taskmanager::Wait(parent);
    CHECK(total == 499502);
}

TEST_CASE_FIXTURE(TestFixture, "Pinned")
{
    for (int i = 0; i < std::thread::hardware_concurrency(); i++)
    {
        taskmanager::SetExecuteOnlyPinnedTasks(i, true);
    }

    for (int i = 0; i < 1000; i++)
    {
        auto parent = taskmanager::CreateTask();

        for (int j = 0; j < 1000; j++)
        {
            uint8_t index = j % std::thread::hardware_concurrency();
            auto task = taskmanager::CreateChildTask(parent, [index](auto, auto) 
            {
                CHECK(taskmanager::WorkerThread::GetThreadIndex() == index);
            });
            taskmanager::Run(task, index);
        }

        taskmanager::Run(parent, 0);
        taskmanager::Wait(parent);
    }

    for (int i = 0; i < std::thread::hardware_concurrency(); i++)
    {
        taskmanager::SetExecuteOnlyPinnedTasks(i, false);
    }
}

TEST_CASE_FIXTURE(TestFixture, "Continuation")
{
    using UserData = taskmanager::UserData<std::atomic<int>*>;

    std::atomic<int> value = 0;

    auto parent = taskmanager::CreateTask([](auto, void* data) { UserData::Get<0>(data)->fetch_add(1); });
    UserData::Get<0>(parent->userData) = &value;
    
    std::vector<taskmanager::TaskHandle> tasks(16);

    for (size_t i = 0; i < tasks.size(); i++)
    {
        tasks[i] = taskmanager::CreateTask([](auto, void* data) { UserData::Get<0>(data)->fetch_add(1); });
        UserData::Get<0>(tasks[i]->userData) = &value;

        taskmanager::AddContinuation(parent, tasks[i], i % 2 == 0);
    }

    taskmanager::Run(parent);

    for (size_t i = 0; i < tasks.size(); i++)
    {
        taskmanager::Wait(tasks[i]);
    }

    CHECK(value.load() == 17);
}

TEST_CASE_FIXTURE(TestFixture, "ComputeScore")
{
    static constexpr int IterationCount = 10000;
    static constexpr int TaskCount = 1000;

    auto start = steady_clock::now();

    for (size_t i = 0; i < IterationCount; i++)
    {
        auto parent = taskmanager::CreateTask();
        for (size_t j = 0; j < TaskCount; j++)
        {
            auto task = taskmanager::CreateChildTask(parent, [](auto, auto)
            {
            });
            taskmanager::Run(task);
        }
        taskmanager::Run(parent);
        taskmanager::Wait(parent);
    }

    auto elapsed = steady_clock::now() - start;
    float score = static_cast<float>(duration_cast<microseconds>(elapsed).count()) / (IterationCount * TaskCount);
    std::cout << "Score: " << score << "us" << std::endl;
}