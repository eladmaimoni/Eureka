#pragma once
#include <vector>

namespace eureka
{
    template<typename T, typename _Alloc = std::allocator<T>>
    class fixed_capacity_vector final : private std::vector<T, _Alloc>
    {
        using base_t = std::vector<T, _Alloc>;
    public:
        using base_t::at;
        using base_t::operator[];
        using base_t::begin;
        using base_t::end;
        using base_t::data;
        fixed_capacity_vector() = default;
        fixed_capacity_vector(std::size_t capacity) 
            : base_t(capacity)
        {
            base_t::resize(0);
        };
        fixed_capacity_vector(fixed_capacity_vector&& that) = default;
        fixed_capacity_vector& operator=(fixed_capacity_vector&& rhs) = default;
        fixed_capacity_vector(base_t&& that) : base_t(std::move(that)) {};
        fixed_capacity_vector& operator=(base_t&& rhs)
        {
            return base_t::operator=(std::move(rhs));
        }

        void resize(std::size_t size)
        {
            assert(size <= base_t::capacity());
            base_t::resize(size);
        }
        template <class... _Valty>
        T& emplace_back(_Valty&&... _Val)
        {
            assert(base_t::size() < base_t::capacity());
       
            return base_t::emplace_back(std::forward<_Valty>(_Val)...);
        }
        ~fixed_capacity_vector() = default;
    };

}

