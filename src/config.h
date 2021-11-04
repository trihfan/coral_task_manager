#pragma once

// config
#ifndef MAX_TASK_COUNT
#define MAX_TASK_COUNT 4096
#endif

#ifndef TASK_SIZE_BYTES
#define TASK_SIZE_BYTES 64
#endif

#ifdef _MSC_VER
#   include <windows.h>
#   define JOB_YIELD() YieldProcessor()
#   define JOB_COMPILER_BARRIER _ReadWriteBarrier()
#   define JOB_MEMORY_BARRIER std::atomic_thread_fence(std::memory_order_seq_cst);
#else
#   include <emmintrin.h>
#   define JOB_YIELD() _mm_pause()
#   define JOB_COMPILER_BARRIER asm volatile("" ::: "memory")
#   define JOB_MEMORY_BARRIER asm volatile("mfence" ::: "memory")
#endif

// config
namespace coral::task_manager
{
    static constexpr uint32_t maxTaskCount = MAX_TASK_COUNT;
    static constexpr uint32_t taskSizeBytes = TASK_SIZE_BYTES;
    static constexpr uint32_t maxTaskCountMask = MAX_TASK_COUNT - 1u;

    static_assert(maxTaskCount % 2 == 0, "MAX_TASK_COUNT must be a power of two");
}