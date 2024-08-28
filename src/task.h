#pragma once
#include <atomic>
#include <array>
#include <type_traits>
#include <cstdint>
#include "Config.h"
#include "ArgumentPacker.h"
#include "FunctionTraits.h"
#include "TaskBuffer.h"

namespace coral::taskmanager
{
    struct Task;
    typedef void (*TaskFunction)(Task*, const void*);
    static constexpr int DataSize = config::TaskSizeBytes - sizeof(TaskFunction) - sizeof(Task*) - sizeof(std::atomic<int32_t>) - sizeof(uint8_t);

    static constexpr uint8_t AnyThreadIndex = std::numeric_limits<uint8_t>::max();

    // The task struct, contains the function and the data
    struct Task
    {
        TaskFunction function;                  // task function to execute
        Task* parent = nullptr;                 // optional task parent
        std::atomic<int32_t> remaining = 0;     // remaining work for the task (current + children)
        char data[DataSize];                    // task data
        uint8_t threadIndex = AnyThreadIndex;                
    };

    // asserts
    static_assert(sizeof(Task) == config::TaskSizeBytes, "Wrong task size");
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "uint32_t is not lock free");

    /************************************************************/
    // Call the function getting the arguments from the data buffer
    template <typename Function, std::size_t... IndexSequence>
    inline void Apply(const Function function, const void* data, std::index_sequence<IndexSequence...>)
    {
        using traits = FunctionTraits<Function>;
        function(items::get<IndexSequence, std::tuple_element_t<IndexSequence, typename traits::arguments>...>(data)...);
    }
}
