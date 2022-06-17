#pragma once
#include <stdexcept>


namespace eureka
{
    class file_not_found_error : public std::runtime_error
    {
    public:
        file_not_found_error(const char* what_arg) : std::runtime_error(what_arg) {}
        file_not_found_error(const std::string& what_arg) : std::runtime_error(what_arg) {}
        file_not_found_error(const std::filesystem::path& what_arg) : std::runtime_error(what_arg.string()) {}
    };
}