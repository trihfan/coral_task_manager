#include "pinned_task_queue.h"
#include "task.h"

using namespace coral::task_manager;

pinned_task_queue::pinned_task_queue(size_t size) : buffer(size), head(&tail)
{
    tail.next = nullptr;
}

void pinned_task_queue::push(task_t task)
{
    // Allocate leaf from free list
    pinned_task_queue_leaf_t leaf = &buffer[index.fetch_add(1, std::memory_order_relaxed) & config::get_max_task_count_mask()];
    leaf->task = task;

    // Add leaf
    leaf->next = nullptr;
    pinned_task_queue_leaf_t previous = head.exchange(leaf);
    previous->next = leaf;

    // Signal the semaphore for any waiting thread
    semaphore.release();
}

task_t pinned_task_queue::pop(bool wait_for_semaphore)
{
    // If the list is not empty and we can wait for the semaphore, dot it
    if (wait_for_semaphore && head.load(std::memory_order_relaxed) != &tail)
    {
        semaphore.acquire();
    }

    // Get next task
    pinned_task_queue_leaf_t tail1 = tail.next;
    if (tail1)
    {
        // Check if there is more than one task, if that's the case we can directly pop this leaf
        pinned_task_queue_leaf_t tail2 = tail1->next;
        if (tail2)
        {
            tail.next = tail2;
        }
        // If there is only one leaf, we must exchange it while other threads can access it
        else
        {
            tail.next = nullptr;
            pinned_task_queue_leaf_t newHead = tail1;
            
            // If false, a task has been added while we were trying to exchange it
            if (!head.compare_exchange_strong(newHead, &tail))
            {
                // Just wait for the new task to be correctly written
                while (tail1->next == nullptr) {;} 

                tail.next = tail1->next.load();
                tail1->next = nullptr;
            }
        }
    }

    return tail1 ? tail1->task : nullptr;
}

void pinned_task_queue::clear()
{
}

void pinned_task_queues::clear()
{
    for (size_t i = 0; i < queues.size(); i++)
    {
        delete queues[i];
    }
    queues.clear();
}

size_t pinned_task_queues::size()
{
    return count;
}

void pinned_task_queues::init(size_t size)
{
    count = size;
    queues.resize(size);
    for (size_t i = 0; i < size; i++)
    {
        queues[i] = new pinned_task_queue(config::get_max_task_count());
    }
}

pinned_task_queue* pinned_task_queues::get(size_t index)
{
    return queues[index];
}
