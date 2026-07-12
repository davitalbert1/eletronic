#ifndef CHIP_CIRCUIT_H
#define CHIP_CIRCUIT_H
#include <vector>
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"

namespace chip {
    class ChipDefinition;
    class Circuit {
    public:
        Circuit() = default;

        // Gates e wires do circuito (propriedade exclusiva do Circuit).
        std::vector<std::shared_ptr<Gate>> gates;
        std::vector<Wire> wires;

        std::shared_ptr<Circuit> clone(const sf::Font& font) const;

        void simulate();
        void draw(sf::RenderWindow& window);
        std::vector<Gate*> findGatesOfType(GateType t);
    };
}

#endif