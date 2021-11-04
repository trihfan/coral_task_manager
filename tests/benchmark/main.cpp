#include <hayai/hayai.hpp>
#include <thread>

#define MAX_TASK_COUNT 2097152
#include "task_manager.h"

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
    void SetUp() override { task_manager::TaskManager::start(7); }
    void TearDown() override { task_manager::TaskManager::stop(); }
};

//************************************************************************************************

BENCHMARK_F(TaskManagerFixture, Nothing, 1, 1)
{
}