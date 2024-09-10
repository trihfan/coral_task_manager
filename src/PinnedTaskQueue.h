#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include <semaphore>
#include "Config.h"
#include "Task.h"

namespace coral::taskmanager
{
    struct Task;

    // Queue of task that will be pinned to a given thread, only this thread will be able to execute it
    // We use a semaphore to be able to let the thread sleep while waiting for new task to arrive (in case the thread
    // is configured to execute only pinned tasks)
    class PinnedTaskQueue
    {
    public:
        // Costruct with the pre allocated size of the queue
        PinnedTaskQueue(size_t bufferSize);

        // Push and pop tasks
        void Push(TaskHandle task);
        TaskHandle Pop(bool waitForSemaphore); 

        // Clear the queue
        void Clear();

        // On cancel, signal the semaphore to prevent infinite wait
        void Cancel() { semaphore.release(); }

    private:
        struct PinnedTaskQueueLeaf;

        struct PinnedTaskQueueLeaf
        {
            TaskHandle task;
            std::atomic<PinnedTaskQueueLeaf*> next;
        };

        std::binary_semaphore semaphore { 0 };
        
        // Free list
        std::vector<PinnedTaskQueueLeaf> buffer;
        std::atomic<int64_t> index { 0 };

        // Internal lock free list
        std::atomic<PinnedTaskQueueLeaf*> head;
        PinnedTaskQueueLeaf tail;

        // Debug data
        #ifndef NDEBUG
        std::atomic<size_t> debugSize { 0 };
        #endif
    };

    // Container to encapsulate a PinnedTaskQueue per thread
    class PinnedTaskQueues
    {
    public:
        // Initialization
        static void Init(size_t size);
        static void Clear();
        
        static size_t Size();
        static PinnedTaskQueue* Get(size_t index);

    private:
        inline static size_t count = 0;
        inline static std::vector<PinnedTaskQueue*> queues;
    };
}
