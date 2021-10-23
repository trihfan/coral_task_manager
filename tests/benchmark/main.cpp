#include <hayai/hayai.hpp>
#include "task_manager.h"
#include <thread>
#include "Random.h"

using namespace coral;

int main(int argc, char** argv) 
{
    hayai::ConsoleOutputter consoleOutputter;
    hayai::Benchmarker::AddOutputter(consoleOutputter);
    hayai::Benchmarker::RunAllTests();
    return 0;
}

/**
 * Fixture to start and stop the task manager
 */ 
class TaskManagerFixture : public ::hayai::Fixture
{
public:
    void SetUp() override { task_manager::TaskManager::start(); }
    void TearDown() override { task_manager::TaskManager::stop(); }
};

BENCHMARK_F(TaskManagerFixture, Test1, 10, 10)
{
    auto parent = task_manager::createTask();
    for (int i = 0; i < 4000; i++)
    {
        auto task = task_manager::createTask([](auto data) 
        { 
            for (int i = 0; i < 1000; i++)
            {
                task_manager::Random<task_manager::Xoshiro256plus>::random();
            }
        });

        task_manager::run(task);
    }
    task_manager::run(parent);
    task_manager::wait(parent);
}