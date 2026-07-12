#ifndef WORKBENCH_UI_HPP
#define WORKBENCH_UI_HPP

#include <SFML/Graphics.hpp>
#include "Constants.h"

class WorkbenchUI {
public:
    WorkbenchUI(const sf::Font& font);

    // Setup methods
    void setupMenu();
    void setupPalette();
    void setupModal();
    void setupTooltip();

    // Drawing methods
    void drawMenu(sf::RenderWindow& window);
    void drawPalette(sf::RenderWindow& window);
    void drawModal(sf::RenderWindow& window, bool isSaveModal, bool isLoadModal, float modalAlpha);
    void drawTooltip(sf::RenderWindow& window, const sf::Vector2f& mousePos);
    void drawProgressBar(sf::RenderWindow& window, bool isSaving, bool isLoading, float progress);
    void drawStatusMessage(sf::RenderWindow& window, const std::string& message, float timer);
    void drawConfirmationDialog(sf::RenderWindow& window, bool show, const std::string& message);
    void drawHelpOverlay(sf::RenderWindow& window, bool show, const sf::Font& font);
    void drawFPS(sf::RenderWindow& window, bool show, float fps, bool performanceMode);

    // Update methods
    void updateMenuAnimation(bool isHovered, bool isPressed, float deltaTime);
    void updateDropdownAnimation(bool show, float deltaTime);
    void updateModalAnimation(bool show, float deltaTime);
    void updateTooltip(const std::string& text, float deltaTime);
    void updateStatusMessage(float deltaTime, float& timer);

    // Getters
    bool isMenuButtonHovered() const { return isMenuButtonHovered_; }
    bool isMenuButtonPressed() const { return isMenuButtonPressed_; }
    bool showDropdown() const { return showDropdown_; }
    float getMenuButtonAlpha() const { return menuButtonAlpha_; }
    float getDropdownAlpha() const { return dropdownAlpha_; }
    float getModalAlpha() const { return modalAlpha_; }
    const std::string& getCurrentTooltip() const { return currentTooltip_; }
    bool isTooltipReady() const { return tooltipTimer_ >= TooltipDelay; }

    // Setters
    void setMenuButtonHovered(bool hovered) { isMenuButtonHovered_ = hovered; }
    void setMenuButtonPressed(bool pressed) { isMenuButtonPressed_ = pressed; }
    void setShowDropdown(bool show) { showDropdown_ = show; }
    void setCurrentTooltip(const std::string& text) { currentTooltip_ = text; }
    void clearTooltip() { currentTooltip_.clear(); tooltipTimer_ = 0.0f; }

    // UI element access
    sf::RectangleShape& getMenuButton() { return menuButton_; }
    sf::Text& getMenuText() { return menuText_; }
    sf::Text& getNewText() { return newText_; }
    sf::Text& getSaveText() { return saveText_; }
    sf::Text& getLoadText() { return loadText_; }
    sf::Text& getSaveAsText() { return saveAsText_; }
    bool isNewHovered() const { return isNewHovered_; }
    bool isSaveHovered() const { return isSaveHovered_; }
    bool isLoadHovered() const { return isLoadHovered_; }
    bool isSaveAsHovered() const { return isSaveAsHovered_; }
    void setNewHovered(bool h) { isNewHovered_ = h; }
    void setSaveHovered(bool h) { isSaveHovered_ = h; }
    void setLoadHovered(bool h) { isLoadHovered_ = h; }
    void setSaveAsHovered(bool h) { isSaveAsHovered_ = h; }

    // Modal geometry helpers
    static sf::FloatRect chipRect(size_t i, float startY);
    sf::FloatRect nameInputRect() const;
    sf::FloatRect categoryInputRect() const;
    sf::FloatRect descriptionInputRect() const;
    sf::FloatRect tagsInputRect() const;
    sf::FloatRect saveButtonRect() const;
    sf::FloatRect exportJsonRect() const;
    sf::FloatRect exportXmlRect() const;
    sf::FloatRect loadRowRect(size_t i) const;
    sf::FloatRect loadRowLoadBtnRect(size_t i) const;
    sf::FloatRect loadRowDeleteBtnRect(size_t i) const;
    sf::FloatRect loadSearchRect() const;
    sf::FloatRect loadSortBtnRect() const;
    sf::FloatRect loadStarRect(size_t i) const;
    sf::FloatRect loadDetailPanelRect() const;
    sf::FloatRect loadSaveMetaBtnRect() const;

    // Constants
    static constexpr float ModalX = 340.f, ModalY = 140.f, ModalW = 520.f, ModalH = 560.f;
    static constexpr float LoadModalH = 580.f;
    static constexpr float ChipW = 100.f, ChipH = 24.f, ChipGap = 8.f;
    static constexpr float TooltipDelay = 0.5f;
    static constexpr float BackupInterval = 300.0f;

private:
    const sf::Font& font_;

    // Menu elements
    sf::RectangleShape menuBar_, menuButton_, dropdown_;
    sf::Text menuText_, saveText_, loadText_, newText_, saveAsText_;
    bool showDropdown_ = false;
    bool isMenuButtonHovered_ = false;
    bool isMenuButtonPressed_ = false;
    bool isNewHovered_ = false;
    bool isSaveHovered_ = false;
    bool isLoadHovered_ = false;
    bool isSaveAsHovered_ = false;

    // Animation
    float menuButtonAlpha_ = 255.0f;
    float dropdownAlpha_ = 255.0f;

    // Modal elements
    sf::RectangleShape overlay_, modalRect_;
    sf::Text modalTitle_, inputText_, listText_;
    float modalAlpha_ = 255.0f;

    // Tooltip
    std::string currentTooltip_;
    sf::Text tooltipText_;
    sf::RectangleShape tooltipBg_;
    float tooltipTimer_ = 0.0f;

    // Progress
    sf::RectangleShape progressBar_;
    sf::Text progressText_;

    // Status
    std::string statusMessage_;
    float statusMessageTimer_ = 0.0f;

    // Palette
    sf::RectangleShape paletteRect_;
};

#endif