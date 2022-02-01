#include "task_buffer.h"
#include "task.h"
#include "worker_thread.h"
#include "config.h"
#include <vector>

using namespace coral::task_manager;

// task_buffer data
namespace internal
{
    // Task buffer, split by thread
    static task_t task_buffer;
    static std::vector<uint32_t> task_buffer_indices;
}

// task_buffer methods
void task_buffer::init(uint32_t thread_count)
{
    internal::task_buffer = new task[config::get_max_task_count() * thread_count];
    internal::task_buffer_indices.resize(thread_count, 0);
}

void task_buffer::clear()
{
    delete[] internal::task_buffer;
    internal::task_buffer_indices.clear();
}

task_t task_buffer::allocate()
{
    const auto thread = worker_thread::get_thread_index();
    const uint32_t i = internal::task_buffer_indices[thread]++;
    assert(internal::task_buffer[thread * config::get_max_task_count() + (i & config::get_max_task_count_mask())].remaining == 0);
    return &internal::task_buffer[thread * config::get_max_task_count() + (i & config::get_max_task_count_mask())];
}
