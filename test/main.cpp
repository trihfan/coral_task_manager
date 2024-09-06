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

// Benchmark to check implementation is not slower than async
static constexpr auto benchmark_n_time = 1000;
static constexpr auto benchmark_thread_count = 4;
void bench_method(double start)
{
    static double tmp = start;
    for (int j = 0; j < benchmark_n_time; j++)
    {
        tmp = std::sqrt(tmp);
    }
}

TEST_CASE("Benchmark thread overhead") 
{
    // --------------------------------------------
    // Using future
    auto start = steady_clock::now();
    std::array<std::future<void>, benchmark_thread_count> futures;

    // Start threads
    for (int i = 0; i < futures.size(); i++)
    {
        futures[i] = std::async(std::launch::async, [](int i)
        {
            bench_method(i);
        }, i);
    }

    // Wait
    for (int i = 0; i < futures.size(); i++)
    {
        futures[i].get();
    }

    // Get time
    auto time_for_future = steady_clock::now() - start;
    std::cout << "time for future: " << duration_cast<microseconds>(time_for_future).count() / 1000. << "ms" << std::endl;

    // --------------------------------------------
    // Using taskmanager
    taskmanager::Start(benchmark_thread_count);
    start = steady_clock::now();

    // Start threads
    auto parent = taskmanager::CreateTask();
    for (int i = 0; i < benchmark_thread_count; i++) 
    {
        auto task = taskmanager::CreateChildTask(parent, [i](auto, auto)
        {
            bench_method(i);
        });
        taskmanager::Run(task);
    }

    // Wait
    taskmanager::Run(parent);
    taskmanager::Wait(parent);

    // Get time
    auto time_for_taskmanager = steady_clock::now() - start;
    std::cout << "time for taskmanager: " << duration_cast<microseconds>(time_for_taskmanager).count() / 1000. << "ms" << std::endl;
    taskmanager::Stop();
}

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
    auto start = steady_clock::now();

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

    auto elapsed = steady_clock::now() - start;
    std::cout << "time for batch: " << duration_cast<microseconds>(elapsed).count() / 1000. << "ms" << std::endl;
}

TEST_CASE_FIXTURE(TestFixture, "LambdaTest1")
{
    auto parent = taskmanager::CreateTask();
    auto last = parent;
    std::atomic<int> total = 2;
    std::vector<taskmanager::Task*> tasks;
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

    auto start = steady_clock::now();

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

    auto elapsed = steady_clock::now() - start;
    std::cout << "time for pinned: " << duration_cast<microseconds>(elapsed).count() / 1000. << "ms" << std::endl;
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