#include "Gate.h"
#include "Constants.h"

// Implementações dos métodos de Pin
void Pin::update(sf::Vector2f gatePos) {
    shape.setPosition({gatePos.x + offset.x, gatePos.y + offset.y});
    shape.setFillColor(state ? Theme::SignalOn : Theme::SignalOff);
}

bool Pin::contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
}

// Implementações dos métodos de Gate
void Gate::draw(sf::RenderWindow& window) {
    if (isSelected) {
        sf::RectangleShape highlight;
        highlight.setSize(body.getSize());
        highlight.setPosition(body.getPosition());
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineThickness(4);
        highlight.setOutlineColor(Theme::Highlight);
        window.draw(highlight);
    }
    window.draw(body);
    window.draw(label);
    for (auto& pin : inputs) pin.update(position), window.draw(pin.shape);
    for (auto& pin : outputs) pin.update(position), window.draw(pin.shape);
}

void Gate::setPosition(sf::Vector2f pos) {
    position = pos;
    body.setPosition(position);
    updateLabelPos();
}

void Gate::updateLabelPos() {
    sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin({bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f});
    label.setPosition({position.x + Config::GateWidth / 2.0f, position.y + Config::GateHeight / 2.0f});
}

bool Gate::contains(sf::Vector2f point) const { 
    return body.getGlobalBounds().contains(point); 
}

sf::Vector2f Gate::getPosition() const { 
    return position; 
}

GateType Gate::getType() const { 
    return type; 
}