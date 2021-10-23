#pragma once
#include <tuple>

namespace coral::task_manager
{
    /*
     * Item extraction from arguments list
     */
    template <size_t index, typename... Type>
    struct Item;

    // recursive case
    template <size_t index, typename Type, typename... Other>
    struct Item<index, Type, Other...> : Item<index - 1, Other...> {};
    
    // base case
    template <typename Type, typename... Other>
    struct Item<0, Type, Other...> 
    {
        using type = Type;
    };

    /*
     * Size of items
     */
    template <size_t Index>
    static constexpr size_t offset()
    {
        return 0;
    };

    template <size_t index, typename Type, typename... Others>
    static constexpr size_t offset()
    {
        return index == 0 ? 0 : sizeof(Type) + offset<index - 1, Others...>();
    };

    /**
     * 
     */
    template <typename... Args>
    class Items
    {
    public:
        // Return the value at the given index in from
        template <size_t index>
        static typename Item<index, Args...>::type& get(const void* from)
        {
            auto item = static_cast<void*>(static_cast<char*>(const_cast<void*>(from)) + offset<index, Args...>());
            return *reinterpret_cast<typename Item<index, Args...>::type*>(item);
        }

        // Copy all data to the buffer
        template <typename Ptr>
        static void copy(Ptr destination, const Args&... args)
        {
            static constexpr std::size_t count = sizeof...(Args);
            static_assert(offset<count, Args...>() <= 44, "Data should be less than 44 bytes");
            copy<0>(static_cast<void*>(destination), args...);
        }

        static size_t count()
        {
            return sizeof...(Args);
        }

    private:
        // Copy item
        template <size_t Index>
        static void copy(void* destination) {}

        template <size_t Index, typename Type, typename... Others>
        static void copy(void* destination, const Type& item, const Others&... others)
        {
            auto dest = static_cast<void*>(static_cast<char*>(destination) + offset<Index, Args...>());
            std::memcpy(dest, &item, sizeof(Type));
            copy<Index + 1>(destination, others...);
        }
    };
}
