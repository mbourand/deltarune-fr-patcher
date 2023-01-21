#pragma once

#include "Button.hpp"
#include "ProgressBar.hpp"
#include "globals.hpp"
#include <SFML/Graphics.hpp>

namespace drfr
{
	class UpdaterWindow
	{
	public:
		enum State
		{
			Idle,
			Downloading,
			DoneDownloading,
			Installing,
			DoneInstalling,
			Paused
		};

		sf::RenderWindow window;
		sf::Event event;
		sf::Texture backgroundImg, logoImg;
		sf::Sprite background, logo;
		Button installButton, uninstallButton, creditsButton, tutorialButton;
		ProgressBar progressBar;
		sf::Vector2f previousResolution;
		sf::Clock cursor_update_clock;
		sf::Font font;
		sf::Image icon;
		std::string latestVersion;
		sf::Text tutorialText;
		bool itch;

		State state;
		uint64_t totalDownloadSize;
		std::vector<std::string> filesToDownload;
		uint64_t downloadTime;
		std::string dataWinPathStr;
		float installProgress;
		std::string errorMessage;

		bool focused;

		void init();

		void handleEvents();
		sf::View getLetterboxView();
		void doTick();
		void render();
		bool isOpen() const;

		bool isInstalled() const;
		std::string getVersion() const;
		bool isUpToDate() const;

	private:
		void _scaleElements();
		void _download();
		void _updateDownloadProgress();
		void _applyPatch();
		void _updateInstallProgressBar();
		void _moveFiles();
		void _uninstallFiles();
		void _downloadFiles(std::string urlbase, const std::vector<std::string>& files, uint64_t time,
							const std::string& platform);
	};
}
