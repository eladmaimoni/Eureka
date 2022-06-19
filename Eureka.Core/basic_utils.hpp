

namespace eureka
{
    template <typename First, typename Second>
    inline constexpr bool any_not_equal_to_first(First&& first, Second&& second)
    {
        return (first != second);
    }

    template <typename First, typename Second, typename ... Args>
    inline constexpr bool any_not_equal_to_first(First&& first, Second&& second, Args&& ... args)
    {
        return (first != second) && any_not_equal_to_first(std::forward<First>(first), std::forward<Args>(args) ...);
    }


    template <typename First, typename Second>
    inline constexpr bool any_pair_is_equal(First&& first, Second&& second)
    {
        return (first == second);
    }

    template <typename First, typename Second, typename ... Args>
    inline constexpr bool any_pair_is_equal(First&& first, Second&& second, Args&& ... args)
    {
        return (first == second) || any_pair_is_equal(std::forward<First>(first), std::forward<Args>(args) ...) || any_pair_is_equal(std::forward<First>(second), std::forward<Args>(args) ...);
    }


    static_assert(any_not_equal_to_first(1, 2, 3, 4));
    static_assert(!any_not_equal_to_first(4, 4, 3, 1));

    static_assert(!any_pair_is_equal(1, 2, 3, 4));
    static_assert(any_pair_is_equal(4, 4, 3, 1));
    static_assert(any_pair_is_equal(4, 5, 3, 3));
    static_assert(any_pair_is_equal(4, 5, 3, 3));
    static_assert(any_pair_is_equal(4, 5, 4, 4));
}