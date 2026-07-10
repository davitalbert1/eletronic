#ifndef WORKBENCH_HPP
#define WORKBENCH_HPP

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include "LogicEntities.h"
#include "Wire.h"

namespace fs = std::filesystem;

class Workbench {
public:
    Workbench(const sf::Font& font);

    void newProject();
    void setupMenu();
    void setupPalette();
    void setupModal();
    void setupTooltip();
    void draw(sf::RenderWindow& window);
    void renderImGui();
    void update(float deltaTime);
    void updatePropagation();
    void refreshProjectList();
    void refreshCategories();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void save(const std::string& name);
    void load(const std::string& name);

    // Modal layout geometry (shared between draw and handleEvent)
    static constexpr float ModalX = 370.f, ModalY = 190.f, ModalW = 460.f, ModalH = 440.f;
    static constexpr float ChipW = 100.f, ChipH = 24.f, ChipGap = 8.f;

    // Project metadata
    struct ProjectInfo {
        std::string name; // file stem (without extension)
        std::string category; // subfolder name, "" for root
        fs::path fullPath;  // absolute path to .gate file
        std::uintmax_t size = 0; // file size in bytes
        std::filesystem::file_time_type modified; // last modification time
        std::string createdStr; // creation time (formatted), "" = unknown
        size_t gateCount = 0;
        size_t wireCount = 0;
    };

    // Modal focus targets (save modal text fields)
    enum class ModalFocus { NAME, CATEGORY };

private:
    const sf::Font& font;
    sf::RectangleShape paletteRect;
    std::vector<std::shared_ptr<Gate>> paletteItems;
    std::vector<std::shared_ptr<Gate>> placedGates;
    std::vector<Wire> wires;

    // UI elements
    sf::RectangleShape menuBar, menuButton, dropdown;
    sf::Text menuText, saveText, loadText, newText, saveAsText;
    bool showDropdown = false;
    bool isMenuButtonHovered = false;
    bool isMenuButtonPressed = false;
    bool isNewHovered = false;
    bool isSaveHovered = false;
    bool isLoadHovered = false;
    bool isSaveAsHovered = false;

    // Modal UI
    bool isSaveModal = false;
    bool isLoadModal = false;
    sf::RectangleShape overlay, modalRect;
    sf::Text modalTitle, inputText, listText;
    std::string inputBuffer;
    std::string currentProjectName;
    std::vector<ProjectInfo> projectInfos;
    size_t selectedProjectIndex = 0;

    // Categories / folders
    std::vector<std::string> categories;
    std::string currentCategory;
    std::string categoryBuffer;
    ModalFocus modalFocus = ModalFocus::NAME;
    bool saveAsMode = false;
    int listScroll = 0;
    int loadCategorySel = 0;

    // Status / feedback message (e.g. full saved path)
    std::string statusMessage;
    float statusMessageTimer = 0.0f;

    // Automatic backup
    float backupTimer = 0.0f;
    bool autoBackupEnabled = true;
    static constexpr float BackupInterval = 300.0f; // seconds (5 minutes)

    // Progress indicators
    bool isSaving = false;
    bool isLoading = false;
    float saveProgress = 0.0f;
    float loadProgress = 0.0f;
    sf::RectangleShape progressBar;
    sf::Text progressText;

    // Tooltip system
    std::string currentTooltip;
    sf::Text tooltipText;
    sf::RectangleShape tooltipBg;
    float tooltipTimer = 0.0f;
    static constexpr float TooltipDelay = 0.5f;  // seconds before showing tooltip

    // Animation/transition system
    float menuButtonAlpha = 255.0f;
    float dropdownAlpha = 255.0f;
    float modalAlpha = 255.0f;
    bool isAnimating = false;

    bool isWiring = false;
    Pin* selectedPin = nullptr;
    sf::Vector2f wireStartPos;  // For visual feedback during wiring

    bool isSelecting = false;
    sf::Vector2f selectStart;
    sf::RectangleShape selectionRect;

    std::vector<bool> loadButtonHovered; // Hover states for load modal buttons

    // Mouse position for hover detection
    sf::Vector2f lastMousePos;

    // Rotation system
    float selectedRotation = 0.0f;
    bool isRotating = false;

    // Snap-to-grid
    static constexpr bool enableSnapToGrid = true;
    sf::Vector2f snapToGrid(sf::Vector2f pos);

    // Undo/Redo system
    struct StateSnapshot {
        std::vector<std::shared_ptr<Gate>> gates;
        std::vector<Wire> wires;
    };
    std::vector<StateSnapshot> undoStack;
    std::vector<StateSnapshot> redoStack;
    static constexpr size_t MaxUndoSteps = 50;
    void saveState();
    void undo();
    void redo();

    // Multi-selection with Shift
    bool isMultiSelecting = false;

    // Selection by type
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
    SelectionFilter currentFilter = SelectionFilter::ALL;
    std::string getFilterName() const;
    
    // Component grouping
    struct ComponentGroup {
        std::vector<std::shared_ptr<Gate>> gates;
        std::vector<size_t> wireIndices;
        sf::Vector2f center;
        std::string name;
    };
    std::vector<ComponentGroup> groups;
    size_t selectedGroup = SIZE_MAX;

    // Save/Load helpers
    ProjectInfo readProjectInfo(const fs::path& p, const std::string& category);
    void writeProject(const fs::path& p);
    void deleteProject(size_t index);
    void exportJSON(const std::string& baseName, const std::string& category);
    void exportXML(const std::string& baseName, const std::string& category);
    void createBackup();
    std::string formatFileTime(std::filesystem::file_time_type t) const;
    std::string formatSize(std::uintmax_t bytes) const;
    fs::path projectDir(const std::string& category) const;

    static sf::FloatRect chipRect(size_t i, float startY);
    sf::FloatRect nameInputRect() const;
    sf::FloatRect categoryInputRect() const;
    sf::FloatRect saveButtonRect() const;
    sf::FloatRect exportJsonRect() const;
    sf::FloatRect exportXmlRect() const;
    sf::FloatRect loadRowRect(size_t i) const;
    sf::FloatRect loadRowLoadBtnRect(size_t i) const;
    sf::FloatRect loadRowDeleteBtnRect(size_t i) const;
};

#endif
