#include <gtest/gtest.h>
#include "task_manager.h"

using namespace coral;

class TaskManagerFixture : public ::testing::Test 
{
protected:
    void SetUp() override { task_manager::TaskManager::start(); }
    void TearDown() override { task_manager::TaskManager::stop(); }
};

int main(int argc, char** argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(TaskManager, StartAndStop) 
{
    task_manager::start(1);
    task_manager::stop();

    task_manager::start(4);
    task_manager::stop();

    task_manager::start(8);
    task_manager::stop();
}

TEST_F(TaskManagerFixture, PushMonoEmpty) 
{
    for (int i = 0; i < 4000; i++)
    {
        auto task = task_manager::createTask();
        task_manager::run(task);
        task_manager::wait(task);
    }
}

TEST_F(TaskManagerFixture, PushMonoFull)
{
    std::array<int, 4000> values { 0 };
    for (size_t i = 0; i < values.size(); i++)
    {
        auto task = task_manager::createTask([](auto task, auto data)
        {
            using Items = task_manager::Items<int*>;
            *Items::get<0>(data) = 2;
        }, &values[i]);
        task_manager::run(task);
        task_manager::wait(task);
    }

    for (auto value : values)
    {
        ASSERT_EQ(value, 2);
    }
}

TEST_F(TaskManagerFixture, PushMultiFull)
{
    auto parent = task_manager::createTask();
    std::array<int, 4000> values { 0 };
    for (size_t i = 0; i < values.size(); i++)
    {
        auto task = task_manager::createChildTask(parent, [](auto task, auto data) 
        {
            using Items = task_manager::Items<int*>;
            *Items::get<0>(data) = 2;
        }, &values[i]);
        task_manager::run(task);
    }

    task_manager::run(parent);
    task_manager::wait(parent);

    for (auto value : values)
    {
        ASSERT_EQ(value, 2);
    }
}