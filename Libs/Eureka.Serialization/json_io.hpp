#pragma once
#include <filesystem>

namespace eureka
{ 
    template <class T> void to_json_file(const T& t, const std::filesystem::path& f);
    template <class T> T from_json_file(const std::filesystem::path& f);
}

