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
		};

		sf::RenderWindow window;
		sf::Event event;
		sf::Texture backgroundImg, logoImg;
		sf::Sprite background, logo;
		Button installButton, uninstallButton, creditsButton;
		ProgressBar progressBar;
		sf::Vector2f previousResolution;
		bool handCursor;
		sf::Clock cursor_update_clock;
		sf::Font font;
		sf::Image icon;
		std::string latestVersion;

		State state;
		uint64_t totalDownloadSize;
		std::vector<std::string> filesToDownload;
		uint64_t downloadTime;
		std::string dataWinPathStr;
		float installProgress;
		std::string errorMessage;

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
		void _download();
		void _updateDownloadProgress();
		void _applyPatch();
		void _updateInstallProgressBar();
		void _moveFiles();
	};
}
