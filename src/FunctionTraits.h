#pragma once
#include <tuple>

namespace coral::taskmanager
{
    template <typename Type>
    struct FunctionTraits : public FunctionTraits<decltype(&Type::operator())> {};

    // Get function traits from template
    template <typename ClassType, typename ReturnType, typename... Args>
    struct FunctionTraits<ReturnType(ClassType::*)(Args...) const>
    {
        // Function return type
        using Return = ReturnType;

        // Arguments arity
        static constexpr auto Arity = sizeof...(Args);

        // Function arguments as a tuple
        using Arguments = std::tuple<Args...>;

        // Function class
        using Class = ClassType;
    };
}