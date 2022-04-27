#include "BPSParser.hpp"
#include "UI/UpdaterWindow.hpp"
#include "utils.hpp"
#include <boxer/boxer.h>

namespace drfr
{
	void UpdaterWindow::_uninstallFiles()
	{
		this->dataWinPathStr = utils::openFileDialog({{"DELTARUNE data", "win"}});
		if (dataWinPathStr.empty())
			return;
		auto deltaruneFolder = std::filesystem::path(this->dataWinPathStr).parent_path();

		std::error_code ec; // To avoid throws
		int count = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(deltaruneFolder))
		{
			if (!std::filesystem::is_regular_file(entry.path()))
				continue;
			auto path = std::filesystem::path(entry.path().string());
			auto extension = path.extension().string();
			if (extension == ".original")
			{
				path.replace_extension("");
				path.replace_filename(path.filename().string().substr(1, std::string::npos));
				std::filesystem::remove(path, ec);
				std::filesystem::rename(entry.path(), path);
				count++;
			}
		}

		std::filesystem::remove("version.txt");

		if (count > 0)
			boxer::show("Le patch a ete desinstalle avec succes !", "Patch desinstsalle");
		else
			throw std::runtime_error("Fichiers originaux introuvables");
	}
}
