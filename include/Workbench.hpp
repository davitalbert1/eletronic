#ifndef WORKBENCH_HPP
#define WORKBENCH_HPP

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>
#include <string>
#include "LogicEntities.hpp"
#include "Wire.hpp"

namespace fs = std::filesystem;

// Implementation of Wire::draw and update
inline void Wire::draw(sf::RenderWindow& window) {
    sf::Vector2f start = fromPin->shape.getPosition();
    sf::Vector2f end = toPin->shape.getPosition();
    sf::Color color = fromPin->state ? Theme::WireOn : Theme::WireOff;
    
    sf::Vertex line[] = {
        {start, color},
        {end, color}
    };
    window.draw(line, 2, sf::PrimitiveType::Lines);
}

inline void Wire::update() {
    toPin->state = fromPin->state;
}

class Workbench {
public:
    Workbench(const sf::Font& font) 
        : font(font), menuText(font), saveText(font), loadText(font), newText(font), modalTitle(font), inputText(font), listText(font) {
        setupMenu();
        setupPalette();
        setupModal();
    }

    void newProject() {
        placedGates.clear();
        wires.clear();
        currentProjectName = "";
    }

    void setupMenu() {
        menuBar.setSize({1200, Config::TopMenuHeight});
        menuBar.setFillColor(Theme::TopMenu);

        menuButton.setSize({100, Config::TopMenuHeight - 10});
        menuButton.setPosition({10, 5});
        menuButton.setFillColor(Theme::Panel);
        
        menuText.setFont(font);
        menuText.setString("Menu");
        menuText.setCharacterSize(16);
        menuText.setFillColor(Theme::Text);
        menuText.setPosition({25, 10});

        dropdown.setSize({120, 115});
        dropdown.setFillColor(Theme::Panel);
        dropdown.setOutlineThickness(1);
        dropdown.setOutlineColor(Theme::GateOutline);

        newText.setFont(font); newText.setString("New (Ctrl+N)"); newText.setCharacterSize(14);
        saveText.setFont(font); saveText.setString("Save (Ctrl+S)"); saveText.setCharacterSize(14);
        loadText.setFont(font); loadText.setString("Load (Ctrl+L)"); loadText.setCharacterSize(14);
    }

    void setupPalette() {
        paletteRect.setSize({Config::PaletteWidth, 800});
        paletteRect.setFillColor(Theme::Panel);
        paletteRect.setPosition({0, Config::TopMenuHeight});

        // Icon-like samples in palette
        float startY = Config::TopMenuHeight + 20;
        paletteItems.push_back(std::make_shared<ANDGate>(sf::Vector2f(25, startY), font));
        paletteItems.push_back(std::make_shared<ORGate>(sf::Vector2f(25, startY + 90), font));
        paletteItems.push_back(std::make_shared<NOTGate>(sf::Vector2f(25, startY + 180), font));
        paletteItems.push_back(std::make_shared<ClockGate>(sf::Vector2f(25, startY + 270), font));
        paletteItems.push_back(std::make_shared<PowerSource>(sf::Vector2f(25, startY + 360), font));
    }

