#include "UI/UpdaterWindow.hpp"
#include "BPSParser.hpp"
#include "utils.hpp"
#include <nfd.h>

namespace drfr
{
	sf::View UpdaterWindow::getLetterboxView()
	{
		sf::View view = window.getView();
		int windowWidth = window.getSize().x;
		int windowHeight = window.getSize().y;
		float windowRatio = windowWidth / (float)windowHeight;
		float viewRatio = view.getSize().x / (float)view.getSize().y;
		float sizeX = 1;
		float sizeY = 1;
		float posX = 0;
		float posY = 0;

		bool horizontalSpacing = true;
		if (windowRatio < viewRatio)
			horizontalSpacing = false;

		if (horizontalSpacing)
		{
			sizeX = viewRatio / windowRatio;
			posX = (1 - sizeX) / 2.f;
		}

		else
		{
			sizeY = windowRatio / viewRatio;
			posY = (1 - sizeY) / 2.f;
		}

		view.setViewport(sf::FloatRect(sf::Vector2f(posX, posY), sf::Vector2f(sizeX, sizeY)));

		return view;
	}

	void UpdaterWindow::init()
	{
		window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "DELTARUNE [Patch FR]");
		previousResolution = window.getView().getSize();

		if (!icon.loadFromFile("assets/icon.png"))
			throw std::runtime_error("Failed to load icon");
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

		if (!this->font.loadFromFile("assets/determination.ttf"))
			throw std::runtime_error("Failed to load font");
		utils::getToString("https://deltaruneapi.mbourand.fr/updater/version.txt", this->latestVersion);

		if (!backgroundImg.loadFromFile("assets/bg.png"))
			throw std::runtime_error("Failed to load background image");
		background = sf::Sprite(backgroundImg);

		if (!logoImg.loadFromFile("assets/logo.png"))
			throw std::runtime_error("Failed to load logo image");
		logo = sf::Sprite(logoImg);

		sf::Vector2f installPos(window.getSize().x / 2 - window.getSize().x * 0.25, window.getSize().y * 0.45);
		sf::Vector2f installSize(window.getSize().x * 0.5, window.getSize().y * 0.13);
		installButton = Button(installPos, installSize, isInstalled() ? L"Mettre à jour" : L"Installer", 50);
		if (isUpToDate())
		{
			installButton.setText(L"Jeu déjà à jour");
			installButton.setEnabled(false);
		}

		sf::Vector2f uninstallPos(window.getSize().x / 2 - window.getSize().x * 0.115, window.getSize().y * 0.795);
		sf::Vector2f uninstallSize(window.getSize().x * 0.23, window.getSize().y * 0.07);
		uninstallButton = Button(uninstallPos, uninstallSize, L"Désinstaller", 30);
		if (!isInstalled())
			uninstallButton.setEnabled(false);

		sf::Vector2f creditsPos(window.getSize().x / 2 - window.getSize().x * 0.15, window.getSize().y * 0.91);
		sf::Vector2f creditsSize(window.getSize().x * 0.3, window.getSize().y * 0.07);
		creditsButton = Button(creditsPos, creditsSize, L"Crédits", 38);

