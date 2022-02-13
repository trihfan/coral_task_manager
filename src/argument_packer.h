#pragma once
#include <tuple>
#include <cstring>

namespace coral::task_manager::items
{
    // Item extraction from item list
    template <size_t Index, typename... Items>
    struct item;

    // Recursive case
    template <size_t Index, typename FirstItem, typename... OtherItems>
    struct item<Index, FirstItem, OtherItems...> : item<Index - 1, OtherItems...> {};
    
    // Base case
    template <typename FirstItem, typename... OtherItems>
    struct item<0, FirstItem, OtherItems...> 
    {
        using type = FirstItem;
    };

    // Offset in bytes for the item at given index
    template <size_t Index>
    static constexpr size_t offset()
    {
        return 0;
    };

    template <size_t Index, typename FirstItem, typename... OtherItems>
    static constexpr size_t offset()
    {
        return Index == 0 ? 0 : sizeof(FirstItem) + offset<Index - 1, OtherItems...>();
    };

    // Extract an item from the data buffer
    template <size_t Index, typename... Items>
    static typename item<Index, Items...>::type& get(const void* from)
    {
        auto data = static_cast<void*>(static_cast<char*>(const_cast<void*>(from)) + offset<Index, Items...>());
        return *reinterpret_cast<typename item<Index, Items...>::type*>(data);
    }

    // Count items
    template <typename... Items>
    static size_t count()
    {
        return sizeof...(Items);
    }

    // Copy all data to the buffer
    namespace detail
    {
        static void copy(void* destination, size_t offset) {}

        template <typename FirstItem, typename... OtherItems>
        static void copy(void* destination, size_t offset, const FirstItem& item, const OtherItems&... others)
        {
            auto dest = static_cast<void*>(static_cast<char*>(destination) + offset);
            std::memcpy(dest, &item, sizeof(FirstItem));
            copy(destination, offset + sizeof(FirstItem), others...);
        }
    }
  
    template <typename Ptr, typename... Items>
    static void copy(Ptr destination, const Items&... items)
    {
        static constexpr std::size_t count = sizeof...(Items);
        static_assert(offset<count, Items...>() <= 44, "Data should be less than 44 bytes");
        detail::copy(static_cast<void*>(destination), 0, items...);
    }
}
