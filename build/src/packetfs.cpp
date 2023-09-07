#include <cppsdns/packetfs.h>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>

namespace packetfs
{
    const std::vector<std::byte> read_block(std::uint32_t offset, std::uint32_t length, std::string const& filename)
    {
        std::ifstream is(filename, std::ios::binary);
        is.seekg(offset);

        std::vector<std::byte> data(length);
        is.read(reinterpret_cast<char *>(data.data()), length);

        data.resize(is.gcount());

        return data;
    }
}