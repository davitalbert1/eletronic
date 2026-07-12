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
    POWER,
    // Novos componentes
    XOR,
    NAND,
    NOR,
    XNOR,
    ROM,
    SEG7,
    LCD,
    LED,
    SWITCH,
    BUTTON,
    COUNTER,
    REGISTER,
    // Componentes de chip personalizado
    INPUT_PIN,   // Pino de entrada (marca pino externo do chip)
    OUTPUT_PIN,  // Pino de saída (marca pino externo do chip)
    CHIP         // Instância de chip personalizado
};

class Gate {
public:
    Gate(GateType t, sf::Vector2f pos, const sf::Font& font);
    virtual ~Gate() = default;

    virtual void updateLogic() = 0;

    virtual void draw(sf::RenderWindow& window);

    virtual void interact() {}
    void drawBase(sf::RenderWindow& window);

    virtual std::shared_ptr<Gate> clone(const sf::Font& font) const = 0;

    void setPosition(sf::Vector2f pos);
    void setRotation(float angle);
    float getRotation() const;
    virtual void updateLabelPos();
    bool contains(sf::Vector2f point) const;

    sf::Vector2f getPosition() const;
    GateType getType() const;

    virtual float getExtraData() const;
    virtual void setExtraData(float data);

    virtual std::string getChipId() const {
        return "";
    }
    virtual std::string getPinName() const {
        return "";
    }
    virtual void setPinName(const std::string&) {}

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