#include "BPSParser.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nfd.hpp>

int main()
{
	NFD_Init();

	std::ifstream conf("files.conf", std::ios::binary);
	if (!conf.is_open())
	{
		std::cout << "Error: Could not open files.conf" << std::endl;
		return 1;
	}

	nfdchar_t* outPath = nullptr;
	std::array<nfdfilteritem_t, 1> filters = {{{"DELTARUNE Data", "win"}}};
	nfdresult_t result = NFD_OpenDialog(&outPath, filters.data(), filters.size(), nullptr);
	if (result == NFD_CANCEL)
		return 0;
	else if (result == NFD_ERROR)
	{
		std::cout << "Error: " << NFD_GetError() << std::endl;
		return 1;
	}

	try
	{
		std::string patchedFile = drfr::applyPatch(outPath, "data.bps");
		std::filesystem::rename(outPath, std::string(outPath) + ".original");
		std::filesystem::rename(patchedFile, outPath);

		std::vector<std::string> config;
		std::string line;
		while (std::getline(conf, line))
			config.push_back(line);
		auto basePath = std::filesystem::path(outPath).parent_path();

		// Used to avoid throw if the original file doesn't exist.
		std::error_code ec;
		for (const auto& item : config)
		{
			if (std::filesystem::is_directory(item))
			{
				for (auto& file : std::filesystem::directory_iterator(item))
				{
					std::filesystem::rename(basePath / item / file.path().filename(),
											(basePath / item / file.path().filename()).string() + ".original", ec);
					std::filesystem::copy(file.path(), basePath / item / file.path().filename());
				}
			}
			else if (std::filesystem::is_regular_file(item))
			{
				std::filesystem::rename(basePath / item, (basePath / item).string() + ".original", ec);
				std::filesystem::copy(item, basePath / item);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}

	NFD_Quit();
}
