#pragma once

#include <nfd.hpp>

namespace drfr
{
	constexpr size_t FOOTER_SIZE = 12;
	constexpr char BPS_MAGIC[5] = "BPS1";
	constexpr size_t BPS_MAGIC_LENGTH = 4;

	struct Action
	{
		enum Type : uint8_t
		{
			ReadSource,
			ReadTarget,
			CopySource,
			CopyTarget
		};
		uint8_t type;
		uint64_t length;
	};

	void applyPatch(const nfdchar_t* file, const char* patch);
}
