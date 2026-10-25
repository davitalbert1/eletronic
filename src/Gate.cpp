#include "Gate.h"
#include "GateRegistry.h"
#include <string>

// Pin implementation
Pin::Pin(PinType t, sf::Vector2f o, Gate* p) : type(t), offset(o), parent(p) {
    shape.setRadius(Config::PinRadius);
    shape.setFillColor(Theme::SignalOff);
    shape.setOrigin(sf::Vector2f(Config::PinRadius, Config::PinRadius));

    // Add thick outline to pins for cartoon effect
    shape.setOutlineThickness(3);
    shape.setOutlineColor(Theme::PinOutline);
}

void Pin::update(sf::Vector2f gatePos) {
    shape.setPosition({gatePos.x + offset.x, gatePos.y + offset.y});
    shape.setFillColor(state ? Theme::SignalOn : Theme::SignalOff);

    // Update outline color based on state for cartoon effect
    shape.setOutlineColor(state ? Theme::SignalOn : Theme::PinOutline);
}

bool Pin::contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
}

// Gate implementation
Gate::Gate(GateType t, sf::Vector2f pos, const sf::Font& font)
    : type(t), position(pos), label(font, GateRegistry::instance().getLabel(t), 16) {
    body.setSize({Config::GateWidth, Config::GateHeight});
    body.setPosition(position);
    body.setFillColor(Theme::GateBody);
    body.setOutlineThickness(Config::GateOutlineThickness);
    body.setOutlineColor(Theme::GateOutline);

    label.setFillColor(Theme::Text);
    label.setStyle(sf::Text::Bold);
    updateLabelPos();
}

float Gate::getExtraData() const {
    return 0.f;
}
void  Gate::setExtraData(float) {}

void Gate::drawBase(sf::RenderTarget& target) {
    // Draw pronounced shadow for cartoon effect
    sf::RectangleShape shadow;
    shadow.setSize(body.getSize());
    shadow.setPosition(body.getPosition() + sf::Vector2f(7, 7));
    shadow.setFillColor(Theme::GateShadow);
    target.draw(shadow);

    // Draw main body with thick outline
    target.draw(body);

    // Draw top highlight for 3D cartoon effect
    sf::RectangleShape highlight;
    highlight.setSize({body.getSize().x - Config::GateOutlineThickness * 2 - 4, 8});
    highlight.setPosition(body.getPosition() + sf::Vector2f(Config::GateOutlineThickness + 2, Config::GateOutlineThickness + 2));
    highlight.setFillColor(Theme::GateHighlight);
    target.draw(highlight);

    // Draw bottom shadow for 3D cartoon effect
    sf::RectangleShape bottomShadow;
    bottomShadow.setSize({body.getSize().x - Config::GateOutlineThickness * 2 - 4, 8});
    bottomShadow.setPosition(body.getPosition() + sf::Vector2f(Config::GateOutlineThickness + 2,
                                                               body.getSize().y - Config::GateOutlineThickness - 10));
    bottomShadow.setFillColor(sf::Color(0, 0, 0, 50));
    target.draw(bottomShadow);

    // Draw selection outline with cartoon style
    if (isSelected) {
        // Outer glow effect
        sf::RectangleShape glowOutline;
        glowOutline.setSize(body.getSize() + sf::Vector2f(12, 12));
        glowOutline.setPosition(body.getPosition() - sf::Vector2f(6, 6));
        glowOutline.setFillColor(sf::Color::Transparent);
        glowOutline.setOutlineThickness(Config::SelectionGlowThickness);
        glowOutline.setOutlineColor(Config::SelectionGlow);
        target.draw(glowOutline);

        // Main selection outline
        sf::RectangleShape selectionOutline;
        selectionOutline.setSize(body.getSize() + sf::Vector2f(8, 8));
        selectionOutline.setPosition(body.getPosition() - sf::Vector2f(4, 4));
        selectionOutline.setFillColor(sf::Color::Transparent);
        selectionOutline.setOutlineThickness(4);
        selectionOutline.setOutlineColor(Theme::Highlight);
        target.draw(selectionOutline);

        // Corner accents for better visibility
        float cornerSize = 8.0f;
        sf::RectangleShape corners[4];
        // Top-left
        corners[0].setSize({cornerSize, 3});
        corners[0].setPosition(body.getPosition() - sf::Vector2f(2, 2));
        // Top-right
        corners[1].setSize({cornerSize, 3});
        corners[1].setPosition(body.getPosition() + sf::Vector2f(body.getSize().x - cornerSize + 2, -2));
        // Bottom-left
        corners[2].setSize({cornerSize, 3});
        corners[2].setPosition(body.getPosition() + sf::Vector2f(-2, body.getSize().y - 1));
        // Bottom-right
        corners[3].setSize({cornerSize, 3});
        corners[3].setPosition(body.getPosition() + sf::Vector2f(body.getSize().x - cornerSize + 2, body.getSize().y - 1));

        for (int i = 0; i < 4; ++i) {
            corners[i].setFillColor(Theme::Highlight);
            target.draw(corners[i]);
        }
    }

    target.draw(label);
    for (auto& pin : inputs) {
        pin.update(position);
        target.draw(pin.shape);
    }
    for (auto& pin : outputs) {
        pin.update(position);
        target.draw(pin.shape);
    }
}

void Gate::draw(sf::RenderTarget& target) {
    drawBase(target);
}

void Gate::setPosition(sf::Vector2f pos) {
    position = pos;
    body.setPosition(position);
    updateLabelPos();
}

void Gate::setRotation(float angle) {
    body.setRotation(sf::degrees(angle));
}

float Gate::getRotation() const {
    return body.getRotation().asDegrees();
}

void Gate::updateLabelPos() {
    sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                     bounds.position.y + bounds.size.y / 2.0f});
    label.setPosition({position.x + Config::GateWidth / 2.0f,
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
