#include "Wire.h"
#include "Constants.h"

void Wire::draw(sf::RenderWindow& window) {
    sf::Vector2f start = fromPin->shape.getPosition();
    sf::Vector2f end = toPin->shape.getPosition();
    sf::Color color = fromPin->state ? Theme::WireOn : Theme::WireOff;
    
    sf::Vertex line[] = {
        {start, color},
        {end, color}
    };
    window.draw(line, 2, sf::PrimitiveType::Lines);
}

void Wire::update() {
    toPin->state = fromPin->state;
}