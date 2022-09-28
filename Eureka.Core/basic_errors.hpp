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

    class file_load_error : public std::runtime_error
    {
    public:
        file_load_error(const char* what_arg) : std::runtime_error(what_arg) {}
        file_load_error(const std::string& what_arg) : std::runtime_error(what_arg) {}
        file_load_error(const std::filesystem::path& what_arg) : std::runtime_error(what_arg.string()) {}
    };

    class operation_cancelled : public std::runtime_error
    {
    public:
        operation_cancelled(const char* what_arg) : std::runtime_error(what_arg) {}
        operation_cancelled(const std::string& what_arg) : std::runtime_error(what_arg) {}
    };

    class connection_failed : public std::runtime_error
    {
    public:
        connection_failed(const char* what_arg) : std::runtime_error(what_arg) {}
        connection_failed(const std::string& what_arg) : std::runtime_error(what_arg) {}
    };
}