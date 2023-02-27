#include "bin2h.hpp"
#include <format>
//#include <iostream>

namespace eureka
{

    void bin2h(const std::filesystem::path& path, const std::string& variable_name, std::ostream& output)
    {
        static constexpr long long CHUNK_SIZE = 30;
        std::ifstream file(path, std::ifstream::binary | std::ios::in | std::ios::ate);
        file.seekg(0, std::ifstream::end);
        auto bytes = file.tellg();
        file.seekg(0, std::ifstream::beg);

        if (bytes <= 0)
        {
            throw std::invalid_argument("empty file");
        }
        //std::cout << std::format("bin2h: compiling shader {} total bytes = {}\n", path.string(), static_cast<std::size_t>(bytes));

        output << std::format("const uint8_t {}[] = \n{{ \n", variable_name);


        // first byte:
        auto val = file.get();
        output << " 0x" << std::hex << val;
        bytes -= 1;

        auto chunks = bytes / CHUNK_SIZE;
        auto remainder = bytes % CHUNK_SIZE;



        for (auto c = 0ll; c < chunks; ++c)
        {
            for (auto i = 0ll; i < CHUNK_SIZE; ++i)
            {
                val = file.get();

                output << ",0x" << std::hex << val;
            }
            output << '\n';
        }

        for (auto i = 0ll; i < remainder; ++i)
        {
            val = file.get();

            output << ",0x" << std::hex << val;
        }

        output << "\n};\n";
    }

}