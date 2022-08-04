#pragma once

#include "globals.hpp"
#include <SFML/Graphics.hpp>

namespace drfr
{
	class ProgressBar
	{
	private:
		std::string desc;
		float current;
		float max;
		sf::RectangleShape rect;
		bool enabled;
		sf::Font font;

	public:
		ProgressBar() = default;
		ProgressBar(const std::string& desc, float current, float max);
		void setProgression(float current);
		void setMax(float max);
		void setDesc(const std::string& desc);
		void setPosition(const sf::Vector2f& pos);
		void setSize(const sf::Vector2f& size);
		void setEnabled(bool enabled);
		void draw(sf::RenderWindow& window);

		float getMax() const;
		float getProgression() const;
	};
}
