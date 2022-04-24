#pragma once

#include <array>
#include <fstream>

namespace drfr
{
	uint32_t crc32(std::ifstream& stream);
}
