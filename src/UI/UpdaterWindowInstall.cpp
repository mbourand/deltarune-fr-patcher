#include "BPSParser.hpp"
#include "UI/UpdaterWindow.hpp"
#include "utils.hpp"

namespace drfr
{
	static void downloadFiles(std::string urlbase, const std::vector<std::string>& files, uint64_t time)
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
		this->dataWinPathStr =
			utils::openFileDialog("Sélectionnez le data.win de DELTARUNE", {{"DELTARUNE data", "win"}});
		if (dataWinPathStr.empty())
			return;

		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();
		std::string os = utils::getOS();
		std::string filesRaw;
		utils::getToString("https://deltaruneapi.mbourand.fr/updater/" + os + "_list.txt", filesRaw);

		this->filesToDownload = utils::split(filesRaw, "\n");
		this->totalDownloadSize = atoll(this->filesToDownload.back().c_str());
		this->filesToDownload.pop_back();

		this->downloadTime = std::time(nullptr);
		std::thread dlThread(downloadFiles, std::string("https://deltaruneapi.mbourand.fr/updater/" + os + "/"),
							 std::ref(this->filesToDownload), this->downloadTime);
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
		std::wstring downloadedStr = std::to_wstring((downloaded / 10000) / 100.0);
		std::wstring toDownloadStr = std::to_wstring((totalDownloadSize / 10000) / 100.0);
		downloadedStr.erase(downloadedStr.find('.') + 3, std::string::npos);
		toDownloadStr.erase(toDownloadStr.find('.') + 3, std::string::npos);
		progressBar.setDesc(std::wstring(L"Téléchargé : ") + downloadedStr + L" / " + toDownloadStr + L"Mo");

		std::cout << downloaded << " / " << totalDownloadSize << std::endl;
		if (downloaded >= this->totalDownloadSize)
			this->state = State::DoneDownloading;
	}

	void UpdaterWindow::_applyPatch()
	{
		auto temp = std::filesystem::temp_directory_path();
		for (auto& file : this->filesToDownload)
		{
			if (std::filesystem::path(file).extension() == ".bps")
			{
				std::string targetPath =
					(temp / std::to_string(this->downloadTime) / std::filesystem::path(this->dataWinPathStr).filename())
						.string();
				std::thread t(drfr::applyPatch, this->dataWinPathStr,
							  (temp / std::to_string(this->downloadTime) / file).string(), targetPath,
							  std::ref(this->installProgress));
				t.detach();
				file = "data.win";
				break;
			}
		}

		this->installProgress = 0;

		this->progressBar.setProgression(0);
		this->progressBar.setMax(1);
		this->progressBar.setDesc(L"Prépatation de l'installation...");

		this->state = State::Installing;
	}

	void UpdaterWindow::_updateInstallProgressBar()
	{
		progressBar.setProgression(this->installProgress);

		std::wstring rounded = std::to_wstring(round(this->installProgress * 10000.0f) / 100.0f);
		rounded.erase(rounded.find('.') + 3, std::string::npos);
		progressBar.setDesc(std::wstring(L"Installation : ") + rounded + L"%");

		if (this->installProgress >= 1)
			this->state = State::DoneInstalling;
	}

	void UpdaterWindow::_moveFiles()
	{
		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();

		std::error_code ec; // To avoid throws
		if (!isInstalled())
			for (auto& file : this->filesToDownload)
				std::filesystem::rename(deltaruneFolder / file, deltaruneFolder / (file + ".original"), ec);

		auto temp = std::filesystem::temp_directory_path();
		for (auto& file : this->filesToDownload)
		{
			std::filesystem::remove(deltaruneFolder / file, ec);
			auto fileFolder = (deltaruneFolder / file).parent_path();
			std::filesystem::create_directories(fileFolder);
			std::filesystem::rename(temp / std::to_string(this->downloadTime) / file, deltaruneFolder / file);
		}

		std::string version;
		utils::getToString("https://deltaruneapi.mbourand.fr/updater/version.txt", version);
		std::ofstream out("version.txt");
		out << version;
		out.close();

		installButton.setText(L"Jeu déjà à jour");
		installButton.setEnabled(false);
		uninstallButton.setEnabled(true);
		progressBar.setEnabled(false);

		this->state = State::Idle;
	}
}
