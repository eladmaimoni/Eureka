#pragma once

#include "basic_concepts.hpp"

namespace eureka
{

    //[[nodiscard]] constexpr decltype(auto) steal(std::movable auto& arg) noexcept
    //{
    //    using T = std::remove_reference_t<decltype(arg)>&&;

    //    if constexpr (std::is_trivially_move_assignable_v<T>)
    //    {

    //    }

    //    return static_cast<T&&>(arg);

    //}

    template <typename T>
    [[nodiscard]] T* steal(T*& p)
    {
        auto tmp = p;
        p = nullptr;
        return tmp;
    }
}

