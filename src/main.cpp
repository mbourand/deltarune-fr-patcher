#include "BPSParser.hpp"
#include "UI/UpdaterWindow.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nfd.hpp>

int main()
{
	NFD_Init();

	drfr::UpdaterWindow window;
	window.init();

	sf::Clock framerateClock;
	while (window.isOpen())
	{
		window.handleEvents();
		if (framerateClock.getElapsedTime().asSeconds() >= 1 / static_cast<float>(FRAMERATE))
		{
			framerateClock.restart();
			window.doTick();
			window.render();
		}
	}

	NFD_Quit();
	return 0;
}
