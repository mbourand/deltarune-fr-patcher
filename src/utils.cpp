#include "utils.hpp"
#include "UI/ProgressBar.hpp"
#include <algorithm>
#include <curl/curl.h>
#include <nfd.h>

namespace utils
{
	static void write_data(void* ptr, size_t size, size_t nmemb, std::string* str)
	{
		*str += std::string((char*)ptr, nmemb);
	}

	static size_t write_data_in_file(void* ptr, size_t size, size_t nmemb, std::fstream* stream)
	{
		stream->write((char*)ptr, size * nmemb);
		return size * nmemb;
	}

	int getToString(const std::string& url, std::string& payload)
	{
		CURL* curl;
		CURLcode res;

		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &payload);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

			res = curl_easy_perform(curl);
			if (res != CURLE_OK)
				return res;
			curl_easy_cleanup(curl);
			// Removing \r\n\r\n at the end
			payload.erase(payload.size() - 5, payload.size() - 1);
			return res;
		}
		return -1;
	}

	int getToFile(const std::string& url, const std::string& dest)
	{
		CURL* curl;
		CURLcode res;

		if (dest.find_last_of("/") != std::string::npos)
		{
			auto folder = std::filesystem::path(dest.substr(0, dest.find_last_of("/")));
			std::filesystem::create_directories(folder);
		}

		std::fstream fs(dest, std::fstream::out | std::fstream::binary);

		curl = curl_easy_init();
		if (!curl)
			return -1;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data_in_file);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fs);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			return res;
		curl_easy_cleanup(curl);
		fs.close();
		return res;
	}

	std::vector<std::string> split(std::string str, const std::string& delimiter)
	{
		std::vector<std::string> splits;
		size_t pos = 0;
		while ((pos = str.find(delimiter)) != std::string::npos)
		{
			if (str.substr(0, pos).size() != 0)
				splits.push_back(str.substr(0, pos));
			str.erase(0, pos + delimiter.size());
		}
		if (str.size() != 0)
			splits.push_back(str);
		return splits;
	}

	std::string getOS()
	{
#if defined(OS_WINDOWS)
		return "windows";
#elif defined(OS_LINUX)
		return "linux";
#elif defined(OS_MACOS)
		return "mac";
#endif
		throw std::runtime_error("Unknown OS");
	}

	std::string openFileDialog(const std::string& title, const std::vector<nfdfilteritem_t>& filters)
	{
		nfdchar_t* outPath = nullptr;
		nfdresult_t result = NFD_OpenDialog(&outPath, filters.data(), filters.size(), nullptr);
		if (result == NFD_CANCEL)
			return "";
		else if (result == NFD_ERROR)
			throw std::runtime_error(NFD_GetError());
		std::string ret = outPath;
		NFD_FreePath(outPath);
		return ret;
	}

	void openWebPage(const std::string& url)
	{
#if defined(OS_WINDOWS)
		system((std::string("start") + url).c_str());
#elif defined(OS_MACOS)
		system((std::string("open '") + url + "'").c_str());
#elif defined(OS_LINUX)
		system((std::string("xdg-open '") + url + "'").c_str());
#endif
	}
}
