#include "WorkbenchAccessibility.h"
#include "Constants.h"

void WorkbenchAccessibility::cycleTheme() {
    int themeInt = static_cast<int>(currentTheme_);
    themeInt = (themeInt + 1) % 3;
    currentTheme_ = static_cast<ThemeType>(themeInt);
    applyTheme();
}

void WorkbenchAccessibility::cycleFontScale() {
    int scaleInt = static_cast<int>(currentFontScale_);
    scaleInt = (scaleInt + 1) % 4;
    currentFontScale_ = static_cast<FontScale>(scaleInt);
    applyFontScale();
}

void WorkbenchAccessibility::toggleHighContrast() {
    highContrastMode_ = !highContrastMode_;
    if (highContrastMode_) {
        currentTheme_ = ThemeType::HIGH_CONTRAST;
    } else {
        currentTheme_ = ThemeType::DARK;
    }
    applyTheme();
}

void WorkbenchAccessibility::toggleLargeText() {
    largeTextMode_ = !largeTextMode_;
    if (largeTextMode_) {
        currentFontScale_ = FontScale::LARGE;
    } else {
        currentFontScale_ = FontScale::NORMAL;
    }
    applyFontScale();
}

void WorkbenchAccessibility::cyclePattern() {
    int patternInt = static_cast<int>(currentPattern_);
    patternInt = (patternInt + 1) % 4;
    currentPattern_ = static_cast<Config::PatternType>(patternInt);
}

void WorkbenchAccessibility::applyTheme() {
    // Theme is applied via Constants.h Theme namespace
    // This method exists for future extensibility
}

void WorkbenchAccessibility::applyFontScale() {
    // Font scale is applied via Constants.h Accessibility namespace
    // This method exists for future extensibility
}

float WorkbenchAccessibility::getFontSizeMultiplier() const {
    return Accessibility::getFontScale(currentFontScale_);
}

sf::Color WorkbenchAccessibility::getThemeColor(const sf::Color& defaultColor) const {
    // Helper to get theme-aware color (keeps original as fallback)
    return defaultColor;
}