#pragma once

// config
#ifndef MAX_TASK_COUNT
#define MAX_TASK_COUNT 4096
#endif

#ifndef TASK_SIZE_BYTES
#define TASK_SIZE_BYTES 64
#endif

// config
namespace coral::task_manager::config
{
    /*struct config
    {
        uint32_t max_task_count;
        uint32_t task_memory_size;
    };*/
    static constexpr uint32_t maxTaskCount = MAX_TASK_COUNT;
    static constexpr uint32_t taskSizeBytes = TASK_SIZE_BYTES;
    static constexpr uint32_t maxTaskCountMask = MAX_TASK_COUNT - 1u;

    static_assert(maxTaskCount % 2 == 0, "MAX_TASK_COUNT must be a power of two");
}