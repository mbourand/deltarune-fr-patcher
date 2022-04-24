#pragma once

#include "globals.hpp"
#include <SFML/Graphics.hpp>
#include <nfd.h>
#include <vector>

namespace utils
{
	int getToString(const std::string& url, std::string& payload);
	int getToFile(const std::string& url, const std::string& dest);
	std::vector<std::string> split(std::string str, const std::string& delimiter);
	std::string getOS();
	std::string openFileDialog(const std::string& title, const std::vector<nfdfilteritem_t>& filters);
}
