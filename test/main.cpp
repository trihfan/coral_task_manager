#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <chrono>
#include <iostream>
#include <future>
#include <atomic>
#include "coral_task_manager.h"
#include "worker_thread.h"

using namespace std::chrono;
using namespace coral;

class TestFixture 
{
public:
    TestFixture() { task_manager::manager::start(); }
    ~TestFixture() { task_manager::manager::stop(); }
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
        }, benchmark_thread_count);
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
    // Using task_manager
    task_manager::manager::start(benchmark_thread_count);
    start = steady_clock::now();

    // Start threads
    auto parent = task_manager::create_task();
    for (int i = 0; i < benchmark_thread_count; i++) 
    {
        auto task = task_manager::create_child_task(parent, [](int i)
        {
            bench_method(i);
        }, benchmark_thread_count);
        task_manager::run(task);
    }

    // Wait
    task_manager::run(parent);
    task_manager::wait(parent);

    // Get time
    auto time_for_task_manager = steady_clock::now() - start;
    std::cout << "time for task_manager: " << duration_cast<microseconds>(time_for_task_manager).count() / 1000. << "ms" << std::endl;
    task_manager::manager::stop();

    // Verify time
    CHECK(time_for_task_manager <= time_for_future);
}

TEST_CASE("StartAndStop") 
{
    task_manager::start(1);
    task_manager::stop();

    task_manager::start(4);
    task_manager::stop();

    task_manager::start(8);
    task_manager::stop();
}

TEST_CASE_FIXTURE(TestFixture, "SequencedTasks")
{
    std::array<int, 4000> values { 0 };
    for (size_t i = 0; i < values.size(); i++)
    {
        auto task = task_manager::create_task([](int* value)
        {
            *value = 2;
        }, &values[i]);
        task_manager::run(task);
        task_manager::wait(task);
    }

    for (auto value : values)
    {
        CHECK(value == 2);
    }
}

TEST_CASE_FIXTURE(TestFixture, "BatchTasks")
{
    auto parent = task_manager::create_task();
    std::array<int, 4000> values { 0 };
    for (size_t i = 0; i < values.size(); i++)
    {
        auto task = task_manager::create_child_task(parent, [](int* value) 
        {
            *value = 2;
        }, &values[i]);
        task_manager::run(task);
    }

    task_manager::run(parent);
    task_manager::wait(parent);

    for (auto value : values)
    {
        CHECK(value == 2);
    }
}

TEST_CASE_FIXTURE(TestFixture, "LambdaTest1")
{
    auto parent = task_manager::create_task();
    auto last = parent;
    std::atomic<int> total = 2;
    std::vector<task_manager::task_t> tasks;
    for (int i = 0; i < 1000; i++)
    {
        last = task_manager::create_child_task(last, [](std::atomic<int>* total, int value) 
        {
            *total += value;
        }, &total, i);
        tasks.push_back(last);
    }

    for (auto task : tasks)
    {
        task_manager::run(task);
    }

    task_manager::run(parent);
    task_manager::wait(parent);
    CHECK(total == 499502);
}

TEST_CASE_FIXTURE(TestFixture, "Pinned")
{
    for (int i = 0; i < std::thread::hardware_concurrency(); i++)
    {
        task_manager::manager::set_execute_only_pinned_tasks(i);
    }

    auto start = steady_clock::now();

    for (int i = 0; i < 1000; i++)
    {
        auto parent = task_manager::create_task();
        parent->threadIndex = 0;

        for (int j = 0; j < 1000; j++)
        {
            uint8_t index = j % std::thread::hardware_concurrency();
            auto task = task_manager::create_child_task(parent, [](uint8_t index) 
            {
                CHECK(task_manager::worker_thread::get_thread_index() == index);
            }, index);
            task->threadIndex = index;
            task_manager::run(task);
        }

        task_manager::run(parent);
        task_manager::wait(parent);
    }

    auto elapsed = steady_clock::now() - start;
    std::cout << "time for pined: " << duration_cast<microseconds>(elapsed).count() / 1000. << "ms" << std::endl;
}