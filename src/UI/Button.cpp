#include "UI/Button.hpp"
#include "globals.hpp"
#include <SFML/Graphics.hpp>

namespace drfr
{

	Button::Button(const sf::Vector2f& pos, const sf::Vector2f& size, const std::string& text, unsigned int fontSize)
		: pressed(false), hovered(false), fontSize(fontSize)
	{
		this->rect.setPosition(pos);
		this->rect.setSize(size);
		this->text = sf::String::fromUtf8(text.begin(), text.end());
		if (!this->font.loadFromFile("assets/determination.ttf"))
			throw std::runtime_error("La police n'a pas pu être chargée");
	}

	bool Button::isHovered() const { return this->hovered && this->enabled; }

	bool Button::isPressed() const { return this->pressed && this->enabled; }

	void Button::update(const sf::RenderWindow& window, bool mousePressed)
	{
		if (!this->enabled)
		{
			this->pressed = false;
			this->hovered = false;
			return;
		}

		// Is Hovered
		sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		this->hovered = this->rect.getGlobalBounds().contains(sf::Vector2f(mousePos.x, mousePos.y));

		// Is Pressed
		if (!mousePressed)
			this->pressed = false;
		else if (this->hovered && mousePressed && !this->pressedLastFrame)
			this->pressed = true;
		else
			this->pressed = false;
		this->pressedLastFrame = mousePressed;
	}

	void Button::draw(sf::RenderWindow& window) const
	{
		sf::RectangleShape rect(this->rect);
		rect.setFillColor(sf::Color(0x000000ff));

		if (this->hovered)
			rect.setOutlineColor(sf::Color(0xffff05ff));
		else
			rect.setOutlineColor(sf::Color(0xff8903ff));
		rect.setOutlineThickness(5);

		// Avoid vertical alignment issues with characters like p,f,j,etc.
		sf::String cleanedText = this->text;
		for (unsigned int i = 0; i < cleanedText.getSize(); i++)
			cleanedText[i] = 'A';

		sf::Text drawableText(cleanedText, this->font, this->fontSize);
		const_cast<sf::Texture&>(this->font.getTexture(this->fontSize)).setSmooth(false);
		drawableText.setPosition(sf::Vector2f(this->rect.getPosition().x + (this->rect.getSize().x / 2),
											  this->rect.getPosition().y + (this->rect.getSize().y / 2) -
												  (drawableText.getGlobalBounds().height / 2)));
		drawableText.setFillColor(rect.getOutlineColor());
		drawableText.setOrigin(
			sf::Vector2f(drawableText.getLocalBounds().width / 2, drawableText.getLocalBounds().height / 2));
		// Putting back the real text after setting up the alignment
		drawableText.setString(this->text);

		window.draw(rect);
		window.draw(drawableText);
	}

	void Button::setRect(const sf::Vector2f& pos, const sf::Vector2f& size)
	{
		this->rect.setPosition(pos);
		this->rect.setSize(size);
	}

	void Button::setFontSize(unsigned int fontSize) { this->fontSize = fontSize; }

	unsigned int Button::getFontSize() const { return this->fontSize; }

	void Button::setEnabled(bool enabled) { this->enabled = enabled; }

	void Button::setText(const std::string& text) { this->text = sf::String::fromUtf8(text.begin(), text.end()); }
}
