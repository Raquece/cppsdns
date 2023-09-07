#include <fstream>
#include <vector>
#include <cstdint>

namespace packetfs
{
    const std::vector<std::byte> read_block(std::uint32_t offset, std::uint32_t length, std::string const& filename);
}