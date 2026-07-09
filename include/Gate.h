#ifndef GATE_H
#define GATE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include "Constants.h"

enum class PinType {
    INPUT,
    OUTPUT
};

struct Pin {
    PinType type;
    sf::Vector2f offset;
    bool state = false;
    class Gate* parent;

    sf::CircleShape shape;

    Pin(PinType t, sf::Vector2f o, class Gate* p);
    void update(sf::Vector2f gatePos);
    bool contains(sf::Vector2f point) const;
};

enum class GateType {
    AND,
    OR,
    NOT,
    CLOCK,
    POWER
};

class Gate {
public:
    Gate(GateType t, sf::Vector2f pos, const sf::Font& font);
    virtual ~Gate() = default;

    virtual void updateLogic() = 0;

    void draw(sf::RenderWindow& window);
    void setPosition(sf::Vector2f pos);
    void updateLabelPos();
    bool contains(sf::Vector2f point) const;
    
    sf::Vector2f getPosition() const;
    GateType getType() const;

    virtual float getExtraData() const;
    virtual void setExtraData(float data);

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