		sf::Vector2f progressPos(window.getSize().x / 2 - window.getSize().x * 0.225, window.getSize().y * 0.595);
		sf::Vector2f progressSize(window.getSize().x * 0.45, window.getSize().y * 0.02);
		progressBar = ProgressBar(L"", 0, 100);
		progressBar.setPosition(progressPos);
		progressBar.setSize(progressSize);
		progressBar.setEnabled(false);
	}

	void UpdaterWindow::handleEvents()
	{
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;

				case sf::Event::Resized:
					window.setSize(sf::Vector2u(std::max(event.size.width, 640u), std::max(event.size.height, 480u)));

					float ratio = WINDOW_WIDTH / static_cast<float>(WINDOW_HEIGHT);

					unsigned int newWidth =
						window.getSize().x < window.getSize().y ? window.getSize().x : window.getSize().y * ratio;
					unsigned int newHeight =
						window.getSize().x < window.getSize().y ? window.getSize().x / ratio : window.getSize().y;

					sf::View view = getLetterboxView();
					window.setView(view);

					sf::Vector2f installPos(window.getSize().x / 2 - window.getSize().x * 0.25,
											window.getSize().y * 0.45);
					sf::Vector2f installSize(window.getSize().x * 0.5, window.getSize().y * 0.13);

					sf::Vector2f uninstallPos(window.getSize().x / 2 - window.getSize().x * 0.115,
											  window.getSize().y * 0.795);
					sf::Vector2f uninstallSize(window.getSize().x * 0.23, window.getSize().y * 0.07);

					sf::Vector2f creditsPos(window.getSize().x / 2 - window.getSize().x * 0.15,
											window.getSize().y * 0.91);
					sf::Vector2f creditsSize(window.getSize().x * 0.3, window.getSize().y * 0.07);

					installButton.setRect(installPos, installSize);
					installButton.setFontSize(installButton.getFontSize() * (window.getView().getSize().x /
																			 static_cast<float>(previousResolution.x)));
					uninstallButton.setRect(uninstallPos, uninstallSize);
					uninstallButton.setFontSize(
						uninstallButton.getFontSize() *
						(window.getView().getSize().x / static_cast<float>(previousResolution.x)));
					creditsButton.setRect(creditsPos, creditsSize);
					creditsButton.setFontSize(creditsButton.getFontSize() * (window.getView().getSize().x /
																			 static_cast<float>(previousResolution.x)));

					progressBar.setPosition(sf::Vector2f(window.getView().getSize().x / 2 - window.getSize().x * 0.225,
														 window.getView().getSize().y * 0.595));
					progressBar.setSize(
						sf::Vector2f(window.getView().getSize().x * 0.45, window.getView().getSize().y * 0.02));

					previousResolution = window.getView().getSize();
					break;
			}
		}
	}

	void UpdaterWindow::doTick()
	{
		bool mousePreessed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
		installButton.update(window, mousePreessed);
		uninstallButton.update(window, mousePreessed);
		creditsButton.update(window, mousePreessed);

		float bgScale = window.getView().getSize().y / static_cast<float>(backgroundImg.getSize().y);
		background.setScale(sf::Vector2f(bgScale, bgScale));
		background.setPosition(sf::Vector2f(window.getView().getSize().x / 2, window.getView().getSize().y / 2));
		background.setOrigin(
			sf::Vector2f(background.getLocalBounds().width / 2, background.getLocalBounds().height / 2));

		float logoScale = (window.getView().getSize().x * (2 / 3.0f)) / static_cast<float>(logoImg.getSize().x);
		logo.setScale(sf::Vector2f(logoScale, logoScale));
		logo.setPosition(sf::Vector2f(window.getView().getSize().x / 2, window.getView().getSize().y * 0.15));
		logo.setOrigin(sf::Vector2f(logo.getLocalBounds().width / 2, logo.getLocalBounds().height / 2));

		sf::Vector2f installPos = sf::Vector2f(window.getView().getSize().x / 2 - window.getView().getSize().x * 0.25,
											   window.getView().getSize().y * 0.45);
		sf::Vector2f installSize =
			sf::Vector2f(window.getView().getSize().x * 0.5, window.getView().getSize().y * 0.13);
		sf::Vector2f uninstallPos =
			sf::Vector2f(window.getView().getSize().x / 2 - window.getView().getSize().x * 0.115,
						 window.getView().getSize().y * 0.795);
		sf::Vector2f uninstallSize =
			sf::Vector2f(window.getView().getSize().x * 0.23, window.getView().getSize().y * 0.07);
		sf::Vector2f creditsPos = sf::Vector2f(window.getView().getSize().x / 2 - window.getView().getSize().x * 0.15,
											   window.getView().getSize().y * 0.91);
		sf::Vector2f creditsSize =
			sf::Vector2f(window.getView().getSize().x * 0.3, window.getView().getSize().y * 0.07);

		installButton.setRect(installPos, installSize);
		installButton.setFontSize(installButton.getFontSize() *
								  (window.getView().getSize().x / static_cast<float>(previousResolution.x)));
		uninstallButton.setRect(uninstallPos, uninstallSize);
		uninstallButton.setFontSize(uninstallButton.getFontSize() *
									(window.getView().getSize().x / static_cast<float>(previousResolution.x)));
		creditsButton.setRect(creditsPos, creditsSize);
		creditsButton.setFontSize(creditsButton.getFontSize() *
								  (window.getView().getSize().x / static_cast<float>(previousResolution.x)));

		progressBar.setPosition(sf::Vector2f(window.getView().getSize().x / 2 - window.getView().getSize().x * 0.225,
											 window.getView().getSize().y * 0.595));
		progressBar.setSize(sf::Vector2f(window.getView().getSize().x * 0.45, window.getView().getSize().y * 0.02));

		if (installButton.isPressed() && !isUpToDate())
			this->_download();
		if (creditsButton.isPressed())
			utils::openWebPage("https://deltarune.fr/credits");

		if (this->state == State::Downloading)
			this->_updateDownloadProgress();
		if (this->state == State::DoneDownloading)
			this->_applyPatch();
		if (this->state == State::Installing)
			this->_updateInstallProgressBar();
		if (this->state == State::DoneInstalling)
			this->_moveFiles();
	}

	void UpdaterWindow::render()
	{
		window.clear();
		window.draw(background);
		window.draw(logo);
		installButton.draw(window);
		progressBar.draw(window);
		uninstallButton.draw(window);
		creditsButton.draw(window);

		std::string currentVersion = getVersion();

		std::string fullText = std::string("Dernière Version : v") + latestVersion +
							   "\nVersion Installée : " + (currentVersion.size() ? "v" + currentVersion : "Aucune");
		sf::Text versionText(sf::String::fromUtf8(fullText.begin(), fullText.end()), font, 17);

		versionText.setPosition(sf::Vector2f(10, window.getView().getSize().y - 45));
		versionText.setFillColor(sf::Color::White);

		window.draw(versionText);
		window.display();
	}

	bool UpdaterWindow::isOpen() const { return window.isOpen(); }

	bool UpdaterWindow::isInstalled() const
	{
		std::ifstream in("version.txt");
		if (!in.is_open())
			return false;
		return true;
	}

	std::string UpdaterWindow::getVersion() const
	{
		std::ifstream in("version.txt");
		if (!in.is_open())
			return "";
		std::string version;
		in >> version;
		return version;
	}

	bool UpdaterWindow::isUpToDate() const
	{
		static std::string latest = "";

		if (latest.size() == 0)
		{
			std::string versionRaw;
			utils::getToString("https://deltaruneapi.mbourand.fr/updater/version.txt", versionRaw);
			latest = versionRaw;
		}

		std::ifstream in("version.txt");
		if (!in.is_open())
			return false;
		std::string version;
		in >> version;

		return version == latest;
	}
}
