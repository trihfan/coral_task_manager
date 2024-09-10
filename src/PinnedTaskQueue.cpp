#include "PinnedTaskQueue.h"
#include "Task.h"

using namespace coral::taskmanager;

PinnedTaskQueue::PinnedTaskQueue(size_t bufferSize) : buffer(bufferSize), head(&tail)
{
    tail.next = nullptr;
}

void PinnedTaskQueue::Push(TaskHandle task)
{
    #ifndef NDEBUG
    // Check if there is still some empty space in the queue
    debugSize.fetch_add(1);
    assert(debugSize < buffer.size());
    #endif

    // Allocate a leaf from free list
    PinnedTaskQueueLeaf* leaf = &buffer[index.fetch_add(1, std::memory_order_relaxed) & config::GetMaxTaskCountMask()];
    leaf->task = task;

    // Add leaf
    leaf->next = nullptr;
    PinnedTaskQueueLeaf* previous = head.exchange(leaf);
    previous->next = leaf;

    // Signal the semaphore for any waiting thread
    semaphore.release();
}

TaskHandle PinnedTaskQueue::Pop(bool waitForSemaphore)
{
    // If the list is not empty and we should wait for the semaphore, dot it
    if (head.load(std::memory_order_acquire) == &tail)
    {
        if (waitForSemaphore)
        {
            semaphore.acquire();
        }
        else
        {
            return NullTask;
        }        
    }

    // Get the next tast
    PinnedTaskQueueLeaf* tailPlusOne = tail.next;
    if (tailPlusOne)
    {
        // Check if there is more than one task, if that's the case we can directly pop this leaf
        PinnedTaskQueueLeaf* tailPlusTwo = tailPlusOne->next;
        if (tailPlusTwo)
        {
            tail.next = tailPlusTwo;
        }
        // If there is only one leaf, we must exchange it while other threads can access it
        else
        {
            tail.next = nullptr;
            PinnedTaskQueueLeaf* newHead = tailPlusOne;
            
            // If false, a task has been added while we were trying to exchange it
            if (!head.compare_exchange_strong(newHead, &tail))
            {
                // Just wait for the new task to be correctly written
                while (tailPlusOne->next == nullptr) {;} 

                tail.next = tailPlusOne->next.load();
                tailPlusOne->next = nullptr;
            }
        }
    }

    #ifndef NDEBUG
    if (tailPlusOne)
    {
        debugSize.fetch_sub(1);
    }
    #endif

    return tailPlusOne ? tailPlusOne->task : NullTask;
}

void PinnedTaskQueue::Clear()
{
    #ifndef NDEBUG
    debugSize.store(0);
    #endif
}

void PinnedTaskQueues::Clear()
{
    for (size_t i = 0; i < queues.size(); i++)
    {
        delete queues[i];
    }
    queues.clear();
}

size_t PinnedTaskQueues::Size()
{
    return count;
}

void PinnedTaskQueues::Init(size_t size)
{
    count = size;
    queues.resize(size);

    for (size_t i = 0; i < queues.size(); i++)
    {
        queues[i] = new PinnedTaskQueue(config::GetMaxTaskCount());
    }
}

PinnedTaskQueue* PinnedTaskQueues::Get(size_t index)
{
    return queues[index];
}
