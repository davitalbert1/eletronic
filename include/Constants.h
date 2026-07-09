#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>

namespace Theme {
    const sf::Color Background = sf::Color(12, 12, 14);
    const sf::Color Panel = sf::Color(25, 25, 30);
    const sf::Color TopMenu = sf::Color(35, 35, 45);
    const sf::Color Accent = sf::Color(0, 120, 215);
    const sf::Color Text = sf::Color(220, 220, 230);
    const sf::Color GateBody = sf::Color(40, 40, 45);
    const sf::Color GateOutline = sf::Color(70, 70, 80);
    const sf::Color Highlight = sf::Color(0, 200, 255);
    
    const sf::Color SignalOff = sf::Color(100, 100, 110);
    const sf::Color SignalOn = sf::Color(0, 255, 100);
    const sf::Color WireOn = sf::Color(0, 255, 100, 180);
    const sf::Color WireOff = sf::Color(80, 80, 90, 150);
    
    const sf::Color Overlay = sf::Color(0, 0, 0, 180);
    const sf::Color ModalBg = sf::Color(30, 30, 35);
    const sf::Color ModalBorder = sf::Color(60, 60, 70);
}

namespace Config {
    const float GateWidth = 110.0f;
    const float GateHeight = 70.0f;
    const float PinRadius = 6.0f;
    const float TopMenuHeight = 40.0f;
    const float PaletteWidth = 160.0f;
}

#endif