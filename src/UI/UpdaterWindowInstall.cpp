#include "BPSParser.hpp"
#include "UI/UpdaterWindow.hpp"
#include "utils.hpp"
#include <boxer/boxer.h>

namespace drfr
{
	void UpdaterWindow::_downloadFiles(std::string urlbase, const std::vector<std::string>& files, uint64_t time)
	{
		auto temp = std::filesystem::temp_directory_path();
		std::filesystem::create_directories(temp / std::to_string(time));
		for (auto& file : files)
		{
			auto to = temp / std::to_string(time) / file;
			utils::getToFile(urlbase + file, to.string());
		}
	}

	void UpdaterWindow::_download()
	{
		this->dataWinPathStr = utils::openFileDialog({{"DELTARUNE data", "win"}});
		if (dataWinPathStr.empty())
			return;

		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();
		std::string os = utils::getOS();
		std::string filesRaw;
		utils::getToString("https://deltarune.fr/installer/" + os + "_list.txt", filesRaw);

		this->filesToDownload = utils::split(filesRaw, "\n");
		this->totalDownloadSize = atoll(this->filesToDownload.back().c_str());
		this->filesToDownload.pop_back();

		this->downloadTime = std::time(nullptr);
		std::thread dlThread([this, os] {
			this->_downloadFiles(std::string("https://deltarune.fr/installer/" + os + "/"), this->filesToDownload,
								 this->downloadTime);
		});
		dlThread.detach();

		this->progressBar.setProgression(0);
		this->progressBar.setMax(totalDownloadSize);
		this->progressBar.setEnabled(true);
		this->installButton.setEnabled(false);
		this->uninstallButton.setEnabled(false);

		this->state = State::Downloading;
	}

	void UpdaterWindow::_updateDownloadProgress()
	{
		auto temp = std::filesystem::temp_directory_path();
		uint64_t downloaded = 0;
		for (auto& file : this->filesToDownload)
		{
			std::ifstream in(temp / std::to_string(this->downloadTime) / file,
							 std::ifstream::ate | std::ifstream::binary);
			if (!in.is_open())
				continue;
			downloaded += in.tellg();
		}

		progressBar.setProgression(downloaded);
		std::string downloadedStr = std::to_string((downloaded / 10000) / 100.0);
		std::string toDownloadStr = std::to_string((totalDownloadSize / 10000) / 100.0);
		downloadedStr.erase(downloadedStr.find('.') + 3, std::string::npos);
		toDownloadStr.erase(toDownloadStr.find('.') + 3, std::string::npos);
		progressBar.setDesc(std::string("Téléchargé : ") + downloadedStr + " / " + toDownloadStr + "Mo");

		if (downloaded >= this->totalDownloadSize)
			this->state = State::DoneDownloading;
	}

	void UpdaterWindow::_applyPatch()
	{
		this->installProgress = 0;
		auto temp = std::filesystem::temp_directory_path();
		for (auto& file : this->filesToDownload)
		{
			if (std::filesystem::path(file).extension() == ".bps")
			{
				std::thread t(
					drfr::applyPatch, this->dataWinPathStr, (temp / std::to_string(this->downloadTime) / file).string(),
					(temp / std::to_string(this->downloadTime) / std::filesystem::path(this->dataWinPathStr).filename())
						.string(),
					std::ref(this->installProgress), std::ref(this->errorMessage));
				t.detach();
				file = DATA_WIN_NAME;
				break;
			}
		}

		this->progressBar.setProgression(0);
		this->progressBar.setMax(1);
		this->progressBar.setDesc("Prépatation de l'installation...");

		this->state = State::Installing;
	}

	void UpdaterWindow::_updateInstallProgressBar()
	{
		if (this->installProgress < 0)
		{
			throw std::runtime_error(this->errorMessage);
			return;
		}
		progressBar.setProgression(this->installProgress);

		std::string rounded = std::to_string(round(this->installProgress * 10000.0f) / 100.0f);
		rounded.erase(rounded.find('.') + 3, std::string::npos);
		progressBar.setDesc(std::string("Installation : ") + rounded + "%");

		if (this->installProgress >= 1)
			this->state = State::DoneInstalling;
	}

	void UpdaterWindow::_moveFiles()
	{
		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();

		for (auto& file : this->filesToDownload)
		{
			auto toPath = (deltaruneFolder / (file + ".original"));
			toPath.replace_filename("." + toPath.filename().string());
			std::filesystem::remove(toPath);
			std::filesystem::rename(deltaruneFolder / file, toPath);
		}

		auto temp = std::filesystem::temp_directory_path();
		std::error_code ec; // To avoid throws
		for (auto& file : this->filesToDownload)
		{
			std::filesystem::remove(deltaruneFolder / file, ec);
			auto fileFolder = (deltaruneFolder / file).parent_path();
			std::filesystem::create_directories(fileFolder);
			std::filesystem::rename(temp / std::to_string(this->downloadTime) / file, deltaruneFolder / file);
		}

		std::string version;
		utils::getToString("https://deltarune.fr/installer/version.txt", version);
		std::ofstream out("version.txt");
		out << version;
		out.close();

		boxer::show("Le patch \u00E0 \u00E9t\u00E9 install\u00E9 avec succ\u00E8s !", "Patch appliqu\u00E9");

		installButton.setEnabled(true);
		uninstallButton.setEnabled(true);
		progressBar.setEnabled(false);

		this->state = State::Idle;
	}
}
