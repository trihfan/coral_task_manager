#pragma once

// config
#ifndef MAX_TASK_COUNT
#define MAX_TASK_COUNT 4096
#endif

#ifndef TASK_SIZE_BYTES
#define TASK_SIZE_BYTES 64
#endif

// config
namespace coral::task_manager
{
    static constexpr int maxTaskCount = MAX_TASK_COUNT;
    static constexpr int taskSizeBytes = TASK_SIZE_BYTES;

    static_assert(maxTaskCount % 2 == 0, "MAX_TASK_COUNT must be a power of two");
}