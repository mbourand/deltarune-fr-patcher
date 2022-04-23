#include "BPSParser.hpp"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// Doc https://github.com/blakesmith/rombp/blob/master/docs/bps_spec.md

namespace drfr
{
	static uint64_t decode_number(std::ifstream& file)
	{
		uint64_t data = 0, shift = 1;

		while (true)
		{
			uint8_t x = file.get();
			data += (x & 0x7f) * shift;
			if (x & 0x80)
				break;
			shift <<= 7;
			data += shift;
		}

		return data;
	}

	Action decode_action(std::ifstream& file)
	{
		uint64_t raw = decode_number(file);
		return {static_cast<uint8_t>(raw & 0b11), (raw >> 2) + 1};
	}

	// Copies data from source to target.
	void readSource(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
					int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset)
	{
		source.seekg(writtenToTarget, std::ios::beg);
		target.seekg(writtenToTarget, std::ios::beg);
		std::vector<uint8_t> buffer(length);
		source.read(reinterpret_cast<char*>(buffer.data()), length);
		target.write(reinterpret_cast<char*>(buffer.data()), length);
		writtenToTarget += length;
	}

	// Copies data from patch to target.
	void readTarget(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
					int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset)
	{
		target.seekg(writtenToTarget, std::ios::beg);
		std::vector<uint8_t> buffer(length);
		patch.read(reinterpret_cast<char*>(buffer.data()), length);
		target.write(reinterpret_cast<char*>(buffer.data()), length);
		writtenToTarget += length;
	}

	// Copies data from source to target with an offset applied to source.
	void copySource(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
					int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset)
	{
		uint64_t data = decode_number(patch);
		sourceRelOffset += (data & 1 ? -1 : 1) * (data >> 1);
		source.seekg(sourceRelOffset, std::ios::beg);
		target.seekg(writtenToTarget, std::ios::beg);
		std::vector<uint8_t> buffer(length);
		source.read(reinterpret_cast<char*>(buffer.data()), length);
		target.write(reinterpret_cast<char*>(buffer.data()), length);
		writtenToTarget += length;
		sourceRelOffset += length;
	}

	// Copies data from target to target with an offset applied to target.
	// Used for optimisation of repeated data
	// Not really optimizable, because we dont assume that every byte copied is already written in target.
	void copyTarget(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
					int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset)
	{
		uint64_t data = decode_number(patch);
		targetRelOffset += (data & 1 ? -1 : 1) * (data >> 1);
		while (length-- > 0)
		{
			target.seekg(targetRelOffset, std::ios::beg);
			uint8_t value = target.get();
			target.seekg(writtenToTarget, std::ios::beg);
			target.put(value);
			writtenToTarget++;
			targetRelOffset++;
		}
	}

	std::string applyPatch(const nfdchar_t* sourcePath, const char* patchPath)
	{
		static constexpr std::array<void (*)(std::ifstream&, std::fstream&, std::ifstream&, uint64_t, int&, int&, int&),
									4>
			actionFuncs = {readSource, readTarget, copySource, copyTarget};

		std::ifstream source(sourcePath, std::ios::binary);
		std::ifstream patch(patchPath, std::ios::binary);

		std::string targetPath = std::string(sourcePath) + ".tmp";
		{
			std::ofstream target(targetPath);
		}

		std::fstream target(targetPath, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);

		if (!source.is_open() || !patch.is_open())
			throw std::runtime_error("Failed to open file");

		uint64_t end = patch.seekg(0, std::ios::end).tellg();
		auto actionEnd = patch.seekg(end - FOOTER_SIZE, std::ios::beg).tellg();
		patch.seekg(0, std::ios::beg);

		for (int i = 0; i < BPS_MAGIC_LENGTH; i++)
			if (patch.get() != BPS_MAGIC[i])
				throw std::runtime_error("Invalid BSP file");

		uint64_t sourceLength = decode_number(patch);
		uint64_t targetLength = decode_number(patch);
		uint64_t metadataSize = decode_number(patch);
		std::string metadata(metadataSize, '\0');
		patch.read(metadata.data(), metadataSize);

		int writtenToTarget = 0;
		int sourceRelOffsest = 0;
		int targetRelOffsest = 0;

		int printProgressrequency = 10000;
		int current = 0;

		auto startOffset = patch.tellg();

		while (source.good() && patch.good() && target.good() && patch.tellg() < actionEnd)
		{
			Action action = decode_action(patch);
			actionFuncs[action.type](source, target, patch, action.length, writtenToTarget, sourceRelOffsest,
									 targetRelOffsest);
			if (patch.tellg() / printProgressrequency > current)
			{
				std::cout << "Progress: " << std::setprecision(2) << std::fixed
						  << ((patch.tellg() - startOffset) * 100) / static_cast<float>(actionEnd - startOffset) << "%"
						  << std::endl;
				current++;
			}
		}

		if (!source.good())
			throw std::runtime_error("Failed to read source file");
		if (!patch.good())
			throw std::runtime_error("Failed to read patch file");
		if (!target.good())
			throw std::runtime_error("Failed to write target file");
		return targetPath;
	}

}
