#ifndef GATE_HPP
#define GATE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include "Constants.hpp"

enum class PinType { INPUT, OUTPUT };

struct Pin {
    PinType type;
    sf::Vector2f offset;
    bool state = false;
    class Gate* parent;

    sf::CircleShape shape;

    Pin(PinType t, sf::Vector2f o, class Gate* p) : type(t), offset(o), parent(p) {
        shape.setRadius(Config::PinRadius);
        shape.setOrigin({Config::PinRadius, Config::PinRadius});
        shape.setFillColor(Theme::SignalOff);
        shape.setOutlineThickness(1);
        shape.setOutlineColor(Theme::GateOutline);
    }

    void update(sf::Vector2f gatePos) {
        shape.setPosition({gatePos.x + offset.x, gatePos.y + offset.y});
        shape.setFillColor(state ? Theme::SignalOn : Theme::SignalOff);
    }

    bool contains(sf::Vector2f point) const {
        return shape.getGlobalBounds().contains(point);
    }
};

enum class GateType { AND, OR, NOT, CLOCK, POWER };

class Gate {
public:
    Gate(GateType t, sf::Vector2f pos, const sf::Font& font) 
        : type(t), position(pos), label(font), isDragging(false) {
        
        body.setSize({Config::GateWidth, Config::GateHeight});
        body.setFillColor(Theme::GateBody);
        body.setOutlineThickness(2);
        body.setOutlineColor(Theme::GateOutline);

        label.setCharacterSize(16);
        label.setFillColor(Theme::Text);
    }

    virtual ~Gate() = default;

    virtual void updateLogic() = 0;

    void draw(sf::RenderWindow& window) {
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

    void setPosition(sf::Vector2f pos) {
        position = pos;
        body.setPosition(position);
        updateLabelPos();
    }

    void updateLabelPos() {
        sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin({bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f});
        label.setPosition({position.x + Config::GateWidth / 2.0f, position.y + Config::GateHeight / 2.0f});
    }

    bool contains(sf::Vector2f point) const { return body.getGlobalBounds().contains(point); }
    
    sf::Vector2f getPosition() const { return position; }
    GateType getType() const { return type; }

    virtual float getExtraData() const { return 0.0f; }
    virtual void setExtraData(float data) {}

    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
    bool isDragging = false;
    bool isSelected = false;
    sf::Vector2f dragOffset;

protected:
    GateType type;
    sf::Vector2f position;
    sf::RectangleShape body;
    sf::Text label;
};

#endif
