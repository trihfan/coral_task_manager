#include "TaskBuffer.h"
#include "Task.h"
#include "WorkerThread.h"
#include "config.h"
#include <vector>
#include <cassert>
#include <algorithm>

using namespace coral::taskmanager;

// TaskBuffer data
namespace internal
{
    // Task buffer, split by thread
    static Task* TaskBuffer;
    static std::vector<uint32_t> TaskBufferIndices;
}

// TaskBuffer methods
void TaskBuffer::Init(uint32_t threadCount)
{
    internal::TaskBuffer = new Task[config::GetMaxTaskCount() * threadCount];
    internal::TaskBufferIndices.resize(threadCount, 0);
}

void TaskBuffer::Clear()
{
    delete[] internal::TaskBuffer;
    internal::TaskBufferIndices.clear();
}

Task* TaskBuffer::Allocate()
{
    const auto thread = WorkerThread::GetThreadIndex();
    const uint32_t i = internal::TaskBufferIndices[thread]++;
    assert(internal::TaskBuffer[thread * config::GetMaxTaskCount() + (i & config::GetMaxTaskCountMask())].remaining == 0);
    return &internal::TaskBuffer[thread * config::GetMaxTaskCount() + (i & config::GetMaxTaskCountMask())];
}
