#include "UI/UpdaterWindow.hpp"
#include "BPSParser.hpp"
#include "utils.hpp"
#include <boxer/boxer.h>
#include <nfd.h>

namespace drfr
{
	sf::View UpdaterWindow::getLetterboxView()
	{
		sf::View view = window.getView();
		float windowRatio = window.getSize().x / static_cast<float>(window.getSize().y);
		float viewRatio = view.getSize().x / static_cast<float>(view.getSize().y);
		float sizeX = 1, sizeY = 1, posX = 0, posY = 0;

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
			throw std::runtime_error("L'icône n'a pas pu être chargée");
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

		if (!this->font.loadFromFile("assets/determination.ttf"))
			throw std::runtime_error("La police n'a pas pu être chargée");

		utils::getToString("https://deltarune.fr/installer/version.txt", this->latestVersion);

		if (!backgroundImg.loadFromFile("assets/bg.png"))
			throw std::runtime_error("L'arrière-plan n'a pas pu être chargé");
		background = sf::Sprite(backgroundImg);

		if (!logoImg.loadFromFile("assets/logo.png"))
			throw std::runtime_error("Le logo n'a pas pu être chargé");
		logo = sf::Sprite(logoImg);

		installButton = Button(sf::Vector2f(0, 0), sf::Vector2f(0, 0), L"Installer", 50);
		uninstallButton = Button(sf::Vector2f(0, 0), sf::Vector2f(0, 0), L"Désinstaller", 30);
		creditsButton = Button(sf::Vector2f(0, 0), sf::Vector2f(0, 0), L"Crédits", 38);
		tutorialButton = Button(sf::Vector2f(0, 0), sf::Vector2f(0, 0), L"Aide", 26);
		progressBar = ProgressBar(L"", 0, 100);
		progressBar.setEnabled(false);

		std::string text =
			std::string("Si vous voulez installer le patch, vérifiez que votre jeu\nest à jour dans sa version "
						"originale.\n\n1. Cliquez sur installer/désinstaller.\n2. Choisissez le fichier \"") +
			DATA_WIN_NAME +
			"\" dans votre dossier deltarune.\n3. Attendez la fin des barres de progression.\n\nSi vous rencontrez "
			"des difficultés, référez-vous au pdf d'aide\nou rendez-vous sur notre discord.";
		this->tutorialText = sf::Text(sf::String::fromUtf8(text.begin(), text.end()), font, 17.5);
		this->tutorialText.setFillColor(sf::Color::White);

		sf::Vector2f textPos(window.getSize().x / 2 - window.getSize().x * 0.3, window.getSize().y * 0.28);
		this->tutorialText.setPosition(textPos);

		this->focused = true;
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
					window.setView(this->getLetterboxView());
					this->_scaleElements();
					break;

				case sf::Event::LostFocus:
					this->focused = false;
					break;

				case sf::Event::GainedFocus:
					this->focused = true;
					break;
			}
		}
	}

	void UpdaterWindow::doTick()
	{
		if (focused)
		{
			bool mousePreessed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
			installButton.update(window, mousePreessed);
			uninstallButton.update(window, mousePreessed);
			creditsButton.update(window, mousePreessed);
			tutorialButton.update(window, mousePreessed);
		}

		this->_scaleElements();

		try
		{
			if (creditsButton.isPressed() && this->focused)
			{
				std::string url;
				utils::getToString("https://deltarune.fr/installer/credits_url.txt", url);
				utils::openWebPage(url);
			}
			if (tutorialButton.isPressed() && this->focused)
			{
				std::string url;
				utils::getToString("https://deltarune.fr/installer/help_url.txt", url);
				utils::openWebPage(url);
			}

			if (installButton.isPressed() && this->focused)
				this->_download();
			if (this->state == State::Downloading)
				this->_updateDownloadProgress();
			if (this->state == State::DoneDownloading)
				this->_applyPatch();
			if (this->state == State::Installing)
				this->_updateInstallProgressBar();
			if (this->state == State::DoneInstalling)
				this->_moveFiles();
		}
		catch (std::exception& e)
		{
			this->state = State::Idle;
			boxer::show((std::string("Une erreur est survenue: ") + e.what() +
						 "\n\nV\u00E9rifiez que le jeu est \u00E0 jour, r\u00E9parez les fichiers, puis "
						 "r\u00E9essayez.\nSi vous "
						 "avez besoin "
						 "d'aide, rendez-vous sur notre discord.")
							.c_str(),
						(std::string("Une erreur est survenue: ") + e.what()).c_str(), boxer::Style::Error);
			this->progressBar.setEnabled(false);
			this->installButton.setEnabled(true);
			this->uninstallButton.setEnabled(true);
		}

		try
		{
			if (uninstallButton.isPressed() && this->focused)
				this->_uninstallFiles();
		}
		catch (std::exception& e)
		{
			this->state = State::Idle;
			boxer::show((std::string("Une erreur est survenue: ") + e.what() +
						 "\n\nLe patch n'a pas pu \u00EAtre d\u00E9sinstall\u00E9. Pour "
						 "d\u00E9sinstaller le patch, mettez votre jeu \u00E0 jour.\nSi vous avez besoin "
						 "d'aide, rendez-vous sur notre discord.")
							.c_str(),
						(std::string("Une erreur est survenue: ") + e.what()).c_str(), boxer::Style::Error);
			this->progressBar.setEnabled(false);
			this->installButton.setEnabled(true);
			this->uninstallButton.setEnabled(true);
		}
	}

	void UpdaterWindow::render()
	{
		this->window.clear();
		this->window.draw(this->background);
		this->window.draw(this->logo);
		this->installButton.draw(this->window);
		this->progressBar.draw(this->window);
		this->uninstallButton.draw(this->window);
		this->creditsButton.draw(this->window);
		this->tutorialButton.draw(this->window);

		window.draw(this->tutorialText);

		std::string currentVersion = getVersion();

		std::string fullText = std::string("Dernière Version : v") + latestVersion +
							   "\nVersion Installée : " + (currentVersion.size() ? "v" + currentVersion : "Aucune");
		sf::Text versionText(sf::String::fromUtf8(fullText.begin(), fullText.end()), font, 17);

		versionText.setPosition(sf::Vector2f(10, window.getView().getSize().y - 45));
		versionText.setFillColor(sf::Color::White);

		this->window.draw(versionText);
		this->window.display();
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
			utils::getToString("https://deltarune.fr/installer/version.txt", versionRaw);
			latest = versionRaw;
		}

		std::ifstream in("version.txt");
		if (!in.is_open())
			return false;
		std::string version;
		in >> version;

		return version == latest;
	}

	void UpdaterWindow::_scaleElements()
	{
		auto viewSize = window.getView().getSize();

		float bgScale = viewSize.y / static_cast<float>(backgroundImg.getSize().y);
		this->background.setScale(sf::Vector2f(bgScale, bgScale));
		this->background.setPosition(sf::Vector2f(viewSize.x / 2, viewSize.y / 2));
		this->background.setOrigin(
			sf::Vector2f(background.getLocalBounds().width / 2, background.getLocalBounds().height / 2));

		float logoScale = (viewSize.x * (2 / 3.0f)) / static_cast<float>(logoImg.getSize().x);
		this->logo.setScale(sf::Vector2f(logoScale, logoScale));
		this->logo.setPosition(sf::Vector2f(viewSize.x / 2, viewSize.y * 0.15));
		this->logo.setOrigin(sf::Vector2f(logo.getLocalBounds().width / 2, logo.getLocalBounds().height / 2));

		sf::Vector2f installPos(viewSize.x * 0.27, viewSize.y * 0.57);
		sf::Vector2f installSize(viewSize.x * 0.46, viewSize.y * 0.11);
		sf::Vector2f uninstallPos(viewSize.x / 2 - viewSize.x * 0.115, viewSize.y * 0.795);
		sf::Vector2f uninstallSize(viewSize.x * 0.23, viewSize.y * 0.07);
		sf::Vector2f creditsPos(viewSize.x / 2 - viewSize.x * 0.15, viewSize.y * 0.91);
		sf::Vector2f creditsSize(viewSize.x * 0.3, viewSize.y * 0.07);
		sf::Vector2f tutorialSize(viewSize.x * 0.14, viewSize.y * 0.055);
		sf::Vector2f tutorialPos(viewSize.x - tutorialSize.x - viewSize.x * 0.015, viewSize.y * 0.93);

		float prevResX = static_cast<float>(previousResolution.x);
		this->installButton.setRect(installPos, installSize);
		this->installButton.setFontSize(this->installButton.getFontSize() * (viewSize.x / prevResX));
		this->uninstallButton.setRect(uninstallPos, uninstallSize);
		this->uninstallButton.setFontSize(this->uninstallButton.getFontSize() * (viewSize.x / prevResX));
		this->creditsButton.setRect(creditsPos, creditsSize);
		this->creditsButton.setFontSize(this->creditsButton.getFontSize() * (viewSize.x / prevResX));
		this->tutorialButton.setRect(tutorialPos, tutorialSize);
		this->tutorialButton.setFontSize(this->tutorialButton.getFontSize() * (viewSize.x / prevResX));

		this->progressBar.setPosition(sf::Vector2f(viewSize.x / 2 - viewSize.x * 0.225, viewSize.y * 0.695));
		this->progressBar.setSize(sf::Vector2f(viewSize.x * 0.45, viewSize.y * 0.02));

		this->tutorialText.setCharacterSize(this->tutorialText.getCharacterSize() * (viewSize.x / prevResX));
	}
}
