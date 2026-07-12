#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>

// Theme enumeration
enum class ThemeType {
    DARK,        // Default dark theme
    LIGHT,       // Light alternative theme
    HIGH_CONTRAST // High contrast theme for accessibility
};

// Font scale enumeration
enum class FontScale {
    SMALL,   // 0.75x
    NORMAL,  // 1.0x
    LARGE,   // 1.25x
    XLARGE // 1.5x
};

namespace Theme {
    // DARK THEME (Default)
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

    // ==================== LIGHT THEME ====================
    namespace Light {
        const sf::Color Background = sf::Color(240, 240, 245);
        const sf::Color Panel = sf::Color(255, 255, 255);
        const sf::Color TopMenu = sf::Color(220, 220, 230);
        const sf::Color Accent = sf::Color(0, 120, 215);
        const sf::Color Text = sf::Color(30, 30, 30);  // Near black for contrast

        const sf::Color GateBody = sf::Color(200, 205, 215);
        const sf::Color GateOutline = sf::Color(80, 80, 90);
        const sf::Color GateHighlight = sf::Color(255, 255, 255, 180);
        const sf::Color GateShadow = sf::Color(0, 0, 0, 50);
        const sf::Color Highlight = sf::Color(0, 120, 215);

        const sf::Color SignalOff = sf::Color(150, 150, 160);
        const sf::Color SignalOn = sf::Color(0, 180, 80);

        const sf::Color WireOn = sf::Color(0, 200, 100);
        const sf::Color WireOff = sf::Color(160, 160, 170);
        const sf::Color WireSelected = sf::Color(0, 120, 215);

        const sf::Color Overlay = sf::Color(0, 0, 0, 150);
        const sf::Color ModalBg = sf::Color(255, 255, 255);
        const sf::Color ModalBorder = sf::Color(150, 150, 160);

        const sf::Color PinOutline = sf::Color(60, 60, 70);
        const sf::Color PinHighlight = sf::Color(255, 255, 255, 200);

        const sf::Color ButtonNormal = sf::Color(230, 230, 240);
        const sf::Color ButtonHover = sf::Color(210, 210, 225);
        const sf::Color ButtonPressed = sf::Color(200, 200, 215);
        const sf::Color ButtonActive = sf::Color(0, 120, 215);

        const sf::Color TooltipBg = sf::Color(250, 250, 252, 240);
        const sf::Color TooltipBorder = sf::Color(150, 150, 160);
        const sf::Color TooltipText = sf::Color(30, 30, 30);

        const sf::Color ProgressBg = sf::Color(220, 220, 230);
        const sf::Color ProgressFill = sf::Color(0, 120, 215);
        const sf::Color ProgressBorder = sf::Color(150, 150, 160);
    }

    // ==================== HIGH CONTRAST THEME ====================
    namespace HighContrast {
        const sf::Color Background = sf::Color(0, 0, 0);
        const sf::Color Panel = sf::Color(0, 0, 0);
        const sf::Color TopMenu = sf::Color(0, 0, 0);
        const sf::Color Accent = sf::Color(255, 255, 0);  // Yellow for maximum visibility
        const sf::Color Text = sf::Color(255, 255, 255);  // Pure white

        const sf::Color GateBody = sf::Color(0, 0, 0);
        const sf::Color GateOutline = sf::Color(255, 255, 255);  // White outline
        const sf::Color GateHighlight = sf::Color(255, 255, 255, 200);
        const sf::Color GateShadow = sf::Color(255, 255, 255, 100);
        const sf::Color Highlight = sf::Color(255, 255, 0);

        const sf::Color SignalOff = sf::Color(128, 128, 128);
        const sf::Color SignalOn = sf::Color(255, 255, 0);  // Yellow for on state

        const sf::Color WireOn = sf::Color(255, 255, 0);
        const sf::Color WireOff = sf::Color(128, 128, 128);
        const sf::Color WireSelected = sf::Color(255, 255, 0);

        const sf::Color Overlay = sf::Color(0, 0, 0, 220);
        const sf::Color ModalBg = sf::Color(0, 0, 0);
        const sf::Color ModalBorder = sf::Color(255, 255, 255);

        const sf::Color PinOutline = sf::Color(255, 255, 255);
        const sf::Color PinHighlight = sf::Color(255, 255, 255, 255);

        const sf::Color ButtonNormal = sf::Color(0, 0, 0);
        const sf::Color ButtonHover = sf::Color(30, 30, 30);
        const sf::Color ButtonPressed = sf::Color(50, 50, 50);
        const sf::Color ButtonActive = sf::Color(255, 255, 0);

        const sf::Color TooltipBg = sf::Color(0, 0, 0, 240);
        const sf::Color TooltipBorder = sf::Color(255, 255, 255);
        const sf::Color TooltipText = sf::Color(255, 255, 255);

        const sf::Color ProgressBg = sf::Color(30, 30, 30);
        const sf::Color ProgressFill = sf::Color(255, 255, 0);
        const sf::Color ProgressBorder = sf::Color(255, 255, 255);
    }
}

