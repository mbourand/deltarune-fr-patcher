#include "UI/ProgressBar.hpp"
#include "globals.hpp"

namespace drfr
{
	ProgressBar::ProgressBar(const std::wstring& desc, float current, float max)
		: desc(desc), current(current), max(max), enabled(true)
	{
		if (!this->font.loadFromFile("assets/determination.ttf"))
			throw std::runtime_error("La police n'a pas pu être chargée");
	}

	void ProgressBar::setProgression(float current) { this->current = current; }
	void ProgressBar::setMax(float max) { this->max = max; }
	void ProgressBar::setDesc(const std::wstring& desc) { this->desc = desc; }
	void ProgressBar::setPosition(const sf::Vector2f& pos) { this->rect.setPosition(pos); }
	void ProgressBar::setSize(const sf::Vector2f& size) { this->rect.setSize(size); }
	void ProgressBar::setEnabled(bool enabled) { this->enabled = enabled; }

	float ProgressBar::getProgression() const { return this->current; }
	float ProgressBar::getMax() const { return this->max; }

	void ProgressBar::draw(sf::RenderWindow& window)
	{
		if (!this->enabled)
			return;

		this->rect.setFillColor(sf::Color(220, 220, 220, 255));
		this->rect.setOutlineColor(sf::Color(0, 0, 0, 255));
		this->rect.setOutlineThickness(1);

		sf::RectangleShape bar;
		bar.setPosition(this->rect.getPosition());
		bar.setSize(sf::Vector2f(this->rect.getSize().x * this->current / this->max, this->rect.getSize().y));
		bar.setFillColor(sf::Color(0x00E000FF));

		unsigned int font_size = 24;

		sf::Text text(sf::String::fromUtf8(this->desc.begin(), this->desc.end()), this->font, font_size);
		const_cast<sf::Texture&>(this->font.getTexture(font_size)).setSmooth(false);
		text.setPosition(this->rect.getPosition() + sf::Vector2f(0, this->rect.getSize().y + 10));
		text.setFillColor(sf::Color::White);

		window.draw(this->rect);
		window.draw(bar);
		window.draw(text);
	}
}
