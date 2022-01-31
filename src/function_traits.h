#pragma once
#include <tuple>

namespace coral::task_manager
{
    template <typename Type>
    struct function_traits : public function_traits<decltype(&Type::operator())>
    {};

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...) const>
    {
        // Function return type
        using return_type = ReturnType;

        // Arguments arity
        static constexpr auto arity = sizeof...(Args);

        // Function arguments as a tuple
        using arguments = std::tuple<Args...>;
    };
}
