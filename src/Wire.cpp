#include "Wire.h"
#include "Constants.h"

void Wire::draw(sf::RenderTarget& target) const {
    sf::Vector2f start = fromPin->shape.getPosition();
    sf::Vector2f end = toPin->shape.getPosition();

    sf::Color wireColor = fromPin->state ? Theme::WireOn : Theme::WireOff;

    if (isSelected) wireColor = Theme::WireSelected;

    float thickness = isSelected ? Config::WireThicknessSelected : Config::WireThickness;

    sf::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (length > 0) {
        sf::Vector2f perpendicular = {-direction.y / length, direction.x / length};

        // Draw shadow for cartoon effect
        sf::Vector2f shadowOffset = {3.0f, 3.0f};
        sf::Vertex shadowLine[] = {
            {start + shadowOffset, sf::Color(0, 0, 0, 100)},
            {end + shadowOffset, sf::Color(0, 0, 0, 100)}
        };
        target.draw(shadowLine, 2, sf::PrimitiveType::Lines);

        // Draw main wire with solid color - multiple lines for thickness
        int lineCount = isSelected ? 8 : 5; // More lines when selected for thicker appearance
        for (int i = 0; i < lineCount; ++i) {
            float t = (float)i / (lineCount - 1) - 0.5f; // -0.5 to 0.5
            float offset = t * thickness;

            sf::Vertex line[] = {
                {start + perpendicular * offset, wireColor},
                {end + perpendicular * offset, wireColor}
            };
            target.draw(line, 2, sf::PrimitiveType::Lines);
        }

        // Draw highlight on top for 3D cartoon effect
        sf::Vertex highlightLine[] = {
            {start + perpendicular * thickness * 0.2f, sf::Color(255, 255, 255, 120)},
            {end + perpendicular * thickness * 0.2f, sf::Color(255, 255, 255, 120)}
        };
        target.draw(highlightLine, 2, sf::PrimitiveType::Lines);

        // Draw bottom shadow for depth
        sf::Vertex bottomShadow[] = {
            {start - perpendicular * thickness * 0.35f, sf::Color(0, 0, 0, 70)},
            {end - perpendicular * thickness * 0.35f, sf::Color(0, 0, 0, 70)}
        };
        target.draw(bottomShadow, 2, sf::PrimitiveType::Lines);
    }
}

void Wire::update() {
    toPin->state = fromPin->state;
}