#ifndef WORKBENCH_ACCESSIBILITY_HPP
#define WORKBENCH_ACCESSIBILITY_HPP

#include <SFML/Graphics.hpp>
#include "Constants.h"

class WorkbenchAccessibility {
public:
    WorkbenchAccessibility() = default;

    // Theme management
    void cycleTheme();
    void applyTheme();
    ThemeType getCurrentTheme() const {
        return currentTheme_;
    }
    void setTheme(ThemeType theme) {
        currentTheme_ = theme;
        applyTheme();
    }

    // Font scale management
    void cycleFontScale();
    void applyFontScale();
    FontScale getCurrentFontScale() const {
        return currentFontScale_;
    }
    void setFontScale(FontScale scale) {
        currentFontScale_ = scale;
        applyFontScale();
    }

    // High contrast mode
    void toggleHighContrast();
    bool isHighContrastMode() const {
        return highContrastMode_;
    }
    void setHighContrastMode(bool enabled) {
        highContrastMode_ = enabled;
    }

    // Large text mode
    void toggleLargeText();
    bool isLargeTextMode() const {
        return largeTextMode_;
    }
    void setLargeTextMode(bool enabled) {
        largeTextMode_ = enabled;
    }

    // Pattern type
    void setPattern(Config::PatternType pattern) {
        currentPattern_ = pattern;
    }
    Config::PatternType getPattern() const {
        return currentPattern_;
    }
    void cyclePattern();

    // Font size helper
    float getFontSizeMultiplier() const;

    // Theme colors
    sf::Color getThemeColor(const sf::Color& defaultColor) const;

private:
    ThemeType currentTheme_ = ThemeType::DARK;
    FontScale currentFontScale_ = FontScale::NORMAL;
    Config::PatternType currentPattern_ = Config::PatternType::NONE;
    bool highContrastMode_ = false;
    bool largeTextMode_ = false;
};

#endif