#include "BPSParser.hpp"
#include "UI/UpdaterWindow.hpp"
#include "utils.hpp"
#include <boxer/boxer.h>

namespace drfr
{
	void UpdaterWindow::_downloadUninstall()
	{
		this->dataWinPathStr =
			utils::openFileDialog("Sélectionnez le data.win de DELTARUNE", {{"DELTARUNE data", "win"}});
		if (dataWinPathStr.empty())
			return;

		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();
		std::string os = utils::getOS();
		std::string filesRaw;
		utils::getToString("https://deltaruneapi.mbourand.fr/updater/" + os + "_uninstall.txt", filesRaw);

		this->filesToDownload = utils::split(filesRaw, "\n");
		this->totalDownloadSize = atoll(this->filesToDownload.back().c_str());
		this->filesToDownload.pop_back();

		this->downloadTime = std::time(nullptr);
		std::thread dlThread([this, os] {
			this->_downloadFiles(std::string("https://deltaruneapi.mbourand.fr/updater/" + os + "/"),
								 this->filesToDownload, this->downloadTime);
		});
		dlThread.detach();

		this->progressBar.setProgression(0);
		this->progressBar.setMax(totalDownloadSize);
		this->progressBar.setEnabled(true);
		this->installButton.setEnabled(false);
		this->uninstallButton.setEnabled(false);

		this->state = State::DownloadUninstall;
	}

	void UpdaterWindow::_updateDownloadUninstallProgress()
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

		auto rounded = std::to_wstring(downloaded * 100 / progressBar.getMax());
		rounded.erase(rounded.find('.') + 3, std::string::npos);
		progressBar.setDesc(std::wstring(L"Desinstallation (1/2) : ") + rounded + L"%");

		if (downloaded >= this->totalDownloadSize)
			this->state = State::DoneDownloadUninstall;
	}

	void UpdaterWindow::_applyUninstaller()
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
				file = "data.win";
				break;
			}
		}

		this->progressBar.setProgression(0);
		this->progressBar.setMax(1);
		this->progressBar.setDesc(L"Prépatation de la déinstallation...");

		this->state = State::ApplyUninstall;
	}

	void UpdaterWindow::_updateUninstallProgress()
	{
		if (this->installProgress < 0)
		{
			throw std::runtime_error(this->errorMessage);
			return;
		}
		progressBar.setProgression(this->installProgress);

		std::wstring rounded = std::to_wstring(this->installProgress * 100.0f);
		rounded.erase(rounded.find('.') + 3, std::string::npos);
		progressBar.setDesc(std::wstring(L"Désinstallation (2/2) : ") + rounded + L"%");

		if (this->installProgress >= 1)
			this->state = State::DoneUninstall;
	}

	void UpdaterWindow::_uninstallFiles()
	{
		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();

		std::error_code ec; // To avoid throws
		for (auto& entry : std::filesystem::recursive_directory_iterator(deltaruneFolder))
		{
			if (!std::filesystem::is_regular_file(entry.path()))
				continue;
			auto path = std::filesystem::path(entry.path().string());
			auto extension = path.extension().string();
			if (extension == ".original")
			{
				path.replace_extension("");
				std::filesystem::remove(path, ec);
				std::filesystem::rename(entry.path(), path);
			}
		}

		std::filesystem::remove("version.txt");

		boxer::show("Le patch a ete desinstalle avec succes !", "Patch desinstsalle");

		installButton.setEnabled(true);
		uninstallButton.setEnabled(true);
		progressBar.setEnabled(false);

		this->state = State::Idle;
	}
}
