#ifndef WORKBENCH_HPP
#define WORKBENCH_HPP

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>
#include <string>
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
    void draw(sf::RenderWindow& window);
    void renderImGui();
    void updatePropagation();
    void refreshProjectList();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void save(const std::string& name);
    void load(const std::string& name);

private:
    const sf::Font& font;
    sf::RectangleShape paletteRect;
    std::vector<std::shared_ptr<Gate>> paletteItems;
    std::vector<std::shared_ptr<Gate>> placedGates;
    std::vector<Wire> wires;

    // UI elements
    sf::RectangleShape menuBar, menuButton, dropdown;
    sf::Text menuText, saveText, loadText, newText;
    bool showDropdown = false;

    // Modal UI
    bool isSaveModal = false;
    bool isLoadModal = false;
    sf::RectangleShape overlay, modalRect;
    sf::Text modalTitle, inputText, listText;
    std::string inputBuffer;
    std::string currentProjectName;
    std::vector<std::string> projectFiles;
    size_t selectedProjectIndex = 0;

    bool isWiring = false;
    Pin* selectedPin = nullptr;

    bool isSelecting = false;
    sf::Vector2f selectStart;
    sf::RectangleShape selectionRect;
};

#endif