#include "CRC32.hpp"
#include <iomanip>
#include <iostream>

namespace drfr
{
	static uint32_t crc32Byte(uint32_t r)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (r & 1)
				r = 0xedb88320 ^ (r >> 1);
			else
				r >>= 1;
		}

		return r;
	}

	uint32_t crc32(std::ifstream& stream)
	{
		static std::array<uint32_t, 256> table;
		static bool init = true;
		if (init)
		{
			for (size_t i = 0; i < 256; i++)
				table[i] = crc32Byte(i);
			init = false;
		}

		stream.seekg(0, std::ios::beg);
		uint32_t crc = 0xFFFFFFFF;
		std::array<uint8_t, 4096> buffer;

		while (!stream.eof())
		{
			stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			for (size_t i = 0; i < stream.gcount(); i++)
				crc = table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
		}

		stream.clear();
		stream.seekg(0, std::ios::beg);

		return crc ^ 0xFFFFFFFF;
	}
}
