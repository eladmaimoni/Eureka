#include <iostream>
#include "bin2h.hpp"

namespace std
{
    template <>
    struct formatter<std::filesystem::path> : std::formatter<std::string>
    {
        template <typename FormatContext>
        auto format(const std::filesystem::path& v, FormatContext& ctx) const
        {
            return std::formatter<std::string>::format(v.string(), ctx);
        }
    };
}



int main(int argc, char* argv[])
{
    
    if (argc <= 1)
    {
        std::cout << std::format("bin2h : not enough arguments\n");
        return 0;
    }

    /*
    
    perhaps we can remove this tool usage

    https://stackoverflow.com/questions/47359106/spir-v-shader-causing-validation-errors-at-runtime
    
    Alternativelly you can load shaders statically (via #include) as a C++ inline file. You can create such file by 
    glslc -mfmt=c or glslangValidator -V -x --vn variable_name.

    */

    try
    {
        auto cwd = std::filesystem::current_path();
        std::cout << std::format("bin2h : current directory {}\n", cwd);

        std::vector<std::filesystem::path> filenames;

        for (auto i = 1; i < argc; ++i)
        {
            filenames.emplace_back(argv[i]);
        }

        for (std::filesystem::path filename : filenames)
        {

            auto input_path = cwd / filename;

            if (!std::filesystem::exists(input_path))
            {

                std::cout << std::format("input file not found = {}\n", input_path);
                continue;
            }


            auto filename_no_extentions = filename.stem().string();

            auto variable_name = "SHADER_BYTES_" + filename_no_extentions;

            auto output_path = input_path;
            output_path.replace_extension("spvhpp");

            std::ofstream output_file(output_path, std::ios::out | std::ios::trunc);

            eureka::bin2h(input_path, variable_name, output_file);

            std::cout << std::format("bin2h : procesing {} var = {} out = {}\n", input_path, variable_name, output_path);
        }

    }
    catch (const std::exception& err)
    {
        std::cout << err.what();
    }
    return 0;
}