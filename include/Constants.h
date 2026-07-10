#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>

namespace Theme {
    // WCAG AA compliant colors - minimum 4.5:1 contrast ratio
    const sf::Color Background = sf::Color(30, 30, 45);
    const sf::Color Panel = sf::Color(45, 45, 65);
    const sf::Color TopMenu = sf::Color(55, 55, 80);
    const sf::Color Accent = sf::Color(0, 160, 255);
    const sf::Color Text = sf::Color(255, 255, 255);  // Pure white for maximum contrast
    
    // Cartoon-style gate colors - absolute and vibrant
    const sf::Color GateBody = sf::Color(100, 105, 120);
    const sf::Color GateOutline = sf::Color(20, 20, 30);
    const sf::Color GateHighlight = sf::Color(255, 255, 255, 100);
    const sf::Color GateShadow = sf::Color(0, 0, 0, 130);
    const sf::Color Highlight = sf::Color(0, 220, 255);

    const sf::Color SignalOff = sf::Color(160, 160, 175);
    const sf::Color SignalOn = sf::Color(50, 255, 150);
    
    // Wire colors - solid and lighter when powered
    const sf::Color WireOn = sf::Color(100, 255, 180);
    const sf::Color WireOff = sf::Color(140, 140, 155);
    const sf::Color WireSelected = sf::Color(0, 220, 255);

    const sf::Color Overlay = sf::Color(0, 0, 0, 200);
    const sf::Color ModalBg = sf::Color(50, 50, 70);
    const sf::Color ModalBorder = sf::Color(100, 100, 120);

    // Cartoon-style pin colors
    const sf::Color PinOutline = sf::Color(15, 15, 25);
    const sf::Color PinHighlight = sf::Color(255, 255, 255, 120);
    
    // Button states for visual feedback
    const sf::Color ButtonNormal = sf::Color(60, 60, 85);
    const sf::Color ButtonHover = sf::Color(80, 80, 110);
    const sf::Color ButtonPressed = sf::Color(40, 40, 60);
    const sf::Color ButtonActive = sf::Color(0, 180, 255);
    
    // Tooltip colors
    const sf::Color TooltipBg = sf::Color(20, 20, 30, 230);
    const sf::Color TooltipBorder = sf::Color(100, 100, 120);
    const sf::Color TooltipText = sf::Color(255, 255, 240);
    
    // Progress indicator colors
    const sf::Color ProgressBg = sf::Color(60, 60, 80);
    const sf::Color ProgressFill = sf::Color(0, 200, 255);
    const sf::Color ProgressBorder = sf::Color(100, 100, 120);
}

namespace Config {
    const float GateWidth = 120.0f;
    const float GateHeight = 80.0f;
    const float PinRadius = 9.0f;
    const float TopMenuHeight = 45.0f;
    const float PaletteWidth = 170.0f;
    const float WireThickness = 6.0f;
    const float WireThicknessSelected = 12.0f;  // Thicker when selected
    const float GateCornerRadius = 15.0f;
    const float GateOutlineThickness = 4.0f;
    
    // Grid and snapping
    const float GridSize = 20.0f;
    const bool SnapToGridEnabled = true;
    
    // Rotation
    const float RotationStep = 90.0f;  // degrees
    
    // Selection
    const bool MultiSelectWithShift = true;
    
    // Selection rectangle visual
    const sf::Color SelectionRectFill = sf::Color(0, 160, 255, 50);  // Mais transparente
    const sf::Color SelectionRectOutline = sf::Color(0, 220, 255);   // Cor mais vibrante
    const float SelectionRectOutlineThickness = 2.0f;
    
    // Selection indicators
    const sf::Color SelectionGlow = sf::Color(0, 220, 255, 80);
    const float SelectionGlowThickness = 6.0f;
}

#endif