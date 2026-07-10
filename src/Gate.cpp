#include "Gate.h"
#include <string>
#include <array>

// Labels for each GateType (order must match GateType enum)
static const std::array<std::string, 5> GateLabels = {"AND", "OR", "NOT", "CLOCK", "POWER"};

// Pin implementation
Pin::Pin(PinType t, sf::Vector2f o, Gate* p) : type(t), offset(o), parent(p) {
    shape.setRadius(Config::PinRadius);
    shape.setFillColor(Theme::SignalOff);
    shape.setOrigin(sf::Vector2f(Config::PinRadius, Config::PinRadius));
}

void Pin::update(sf::Vector2f gatePos) {
    shape.setPosition({gatePos.x + offset.x, gatePos.y + offset.y});
    shape.setFillColor(state ? Theme::SignalOn : Theme::SignalOff);
}

bool Pin::contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
}

// Gate implementation
Gate::Gate(GateType t, sf::Vector2f pos, const sf::Font& font)
    : type(t), position(pos), label(font, GateLabels[static_cast<int>(t)], 14) {

    body.setSize({Config::GateWidth, Config::GateHeight});
    body.setPosition(position);
    body.setFillColor(Theme::GateBody);
    body.setOutlineThickness(2);
    body.setOutlineColor(Theme::GateOutline);

    label.setFillColor(Theme::Text);
    updateLabelPos();
}

float Gate::getExtraData() const {
    return 0.f;
}
void  Gate::setExtraData(float) {}

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
    for (auto& pin : inputs)  {
        pin.update(position);
        window.draw(pin.shape);
    }
    for (auto& pin : outputs) {
        pin.update(position);
        window.draw(pin.shape);
    }
}

void Gate::setPosition(sf::Vector2f pos) {
    position = pos;
    body.setPosition(position);
    updateLabelPos();
}

void Gate::updateLabelPos() {
    sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                     bounds.position.y + bounds.size.y / 2.0f});
    label.setPosition({position.x + Config::GateWidth  / 2.0f,
                       position.y + Config::GateHeight / 2.0f});
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
