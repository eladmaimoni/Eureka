
#include <fstream>
#include <filesystem>
#include <ostream>

namespace eureka
{
    void bin2h(const std::filesystem::path& path, const std::string& variable_name, std::ostream& output);
}