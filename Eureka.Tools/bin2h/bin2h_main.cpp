#include <boost/program_options.hpp>
#include <iostream>
#include "bin2h.hpp"

namespace std
{
    template <>
    struct formatter<std::filesystem::path> : std::formatter<std::string>
    {

        //constexpr auto parse(format_parse_context& ctx)
        //{
        //    return end(ctx);
        //}
        template <typename FormatContext>
        auto format(const std::filesystem::path& v, FormatContext& ctx) const
        {
            return std::formatter<std::string>::format(v.string(), ctx);
            //auto&& out = ctx.out();
            //return std::vformat_to(out, std::make_format_args(v.string()));
        }
    };
}



int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    try
    {
        auto cwd = std::filesystem::current_path();
        std::cout << std::format("bin2h : current directory {}\n", cwd);

        po::options_description options_desc("Allowed options");
        options_desc.add_options()
            ("help", "produce help message")
            ("input", po::value<std::vector<std::string>>(), "list of input files path : bin2h --input=a.hlsl")
       ;

       po::positional_options_description positional_desc;
       positional_desc.add("input", -1); // all positional options should translate to the input parameter

       po::variables_map variable_map;
       po::store(
            po::command_line_parser(argc, argv)
            .options(options_desc)
            .positional(positional_desc).run(),
            variable_map
        );

        po::notify(variable_map);

        if (variable_map.count("help"))
        {
            std::cout << options_desc << '\n';
            return 1;
        }

        if (variable_map.count("input"))
        {

            auto filenames = variable_map["input"].as<std::vector<std::string>> ();
       
            if (filenames.size() == 1 && filenames.at(0) == "*")
            {
                // all spv files
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
    }
    catch (const std::exception& err)
    {
        std::cout << err.what();
    }
    return 0;
}