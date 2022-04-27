#include "BPSParser.hpp"
#include "CRC32.hpp"
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

	static Action decode_action(std::ifstream& file)
	{
		uint64_t raw = decode_number(file);
		return {static_cast<uint8_t>(raw & 0b11), (raw >> 2) + 1};
	}

	// Copies data from source to target.
	static void readSource(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
						   int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset, bool dry = false)
	{
		if (dry)
			return;
		source.seekg(writtenToTarget, std::ios::beg);
		target.seekg(writtenToTarget, std::ios::beg);
		std::vector<uint8_t> buffer(length);
		source.read(reinterpret_cast<char*>(buffer.data()), length);
		target.write(reinterpret_cast<char*>(buffer.data()), length);
		writtenToTarget += length;
	}

	// Copies data from patch to target.
	static void readTarget(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
						   int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset, bool dry = false)
	{
		std::vector<uint8_t> buffer(length);
		patch.read(reinterpret_cast<char*>(buffer.data()), length);
		if (dry)
			return;
		target.seekg(writtenToTarget, std::ios::beg);
		target.write(reinterpret_cast<char*>(buffer.data()), length);
		writtenToTarget += length;
	}

	// Copies data from source to target with an offset applied to source.
	static void copySource(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
						   int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset, bool dry = false)
	{
		uint64_t data = decode_number(patch);
		if (dry)
			return;
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
	static void copyTarget(std::ifstream& source, std::fstream& target, std::ifstream& patch, uint64_t length,
						   int& writtenToTarget, int& sourceRelOffset, int& targetRelOffset, bool dry = false)
	{
		uint64_t data = decode_number(patch);
		if (dry)
			return;
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

	static void _applyPatch(std::string sourcePath, std::string patchPath, std::string targetPath, float& progress)
	{
		static constexpr std::array<
			void (*)(std::ifstream&, std::fstream&, std::ifstream&, uint64_t, int&, int&, int&, bool), 4>
			actionFuncs = {readSource, readTarget, copySource, copyTarget};

		std::ifstream source(sourcePath, std::ios::binary);
		std::ifstream patch(patchPath, std::ios::binary);

		if (!source.is_open() || !patch.is_open())
			throw std::runtime_error("Impossible d'ouvrir le fichier source patch.");

		uint64_t end = patch.seekg(0, std::ios::end).tellg();
		auto actionEnd = patch.seekg(end - FOOTER_SIZE, std::ios::beg).tellg();

		std::array<uint32_t, 3> crcs;
		patch.read(reinterpret_cast<char*>(crcs.data()), FOOTER_SIZE / crcs.size());

		patch.seekg(0, std::ios::beg);

		if (crcs[0] != crc32(source))
			throw std::runtime_error("Fichier source invalide: les CRC32 ne correspondent pas");

		{
			std::ofstream target(targetPath);
		}

		std::fstream target(targetPath, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
		if (!target.is_open())
			throw std::runtime_error("Impossible d'ouvrir le fichier cible");

		for (int i = 0; i < BPS_MAGIC_LENGTH; i++)
			if (patch.get() != BPS_MAGIC[i])
				throw std::runtime_error("Fichier BSP invalide");

		uint64_t sourceLength = decode_number(patch);
		uint64_t targetLength = decode_number(patch);
		uint64_t metadataSize = decode_number(patch);
		std::string metadata(metadataSize, '\0');
		patch.read(metadata.data(), metadataSize);

		int writtenToTarget = 0;
		int sourceRelOffsest = 0;
		int targetRelOffsest = 0;

		auto startOffset = patch.tellg();

		uint64_t totalActionsLength = 0;
		uint64_t totalActions = 0;
		while (source.good() && patch.good() && target.good() && patch.tellg() < actionEnd)
		{
			Action action = decode_action(patch);
			actionFuncs[action.type](source, target, patch, action.length, writtenToTarget, sourceRelOffsest,
									 targetRelOffsest, true);
			totalActionsLength += action.length;
			totalActions++;
		}

		float progressStep = totalActionsLength / static_cast<float>(totalActions);

		patch.seekg(startOffset, std::ios::beg);

		int count = 0;
		while (source.good() && patch.good() && target.good() && patch.tellg() < actionEnd)
		{
			Action action = decode_action(patch);
			actionFuncs[action.type](source, target, patch, action.length, writtenToTarget, sourceRelOffsest,
									 targetRelOffsest, false);
			count += progressStep;
			progress = count / static_cast<float>(totalActionsLength);
		}

		if (!source.good())
			throw std::runtime_error("La lecture du fichier source a échoué");
		if (!patch.good())
			throw std::runtime_error("La lecture du fichier patch a échoué");
		if (!target.good())
			throw std::runtime_error("La lecture du fichier cible a échoué");

		progress = 1;
	}

	void applyPatch(std::string sourcePath, std::string patchPath, std::string targetPath, float& progress,
					std::string& errorMessage)
	{
		try
		{
			_applyPatch(sourcePath, patchPath, targetPath, progress);
		}
		catch (std::exception& e)
		{
			progress = -1;
			errorMessage = e.what();
		}
	}

}
