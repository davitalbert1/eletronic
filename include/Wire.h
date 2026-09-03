#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include <algorithm>
#include "Constants.h"
#include "Gate.h"

class Wire {
public:
    Wire(Pin* from, Pin* to) : fromPin(from), toPin(to) {}

    void draw(sf::RenderTarget& target) const;
    void update();

    bool contains(sf::Vector2f p) const {
        sf::Vector2f a = fromPin->shape.getPosition();
        sf::Vector2f b = toPin->shape.getPosition();
        float dist = std::abs((b.y - a.y) * p.x - (b.x - a.x) * p.y + b.x * a.y - b.y * a.x) /
                    std::sqrt(std::pow(b.y - a.y, 2) + std::pow(b.x - a.x, 2));

        // Check if Point is within the segment's bounding box
        if (p.x < std::min(a.x, b.x) - 5 || p.x > std::max(a.x, b.x) + 5 ||
            p.y < std::min(a.y, b.y) - 5 || p.y > std::max(a.y, b.y) + 5) return false;

        return dist < 8.0f; // Threshold for clicking a wire
    }

    Pin* fromPin;
    Pin* toPin;
    bool isSelected = false;
};