    void setupModal() {
        overlay.setSize({1200, 800});
        overlay.setFillColor(Theme::Overlay);

        modalRect.setSize({400, 300});
        modalRect.setFillColor(Theme::ModalBg);
        modalRect.setOutlineThickness(2);
        modalRect.setOutlineColor(Theme::ModalBorder);
        modalRect.setPosition({400, 250});

        modalTitle.setCharacterSize(20);
        modalTitle.setFillColor(Theme::Highlight);
        
        inputText.setCharacterSize(18);
        inputText.setFillColor(Theme::Text);

        listText.setCharacterSize(16);
        listText.setFillColor(Theme::Text);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(paletteRect);
        for (auto& item : paletteItems) item->draw(window);

        for (auto& wire : wires) wire.draw(window);
        for (auto& gate : placedGates) gate->draw(window);

        // Wiring preview
        if (isWiring && selectedPin) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            sf::Vertex line[] = {
                {selectedPin->shape.getPosition(), Theme::Accent},
                {mousePosF, Theme::Accent}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }

        // Selection box preview
        if (isSelecting) {
            window.draw(selectionRect);
        }

        window.draw(menuBar);
        window.draw(menuButton);
        window.draw(menuText);

        if (showDropdown) {
            dropdown.setPosition({10, Config::TopMenuHeight});
            window.draw(dropdown);
            newText.setPosition({20, Config::TopMenuHeight + 10});
            saveText.setPosition({20, Config::TopMenuHeight + 45});
            loadText.setPosition({20, Config::TopMenuHeight + 80});
            window.draw(newText);
            window.draw(saveText);
            window.draw(loadText);
        }

        if (isSaveModal || isLoadModal) {
            window.draw(overlay);
            window.draw(modalRect);
            
            modalTitle.setPosition({420, 270});
            modalTitle.setString(isSaveModal ? "Salvar Projeto" : "Carregar Projeto");
            window.draw(modalTitle);

            if (isSaveModal) {
                sf::RectangleShape inputBox;
                inputBox.setSize({360, 40});
                inputBox.setPosition({420, 320});
                inputBox.setFillColor(Theme::GateBody);
                inputBox.setOutlineThickness(1);
                inputBox.setOutlineColor(Theme::Accent);
                window.draw(inputBox);

                inputText.setPosition({430, 330});
                inputText.setString(inputBuffer + "_");
                window.draw(inputText);
            } else {
                // Draw list of projects
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                for (size_t i = 0; i < projectFiles.size(); ++i) {
                    float yPos = 310 + static_cast<float>(i) * 30; // Increased spacing slightly
                    sf::FloatRect rowBounds({415.f, yPos}, {370.f, 28.f});
                    
                    bool isHovered = rowBounds.contains(mousePosF);
                    
                    if (isHovered || i == selectedProjectIndex) {
                        sf::RectangleShape sel;
                        sel.setSize({370, 28});
                        sel.setPosition({415, yPos});
                        sel.setFillColor(sf::Color(255, 255, 255, 30));
                        if (i == selectedProjectIndex) sel.setOutlineThickness(1);
                        sel.setOutlineColor(Theme::Highlight);
                        window.draw(sel);
                        listText.setFillColor(Theme::Highlight);
                    } else {
                        listText.setFillColor(Theme::Text);
                    }

                    listText.setString(projectFiles[i]);
                    listText.setPosition({420, yPos + 2});
                    window.draw(listText);

                    // Draw "Carregar" button on the right of the row
                    sf::RectangleShape btn;
                    btn.setSize({80, 22});
                    btn.setPosition({700, yPos + 3});
                    btn.setFillColor(isHovered ? Theme::Highlight : Theme::Accent);
                    window.draw(btn);

                    sf::Text btnText(font);
                    btnText.setString("Carregar");
                    btnText.setCharacterSize(12);
                    btnText.setFillColor(sf::Color::White);
                    btnText.setPosition({710, yPos + 6});
                    window.draw(btnText);
                }
                if (projectFiles.empty()) {
                    listText.setString("Nenhum projeto encontrado.");
                    listText.setPosition({420, 310});
                    listText.setFillColor(sf::Color(150, 150, 150));
                    window.draw(listText);
                }
            }
        }
    }

    void updatePropagation() {
        if (isSaveModal || isLoadModal) return;

        // Reset non-output pins (basic approach)
        for (auto& gate : placedGates) {
            for (auto& pin : gate->inputs) pin.state = false;
        }

        // Propagate through wires
        for (auto& wire : wires) wire.update();

        // Update each gate logic
        for (auto& gate : placedGates) gate->updateLogic();
    }

