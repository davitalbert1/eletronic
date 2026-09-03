#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "Gate.h"
#include "Wire.h"
#include "WorkbenchProject.h"
#include "Constants.h"

class WorkbenchInput {
public:
    // Input state tracking
    struct InputState {
        // Mouse state
        bool isMenuButtonHovered = false;
        bool isMenuButtonPressed = false;
        bool isNewHovered = false;
        bool isSaveHovered = false;
        bool isLoadHovered = false;
        bool isSaveAsHovered = false;
        bool showDropdown = false;

        // Interaction state
        bool isSelecting = false;
        bool isMultiSelecting = false;
        bool isWiring = false;
        Pin* selectedPin = nullptr;
        sf::Vector2f wireStartPos;
        sf::Vector2f selectStart;

        // Modal focus
        enum class ModalFocus {
            NAME,
            CATEGORY,
            DESCRIPTION,
            TAGS,
            SEARCH
        };
        ModalFocus modalFocus = ModalFocus::NAME;
    };

    // Component grouping for organization
    struct ComponentGroup {
        std::string name;
        std::vector<std::shared_ptr<Gate>> gates;
        sf::Vector2f center;
    };

    // Selection filter types
    enum class SelectionFilter {
        ALL,
        GATES_ONLY,
        WIRES_ONLY,
        AND_GATES,
        OR_GATES,
        NOT_GATES,
        CLOCK_GATES,
        POWER_SOURCES
    };

    // Mouse event handlers
    void handleMouseMove(const sf::Event::MouseMoved& event,
                        const sf::Vector2f& mousePos,
                        const std::vector<std::shared_ptr<Gate>>& placedGates,
                        const std::vector<Wire>& wires,
                        const sf::FloatRect& menuButtonBounds,
                        const sf::FloatRect& newTextBounds,
                        const sf::FloatRect& saveTextBounds,
                        const sf::FloatRect& loadTextBounds,
                        const sf::FloatRect& saveAsTextBounds,
                        const sf::FloatRect& dropdownBounds);

    void handleMousePress(const sf::Event::MouseButtonPressed& event,
                        const sf::Vector2f& mousePos,
                        const std::vector<std::shared_ptr<Gate>>& paletteItems,
                        std::vector<std::shared_ptr<Gate>>& placedGates,
                        std::vector<Wire>& wires,
                        const sf::FloatRect& menuButtonBounds,
                        const sf::FloatRect& newTextBounds,
                        const sf::FloatRect& saveTextBounds,
                        const sf::FloatRect& loadTextBounds,
                        const sf::FloatRect& saveAsTextBounds,
                        bool& showDropdown,
                        bool& isSaveModal,
                        bool& isLoadModal,
                        std::string& inputBuffer,
                        std::string& categoryBuffer,
                        std::string& descriptionBuffer,
                        std::string& tagsBuffer,
                        std::string& currentProjectName,
                        std::string& currentCategory,
                        std::string& currentDescription,
                        std::string& currentTags,
                        std::vector<std::string>& categories,
                        int& loadCategorySel,
                        std::vector<WorkbenchProject::ProjectInfo>& projectInfos,
                        size_t& selectedProjectIndex,
                        int& listScroll,
                        std::string& searchBuffer,
                        const sf::Font& font);

    void handleMouseRelease(const sf::Event::MouseButtonReleased& event,
                            const sf::Vector2f& mousePos,
                            std::vector<std::shared_ptr<Gate>>& placedGates,
                            sf::RectangleShape& selectionRect);

    // Keyboard event handlers
    void handleKeyPress(const sf::Event::KeyPressed& event,
                        const sf::Vector2f& mousePos,
                        std::vector<std::shared_ptr<Gate>>& placedGates,
                        std::vector<Wire>& wires,
                        bool& isSaveModal,
                        bool& isLoadModal,
                        bool& showHelp,
                        bool& showConfirmDialog,
                        std::string& inputBuffer,
                        std::string& categoryBuffer,
                        std::string& descriptionBuffer,
                        std::string& tagsBuffer,
                        std::string& currentProjectName,
                        std::string& currentCategory,
                        std::string& currentDescription,
                        std::string& currentTags,
                        std::vector<std::string>& categories,
                        std::vector<WorkbenchProject::ProjectInfo>& projectInfos,
                        size_t& selectedProjectIndex,
                        int& listScroll,
                        std::string& searchBuffer,
                        bool& enableSnapToGrid,
                        bool& isWiring,
                        Pin*& selectedPin,
                        SelectionFilter& currentFilter,
                        std::vector<ComponentGroup>& groups,
                        size_t& selectedGroup,
                        const sf::Font& font);

    void handleTextEntered(const sf::Event::TextEntered& event,
                        bool isSaveModal,
                        bool isLoadModal,
                        std::string& inputBuffer,
                        std::string& categoryBuffer,
                        std::string& descriptionBuffer,
                        std::string& tagsBuffer,
                        std::string& searchBuffer);

    void handleMouseWheel(const sf::Event::MouseWheelScrolled& event,
                        std::vector<WorkbenchProject::ProjectInfo>& projectInfos,
                        int& listScroll);

    // Utility functions
    sf::Vector2f snapToGrid(sf::Vector2f pos, bool enable) const;
    std::shared_ptr<Gate> createGateFromType(GateType type, const sf::Vector2f& pos, const sf::Font& font) const;

    // State access
    InputState& getState() {
        return state;
    }
    const InputState& getState() const {
        return state;
    }

private:
    InputState state;
};
