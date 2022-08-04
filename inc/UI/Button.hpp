#pragma once

#include <SFML/Graphics.hpp>

namespace drfr
{
	class Button
	{
	private:
		bool hovered;
		bool pressed;
		sf::RectangleShape rect;
		sf::String text;
		unsigned int fontSize;
		sf::Font font;
		bool pressedLastFrame = false;
		bool enabled = true;

	public:
		Button() = default;
		Button(const sf::Vector2f& pos, const sf::Vector2f& size, const std::string& text, unsigned int fontSize);

		bool isHovered() const;
		bool isPressed() const;

		void setRect(const sf::Vector2f& pos, const sf::Vector2f& size);
		void setFontSize(unsigned int fontSize);
		void setText(const std::string& text);
		unsigned int getFontSize() const;

		void update(const sf::RenderWindow& window, bool mousePreessed);
		void draw(sf::RenderWindow& window) const;

		void setEnabled(bool enabled);
	};
}
