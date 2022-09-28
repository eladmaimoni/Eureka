

namespace eureka
{

    template <typename First, typename Second>
    inline constexpr bool any_equal_to_first(First&& first, Second&& second)
    {
        return (first == second);
    }

    template <typename First, typename Second, typename ... Args>
    inline constexpr bool any_equal_to_first(First&& first, Second&& second, Args&& ... args)
    {
        return (first == second) || any_equal_to_first(std::forward<First>(first), std::forward<Args>(args) ...);
    }

    template <typename First, typename Second>
    inline constexpr bool any_not_equal_to_first(First&& first, Second&& second)
    {
        return (first != second);
    }

    template <typename First, typename Second, typename ... Args>
    inline constexpr bool any_not_equal_to_first(First&& first, Second&& second, Args&& ... args)
    {
        return (first != second) || any_not_equal_to_first(std::forward<First>(first), std::forward<Args>(args) ...);
    }

    template <typename First, typename Second>
    inline constexpr bool all_not_equal_to_first(First&& first, Second&& second)
    {
        return (first != second);
    }

    template <typename First, typename Second, typename ... Args>
    inline constexpr bool all_not_equal_to_first(First&& first, Second&& second, Args&& ... args)
    {
        return (first != second) && all_not_equal_to_first(std::forward<First>(first), std::forward<Args>(args) ...);
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
    static_assert(any_not_equal_to_first(4, 4, 3, 1));
    static_assert(!any_not_equal_to_first(4, 4, 4, 4));

    static_assert(!any_pair_is_equal(1, 2, 3, 4));
    static_assert(any_pair_is_equal(4, 4, 3, 1));
    static_assert(any_pair_is_equal(4, 5, 3, 3));
    static_assert(any_pair_is_equal(4, 5, 3, 3));
    static_assert(any_pair_is_equal(4, 5, 4, 4));

    template<typename Callable>
    class scoped_raii
    {
        Callable _callable;
    public:
        scoped_raii(Callable&& callable)
            : _callable(std::move(callable))
        {

        }
        scoped_raii(const scoped_raii&) = delete;
        scoped_raii& operator=(const scoped_raii&) = delete;
        scoped_raii(scoped_raii&&) = delete;
        scoped_raii& operator=(scoped_raii&&) = delete;

        ~scoped_raii()
        {
            _callable();
        }
    };


    inline bool append_until_delimiter(std::istream& is, std::string& lineSoFar, char delim = '\n')
    {   
        std::streamsize charsRead = 0;
        do 
        {
            char inChar;
            charsRead = is.readsome(&inChar, 1);
            if (charsRead == 1) 
            {
                lineSoFar.append(1, inChar);        
                if (inChar == delim) 
                {
                    return true; // delimiter is reached 
                }
            }
        } while (charsRead != 0);

        return false;
    }
}