namespace Config {
    constexpr float GateWidth = 120.0f;
    constexpr float GateHeight = 80.0f;
    constexpr float PinRadius = 9.0f;
    constexpr float TopMenuHeight = 45.0f;
    constexpr float PaletteWidth = 170.0f;
    constexpr float WireThickness = 6.0f;
    constexpr float WireThicknessSelected = 12.0f;  // Thicker when selected
    constexpr float GateCornerRadius = 15.0f;
    constexpr float GateOutlineThickness = 4.0f;

    // Grid and snapping
    constexpr float GridSize = 20.0f;
    constexpr bool SnapToGridEnabled = true;

    // Rotation
    constexpr float RotationStep = 90.0f;  // degrees

    // Selection
    constexpr bool MultiSelectWithShift = true;

    // Selection rectangle visual
    const sf::Color SelectionRectFill = sf::Color(0, 160, 255, 50);  // Mais transparente
    const sf::Color SelectionRectOutline = sf::Color(0, 220, 255);   // Cor mais vibrante
    constexpr float SelectionRectOutlineThickness = 2.0f;

// Selection indicators
const sf::Color SelectionGlow = sf::Color(0, 220, 255, 80);
constexpr float SelectionGlowThickness = 6.0f;

// ==================== PERFORMANCE SETTINGS ====================
// LOD (Level of Detail) thresholds
constexpr float LODDistanceNear = 300.0f;      // Full detail
constexpr float LODDistanceFar = 800.0f;       // Minimal detail
constexpr size_t LODComponentThreshold = 50;   // Enable LOD when components exceed this

// Performance optimization flags
constexpr bool EnableShadows = true;
constexpr bool EnableEffects = true;
constexpr bool EnableLOD = true;

// ==================== ACCESSIBILITY SETTINGS ====================
    // Font scale multipliers
    constexpr float FontScaleSmall = 0.75f;
    constexpr float FontScaleNormal = 1.0f;
    constexpr float FontScaleLarge = 1.25f;
    constexpr float FontScaleXLarge = 1.5f;

    // Visual pattern types for colorblind accessibility
    enum class PatternType {
        NONE,           // No pattern (default)
        STRIPES,        // Diagonal stripes
        DOTS,           // Dots pattern
        GRID,           // Grid pattern
        WAVES           // Wave pattern
    };
}

// Accessibility helper functions
namespace Accessibility {
    // Get current theme colors based on theme type
    inline sf::Color getBackground(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::Background;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::Background;
        return Theme::Background;
    }

    inline sf::Color getPanel(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::Panel;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::Panel;
        return Theme::Panel;
    }

    inline sf::Color getTopMenu(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::TopMenu;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::TopMenu;
        return Theme::TopMenu;
    }

    inline sf::Color getAccent(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::Accent;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::Accent;
        return Theme::Accent;
    }

    inline sf::Color getText(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::Text;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::Text;
        return Theme::Text;
    }

    inline sf::Color getGateBody(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::GateBody;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::GateBody;
        return Theme::GateBody;
    }

    inline sf::Color getGateOutline(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::GateOutline;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::GateOutline;
        return Theme::GateOutline;
    }

    inline sf::Color getHighlight(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::Highlight;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::Highlight;
        return Theme::Highlight;
    }

    inline sf::Color getSignalOn(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::SignalOn;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::SignalOn;
        return Theme::SignalOn;
    }

    inline sf::Color getSignalOff(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::SignalOff;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::SignalOff;
        return Theme::SignalOff;
    }

    inline sf::Color getWireOn(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::WireOn;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::WireOn;
        return Theme::WireOn;
    }

    inline sf::Color getWireOff(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::WireOff;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::WireOff;
        return Theme::WireOff;
    }

    inline sf::Color getModalBg(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::ModalBg;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::ModalBg;
        return Theme::ModalBg;
    }

    inline sf::Color getButtonNormal(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::ButtonNormal;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::ButtonNormal;
        return Theme::ButtonNormal;
    }

    inline sf::Color getButtonHover(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::ButtonHover;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::ButtonHover;
        return Theme::ButtonHover;
    }

    inline sf::Color getButtonActive(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::ButtonActive;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::ButtonActive;
        return Theme::ButtonActive;
    }

    inline sf::Color getTooltipBg(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::TooltipBg;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::TooltipBg;
        return Theme::TooltipBg;
    }

    inline sf::Color getTooltipText(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::TooltipText;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::TooltipText;
        return Theme::TooltipText;
    }

    inline sf::Color getProgressFill(ThemeType theme) {
        if (theme == ThemeType::LIGHT) return Theme::Light::ProgressFill;
        if (theme == ThemeType::HIGH_CONTRAST) return Theme::HighContrast::ProgressFill;
        return Theme::ProgressFill;
    }

    // Get font scale multiplier
    inline float getFontScale(FontScale scale) {
        switch (scale) {
            case FontScale::SMALL: return Config::FontScaleSmall;
            case FontScale::NORMAL: return Config::FontScaleNormal;
            case FontScale::LARGE: return Config::FontScaleLarge;
            case FontScale::XLARGE: return Config::FontScaleXLarge;
            default: return Config::FontScaleNormal;
        }
    }
}

#endif
