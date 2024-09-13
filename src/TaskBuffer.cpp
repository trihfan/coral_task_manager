#include "TaskBuffer.h"
#include "Task.h"
#include "WorkerThread.h"
#include "Config.h"
#include <vector>
#include <cassert>
#include <algorithm>

using namespace coral::taskmanager;

// TaskBuffer data
namespace internal
{
    // Task buffer, split by thread
    static TaskData* TaskBuffer;
    static std::vector<TaskId> TaskBufferIndices;
}

// TaskBuffer methods
void TaskBuffer::Init(size_t threadCount)
{
    internal::TaskBuffer = new TaskData[Config::GetMaxTaskCount() * threadCount];
    internal::TaskBufferIndices.resize(threadCount, 0);
}

void TaskBuffer::Clear()
{
    delete[] internal::TaskBuffer;
    internal::TaskBufferIndices.clear();
}

TaskHandle TaskBuffer::Allocate()
{
    TaskHandle handle;

    const auto thread = WorkerThread::GetThreadIndex();
    handle.id = thread * Config::GetMaxTaskCount() + (internal::TaskBufferIndices[thread]++ & Config::GetMaxTaskCountMask()) + 1;
    assert(internal::TaskBuffer[handle.id - 1].remaining == 0);

#ifndef NDEBUG
    handle.data = &internal::TaskBuffer[handle.id - 1];
#endif

    return handle;
}

TaskData* TaskBuffer::Get(TaskId id)
{
    return &internal::TaskBuffer[id - 1];
}