    void refreshProjectList() {
        projectFiles.clear();
        if (!fs::exists("projetos")) fs::create_directory("projetos");
        for (const auto& entry : fs::directory_iterator("projetos")) {
            if (entry.path().extension() == ".gate") {
                projectFiles.push_back(entry.path().stem().string());
            }
        }
        selectedProjectIndex = 0;
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        if (isSaveModal) {
            if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
                if (textEntered->unicode < 128) {
                    char c = static_cast<char>(textEntered->unicode);
                    if (c == '\r' || c == '\n') {
                        if (!inputBuffer.empty()) {
                            save(inputBuffer);
                            isSaveModal = false;
                        }
                    } else if (c == '\b') {
                        if (!inputBuffer.empty()) inputBuffer.pop_back();
                    } else if (std::isalnum(c) || c == '_' || c == '-') {
                        inputBuffer += c;
                    }
                }
            }
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) isSaveModal = false;
            }
            return;
        }

        if (isLoadModal) {
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) isLoadModal = false;
                else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    if (!projectFiles.empty()) {
                        load(projectFiles[selectedProjectIndex]);
                        isLoadModal = false;
                    }
                }
                else if (keyPressed->code == sf::Keyboard::Key::Up) {
                    if (selectedProjectIndex > 0) selectedProjectIndex--;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    if (!projectFiles.empty() && selectedProjectIndex < projectFiles.size() - 1) selectedProjectIndex++;
                }
            }
            if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    for (size_t i = 0; i < projectFiles.size(); ++i) {
                        float yPos = 310 + static_cast<float>(i) * 30;
                        sf::FloatRect rowBounds({415.f, yPos}, {370.f, 28.f});
                        if (rowBounds.contains(mousePosF)) {
                            load(projectFiles[i]);
                            isLoadModal = false;
                            return;
                        }
                    }
                }
            }
            return;
        }

        showDropdown = menuBar.getGlobalBounds().contains(mousePosF) || 
                       (showDropdown && dropdown.getGlobalBounds().contains(mousePosF));

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                // Check dropdown interaction
                if (showDropdown) {
                    sf::FloatRect newBounds = newText.getGlobalBounds();
                    sf::FloatRect saveBounds = saveText.getGlobalBounds();
                    sf::FloatRect loadBounds = loadText.getGlobalBounds();
                    if (newBounds.contains(mousePosF)) {
                        newProject();
                        showDropdown = false;
                        return;
                    }
                    if (saveBounds.contains(mousePosF)) {
                        if (!currentProjectName.empty()) {
                            save(currentProjectName);
                        } else {
                            isSaveModal = true;
                            inputBuffer = "";
                        }
                        return;
                    }
                    if (loadBounds.contains(mousePosF)) {
                        isLoadModal = true;
                        refreshProjectList();
                        return;
                    }
                }

                // Check palette
                for (auto& item : paletteItems) {
                    if (item->contains(mousePosF)) {
                        std::shared_ptr<Gate> newGate;
                        switch (item->getType()) {
                            case GateType::AND: newGate = std::make_shared<ANDGate>(mousePosF, font); break;
                            case GateType::OR:  newGate = std::make_shared<ORGate>(mousePosF, font); break;
                            case GateType::NOT: newGate = std::make_shared<NOTGate>(mousePosF, font); break;
                            case GateType::CLOCK: newGate = std::make_shared<ClockGate>(mousePosF, font); break;
                            case GateType::POWER: newGate = std::make_shared<PowerSource>(mousePosF, font); break;
                        }
                        if (newGate) {
                            for (auto& g : placedGates) g->isSelected = false;
                            newGate->isSelected = true;
                            newGate->isDragging = true;
                            newGate->dragOffset = {Config::GateWidth/2.0f, Config::GateHeight/2.0f};
                            placedGates.push_back(newGate);
                        }
                        return;
                    }
                }

                // Check wiring pins
                for (auto& gate : placedGates) {
                    for (auto& pin : gate->outputs) {
                        if (pin.contains(mousePosF)) {
                            isWiring = true;
                            selectedPin = &pin;
                            return;
                        }
                    }
                    if (isWiring && selectedPin) {
                        for (auto& pin : gate->inputs) {
                            if (pin.contains(mousePosF)) {
                                wires.emplace_back(selectedPin, &pin);
                                isWiring = false;
                                selectedPin = nullptr;
                                return;
                            }
                        }
                    }
                }

                // Dragging or Selection
                bool clickedGate = false;
                for (auto it = placedGates.rbegin(); it != placedGates.rend(); ++it) {
                    if ((*it)->contains(mousePosF)) {
                        clickedGate = true;
                        if ((*it)->getType() == GateType::POWER) dynamic_cast<PowerSource*>((*it).get())->toggle();
                        if ((*it)->getType() == GateType::CLOCK) dynamic_cast<ClockGate*>((*it).get())->toggleInterval();
                        
                        // Toggle selection if not already selected
                        if (!(*it)->isSelected) {
                            for (auto& g : placedGates) g->isSelected = false;
                            (*it)->isSelected = true;
                        }

                        // Prepare all selected gates for dragging
                        for (auto& g : placedGates) {
                            if (g->isSelected) {
                                g->isDragging = true;
                                g->dragOffset = mousePosF - g->getPosition();
                            }
                        }
                        break;
                    }
                }

                if (!clickedGate && !isWiring && mousePosF.x > Config::PaletteWidth) {
                    isSelecting = true;
                    selectStart = mousePosF;
                    for (auto& g : placedGates) g->isSelected = false;
                }
                
                if (clickedGate) isWiring = false;
            } 
            else if (mousePressed->button == sf::Mouse::Button::Right) {
                // Delete wire on right click
                for (auto it = wires.begin(); it != wires.end(); ) {
                    if (it->contains(mousePosF)) it = wires.erase(it);
                    else ++it;
                }
            }
        }
        else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseReleased->button == sf::Mouse::Button::Left) {
                for (auto& gate : placedGates) gate->isDragging = false;
                if (isSelecting) {
                    sf::FloatRect selectRect = selectionRect.getGlobalBounds();
                    for (auto& g : placedGates) {
                        sf::FloatRect gBounds(g->getPosition(), {Config::GateWidth, Config::GateHeight});
                        if (selectRect.findIntersection(gBounds)) g->isSelected = true;
                    }
                    isSelecting = false;
                }
            }
        }
        else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
            if (isSelecting) {
                float x = std::min(selectStart.x, mousePosF.x);
                float y = std::min(selectStart.y, mousePosF.y);
                float w = std::abs(selectStart.x - mousePosF.x);
                float h = std::abs(selectStart.y - mousePosF.y);
                selectionRect.setPosition({x, y});
                selectionRect.setSize({w, h});
                selectionRect.setFillColor(sf::Color(0, 120, 215, 40));
                selectionRect.setOutlineColor(Theme::Highlight);
                selectionRect.setOutlineThickness(1);
            }
            for (auto& gate : placedGates) {
                if (gate->isDragging) gate->setPosition(mousePosF - gate->dragOffset);
            }
        }
        else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Delete) {
                // Delete selected gates and their wires
                for (auto it = placedGates.begin(); it != placedGates.end(); ) {
                    if ((*it)->isSelected) {
                        Gate* gatePtr = (*it).get();
                        for (auto wIt = wires.begin(); wIt != wires.end(); ) {
                            if (wIt->fromPin->parent == gatePtr || wIt->toPin->parent == gatePtr)
                                wIt = wires.erase(wIt);
                            else ++wIt;
                        }
                        it = placedGates.erase(it);
                    } else ++it;
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::N && keyPressed->control && !isSaveModal && !isLoadModal) {
                newProject();
            }
            else if (keyPressed->code == sf::Keyboard::Key::S && keyPressed->control && !isSaveModal && !isLoadModal) {
                if (!currentProjectName.empty()) {
                    save(currentProjectName);
                } else {
                    isSaveModal = true;
                    inputBuffer = "";
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::L && keyPressed->control && !isSaveModal && !isLoadModal) {
                isLoadModal = true;
                refreshProjectList();
            }
        }
    }

    void save(const std::string& name) {
        std::ofstream f("projetos/" + name + ".gate");
        if (!f) return;
        f << placedGates.size() << "\n";
        for (auto& g : placedGates) {
            f << (int)g->getType() << " " << g->getPosition().x << " " << g->getPosition().y << " " << g->getExtraData() << "\n";
        }

        // Save wires: (gate_from_idx, pin_from_idx, gate_to_idx, pin_to_idx)
        auto getGateIdx = [&](Gate* g) -> int {
            for (size_t i = 0; i < placedGates.size(); ++i) if (placedGates[i].get() == g) return (int)i;
            return -1;
        };
        auto getPinIdx = [&](Gate* g, Pin* p, bool input) -> int {
            const auto& pins = input ? g->inputs : g->outputs;
            for (size_t i = 0; i < pins.size(); ++i) if (&pins[i] == p) return (int)i;
            return -1;
        };

        f << wires.size() << "\n";
        for (auto& w : wires) {
            int gFrom = getGateIdx(w.fromPin->parent);
            int pFrom = getPinIdx(w.fromPin->parent, w.fromPin, false);
            int gTo = getGateIdx(w.toPin->parent);
            int pTo = getPinIdx(w.toPin->parent, w.toPin, true);
            f << gFrom << " " << pFrom << " " << gTo << " " << pTo << "\n";
        }
        f.close();
        currentProjectName = name;
    }

    void load(const std::string& name) {
        std::ifstream f("projetos/" + name + ".gate");
        if (!f) return;
        placedGates.clear(); wires.clear();
        size_t gateCount; if (!(f >> gateCount)) return;
        while (gateCount--) {
            int t; float x, y, extra; f >> t >> x >> y >> extra;
            sf::Vector2f p(x, y);
            std::shared_ptr<Gate> newGate;
            switch ((GateType)t) {
                case GateType::AND: newGate = std::make_shared<ANDGate>(p, font); break;
                case GateType::OR:  newGate = std::make_shared<ORGate>(p, font); break;
                case GateType::NOT: newGate = std::make_shared<NOTGate>(p, font); break;
                case GateType::CLOCK: newGate = std::make_shared<ClockGate>(p, font); break;
                case GateType::POWER: newGate = std::make_shared<PowerSource>(p, font); break;
            }
            if (newGate) {
                newGate->setExtraData(extra);
                placedGates.push_back(newGate);
            }
        }

        size_t wireCount; 
        if (f >> wireCount) {
            while (wireCount--) {
                int gF, pF, gT, pT;
                f >> gF >> pF >> gT >> pT;
                if (gF >= 0 && gF < (int)placedGates.size() && gT >= 0 && gT < (int)placedGates.size()) {
                    Pin* from = &placedGates[gF]->outputs[pF];
                    Pin* to = &placedGates[gT]->inputs[pT];
                    wires.emplace_back(from, to);
                }
            }
        }
        currentProjectName = name;
    }

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
