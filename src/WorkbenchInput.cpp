#include "WorkbenchInput.h"
#include "Constants.h"
#include "WorkbenchUI.h"
#include <algorithm>

void WorkbenchInput::handleMouseMove(const sf::Event::MouseMoved& event,
                                     const sf::Vector2f& mousePos,
                                     const std::vector<std::shared_ptr<Gate>>& placedGates,
                                     const std::vector<Wire>& wires,
                                     const sf::FloatRect& menuButtonBounds,
                                     const sf::FloatRect& newTextBounds,
                                     const sf::FloatRect& saveTextBounds,
                                     const sf::FloatRect& loadTextBounds,
                                     const sf::FloatRect& saveAsTextBounds,
                                     const sf::FloatRect& dropdownBounds) {
    state.isMenuButtonHovered = menuButtonBounds.contains(mousePos);
    state.isNewHovered = newTextBounds.contains(mousePos);
    state.isSaveHovered = saveTextBounds.contains(mousePos);
    state.isLoadHovered = loadTextBounds.contains(mousePos);
    state.isSaveAsHovered = saveAsTextBounds.contains(mousePos);

    state.showDropdown = menuButtonBounds.contains(mousePos) ||
                        (state.showDropdown && dropdownBounds.contains(mousePos));

    if (state.isSelecting) {
        float x = std::min(state.selectStart.x, mousePos.x);
        float y = std::min(state.selectStart.y, mousePos.y);
        float w = std::abs(state.selectStart.x - mousePos.x);
        float h = std::abs(state.selectStart.y - mousePos.y);
        // selectionRect is handled by Workbench
    }
}

void WorkbenchInput::handleMousePress(const sf::Event::MouseButtonPressed& event,
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
                                      const sf::Font& font) {
    if (event.button == sf::Mouse::Button::Left) {
        state.isMenuButtonPressed = menuButtonBounds.contains(mousePos);

        if (state.showDropdown) {
            if (newTextBounds.contains(mousePos)) {
                // New project
                placedGates.clear();
                wires.clear();
                currentProjectName = "";
                currentCategory = "";
                currentDescription = "";
                currentTags = "";
                showDropdown = false;
                return;
            }
            if (saveTextBounds.contains(mousePos)) {
                if (!currentProjectName.empty()) {
                    // Save current project
                    isSaveModal = true;
                    inputBuffer = currentProjectName;
                    categoryBuffer = currentCategory;
                    descriptionBuffer = currentDescription;
                    tagsBuffer = currentTags;
                    state.modalFocus = InputState::ModalFocus::NAME;
                } else {
                    isSaveModal = true;
                    inputBuffer = "";
                    categoryBuffer = currentCategory;
                    descriptionBuffer = currentDescription;
                    tagsBuffer = currentTags;
                    state.modalFocus = InputState::ModalFocus::NAME;
                }
                showDropdown = false;
                return;
            }
            if (saveAsTextBounds.contains(mousePos)) {
                isSaveModal = true;
                inputBuffer = currentProjectName;
                categoryBuffer = currentCategory;
                descriptionBuffer = currentDescription;
                tagsBuffer = currentTags;
                state.modalFocus = InputState::ModalFocus::NAME;
                showDropdown = false;
                return;
            }
            if (loadTextBounds.contains(mousePos)) {
                isLoadModal = true;
                loadCategorySel = 0;
                // Refresh project list would be called by Workbench
                showDropdown = false;
                return;
            }
        }

        // Check palette items
        for (auto& item : paletteItems) {
            if (item->contains(mousePos)) {
                std::shared_ptr<Gate> newGate;
                switch (item->getType()) {
                    case GateType::AND:
                        newGate = std::make_shared<ANDGate>(mousePos, font);
                        break;
                    case GateType::OR:
                        newGate = std::make_shared<ORGate>(mousePos, font);
                        break;
                    case GateType::NOT:
                        newGate = std::make_shared<NOTGate>(mousePos, font);
                        break;
                    case GateType::XOR:
                        newGate = std::make_shared<XORGate>(mousePos, font);
                        break;
                    case GateType::NAND:
                        newGate = std::make_shared<NANDGate>(mousePos, font);
                        break;
                    case GateType::NOR:
                        newGate = std::make_shared<NORGate>(mousePos, font);
                        break;
                    case GateType::XNOR:
                        newGate = std::make_shared<XNORGate>(mousePos, font);
                        break;
                    case GateType::CLOCK:
                        newGate = std::make_shared<ClockGate>(mousePos, font);
                        break;
                    case GateType::POWER:
                        newGate = std::make_shared<PowerSource>(mousePos, font);
                        break;
                    case GateType::SWITCH:
                        newGate = std::make_shared<Switch>(mousePos, font);
                        break;
                    case GateType::BUTTON:
                        newGate = std::make_shared<Button>(mousePos, font);
                        break;
                    case GateType::LED:
                        newGate = std::make_shared<LED>(mousePos, font);
                        break;
                    case GateType::ROM:
                        newGate = std::make_shared<ROM>(mousePos, font);
                        break;
                    case GateType::SEG7:
                        newGate = std::make_shared<Seg7Display>(mousePos, font);
                        break;
                    case GateType::LCD:
                        newGate = std::make_shared<LCDDisplay>(mousePos, font);
                        break;
                    case GateType::COUNTER:
                        newGate = std::make_shared<Counter>(mousePos, font);
                        break;
                    case GateType::REGISTER:
                        newGate = std::make_shared<Register>(mousePos, font);
                        break;
                }
                if (newGate) {
                    for (auto& g : placedGates) g->isSelected = false;
                    newGate->isSelected = true;
                    newGate->isDragging = true;
                    newGate->dragOffset = {Config::GateWidth / 2.0f, Config::GateHeight / 2.0f};
                    placedGates.push_back(newGate);
                }
                return;
            }
        }

        // Wiring
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
            state.isMultiSelecting = true;
        }

        for (auto& gate : placedGates) {
            for (auto& pin : gate->outputs) {
                if (pin.contains(mousePos)) {
                    state.isWiring = true;
                    state.selectedPin = &pin;
                    state.wireStartPos = state.selectedPin->shape.getPosition();
                    return;
                }
            }
            if (state.isWiring && state.selectedPin) {
                for (auto& pin : gate->inputs) {
                    if (pin.contains(mousePos)) {
                        wires.emplace_back(state.selectedPin, &pin);
                        state.isWiring = false;
                        state.selectedPin = nullptr;
                        return;
                    }
                }
            }
        }

        // Gate selection
        bool clickedGate = false;
        for (auto it = placedGates.rbegin(); it != placedGates.rend(); ++it) {
            if ((*it)->contains(mousePos)) {
                clickedGate = true;
                (*it)->interact();

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                    (*it)->isSelected = !(*it)->isSelected;
                } else {
                    if (!(*it)->isSelected) {
                        for (auto& g : placedGates) g->isSelected = false;
                        (*it)->isSelected = true;
                    }
                }

                for (auto& g : placedGates) {
                    if (g->isSelected) {
                        g->isDragging = true;
                        g->dragOffset = mousePos - g->getPosition();
                    }
                }
                break;
            }
        }

        // Wire selection
        bool clickedWire = false;
        if (!clickedGate && !state.isWiring && mousePos.x > Config::PaletteWidth) {
            for (auto& wire : wires) {
                if (wire.contains(mousePos)) {
                    clickedWire = true;
                    for (auto& w : wires) w.isSelected = false;
                    for (auto& g : placedGates) g->isSelected = false;
                    wire.isSelected = true;
                    break;
                }
            }
        }

        // Selection rectangle
        if (!clickedGate && !clickedWire && !state.isWiring && mousePos.x > Config::PaletteWidth) {
            state.isSelecting = true;
            state.selectStart = mousePos;
            for (auto& g : placedGates) g->isSelected = false;
            for (auto& w : wires) w.isSelected = false;
        }

        if (clickedGate) state.isWiring = false;
    }
    else if (event.button == sf::Mouse::Button::Right) {
        if (state.isWiring) {
            state.isWiring = false;
            state.selectedPin = nullptr;
        } else {
            for (auto& wire : wires) {
                if (wire.contains(mousePos)) wire.isSelected = false;
            }
        }
    }
}

void WorkbenchInput::handleMouseRelease(const sf::Event::MouseButtonReleased& event,
                                        const sf::Vector2f& mousePos,
                                        std::vector<std::shared_ptr<Gate>>& placedGates,
                                        sf::RectangleShape& selectionRect) {
    if (event.button == sf::Mouse::Button::Left) {
        for (auto& gate : placedGates) gate->isDragging = false;
        if (state.isSelecting) {
            sf::FloatRect selectRect = selectionRect.getGlobalBounds();
            for (auto& g : placedGates) {
                sf::FloatRect gBounds(g->getPosition(), {Config::GateWidth, Config::GateHeight});
                if (selectRect.findIntersection(gBounds)) g->isSelected = true;
            }
            state.isSelecting = false;
        }
    }
}

void WorkbenchInput::handleKeyPress(const sf::Event::KeyPressed& event,
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
                                    const sf::Font& font) {
    if (isSaveModal || isLoadModal) return;

    if (event.code == sf::Keyboard::Key::Delete) {
        for (auto it = placedGates.begin(); it != placedGates.end(); ) {
            if ((*it)->isSelected) {
                Gate* gatePtr = (*it).get();
                for (auto wIt = wires.begin(); wIt != wires.end(); ) {
                    if (wIt->fromPin->parent == gatePtr || wIt->toPin->parent == gatePtr) {
                        wIt = wires.erase(wIt);
                    } else {
                        ++wIt;
                    }
                }
                it = placedGates.erase(it);
            } else {
                ++it;
            }
        }
    } else if (event.code == sf::Keyboard::Key::D && event.control) {
        std::vector<std::shared_ptr<Gate>> newGates;
        for (auto& gate : placedGates) {
            if (gate->isSelected) {
                std::shared_ptr<Gate> newGate;
                switch (gate->getType()) {
                    case GateType::AND:
                        newGate = std::make_shared<ANDGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::OR:
                        newGate = std::make_shared<ORGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::NOT:
                        newGate = std::make_shared<NOTGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::XOR:
                        newGate = std::make_shared<XORGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::NAND:
                        newGate = std::make_shared<NANDGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::NOR:
                        newGate = std::make_shared<NORGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::XNOR:
                        newGate = std::make_shared<XNORGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::CLOCK:
                        newGate = std::make_shared<ClockGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::POWER:
                        newGate = std::make_shared<PowerSource>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::SWITCH:
                        newGate = std::make_shared<Switch>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::BUTTON:
                        newGate = std::make_shared<Button>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::LED:
                        newGate = std::make_shared<LED>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::ROM:
                        newGate = std::make_shared<ROM>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::SEG7:
                        newGate = std::make_shared<Seg7Display>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::LCD:
                        newGate = std::make_shared<LCDDisplay>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::COUNTER:
                        newGate = std::make_shared<Counter>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                    case GateType::REGISTER:
                        newGate = std::make_shared<Register>(gate->getPosition() + sf::Vector2f(20, 20), font);
                        break;
                }
                if (newGate) {
                    newGate->setRotation(gate->getRotation());
                    newGate->isSelected = true;
                    newGates.push_back(newGate);
                }
            }
        }
        for (auto& g : placedGates) g->isSelected = false;
        for (auto& g : newGates) {
            g->isSelected = true;
            placedGates.push_back(g);
        }
    } else if (event.code == sf::Keyboard::Key::A && event.control) {
        for (auto& g : placedGates) g->isSelected = true;
        for (auto& w : wires) w.isSelected = true;
    } else if (event.code == sf::Keyboard::Key::T && event.control) {
        int filterInt = static_cast<int>(currentFilter);
        filterInt = (filterInt + 1) % 8;
        currentFilter = static_cast<SelectionFilter>(filterInt);

        for (auto& g : placedGates) {
            g->isSelected = false;
            switch (currentFilter) {
                case SelectionFilter::ALL:
                case SelectionFilter::GATES_ONLY:
                    g->isSelected = true;
                    break;
                case SelectionFilter::AND_GATES:
                    if (g->getType() == GateType::AND) g->isSelected = true;
                    break;
                case SelectionFilter::OR_GATES:
                    if (g->getType() == GateType::OR) g->isSelected = true;
                    break;
                case SelectionFilter::NOT_GATES:
                    if (g->getType() == GateType::NOT) g->isSelected = true;
                    break;
                case SelectionFilter::CLOCK_GATES:
                    if (g->getType() == GateType::CLOCK) g->isSelected = true;
                    break;
                case SelectionFilter::POWER_SOURCES:
                    if (g->getType() == GateType::POWER) g->isSelected = true;
                    break;
                default:
                    break;
            }
        }
        if (currentFilter == SelectionFilter::WIRES_ONLY) {
            for (auto& w : wires) w.isSelected = true;
        } else {
            for (auto& w : wires) w.isSelected = false;
        }
    } else if (event.code == sf::Keyboard::Key::G && event.control) {
        if (!placedGates.empty() || !wires.empty()) {
            ComponentGroup newGroup;
            newGroup.name = "Group " + std::to_string(groups.size() + 1);

            sf::Vector2f center(0, 0);
            int count = 0;
            for (auto& g : placedGates) {
                if (g->isSelected) {
                    newGroup.gates.push_back(g);
                    center += g->getPosition();
                    count++;
                }
            }
            if (count > 0) {
                newGroup.center = center / static_cast<float>(count);
                groups.push_back(newGroup);
            }
        }
    } else if (event.code == sf::Keyboard::Key::U && event.control) {
        if (selectedGroup < groups.size()) {
            groups.erase(groups.begin() + selectedGroup);
            selectedGroup = SIZE_MAX;
        }
    } else if (event.code == sf::Keyboard::Key::F1 || event.code == sf::Keyboard::Key::Slash) {
        if (!showConfirmDialog) {
            showHelp = !showHelp;
        }
    } else if (event.code == sf::Keyboard::Key::R) {
        if (!placedGates.empty()) {
            for (auto& gate : placedGates) {
                if (gate->isSelected) {
                    float newRot = gate->getRotation() + Config::RotationStep;
                    gate->setRotation(newRot);
                }
            }
        }
    } else if (event.code == sf::Keyboard::Key::G && !event.control) {
        enableSnapToGrid = !enableSnapToGrid;
    } else if (event.code == sf::Keyboard::Key::Left ||
               event.code == sf::Keyboard::Key::Right ||
               event.code == sf::Keyboard::Key::Up ||
               event.code == sf::Keyboard::Key::Down) {
        if (!placedGates.empty() && !event.control) {
            sf::Vector2f delta(0, 0);
            float moveSpeed = 1.0f;

            if (event.code == sf::Keyboard::Key::Left) delta.x = -moveSpeed;
            else if (event.code == sf::Keyboard::Key::Right) delta.x = moveSpeed;
            else if (event.code == sf::Keyboard::Key::Up) delta.y = -moveSpeed;
            else if (event.code == sf::Keyboard::Key::Down) delta.y = moveSpeed;

            bool hasSelection = false;
            for (auto& gate : placedGates) {
                if (gate->isSelected) {
                    hasSelection = true;
                    break;
                }
            }

            if (hasSelection) {
                for (auto& gate : placedGates) {
                    if (gate->isSelected) {
                        sf::Vector2f newPos = gate->getPosition() + delta;
                        if (enableSnapToGrid) newPos = snapToGrid(newPos, enableSnapToGrid);
                        gate->setPosition(newPos);
                    }
                }
            }
        }
    }
}

void WorkbenchInput::handleTextEntered(const sf::Event::TextEntered& event,
                                       bool isSaveModal,
                                       bool isLoadModal,
                                       std::string& inputBuffer,
                                       std::string& categoryBuffer,
                                       std::string& descriptionBuffer,
                                       std::string& tagsBuffer,
                                       std::string& searchBuffer) {
    if (event.unicode < 128) {
        char c = static_cast<char>(event.unicode);

        if (isSaveModal) {
            std::string* buf = nullptr;
            if (state.modalFocus == InputState::ModalFocus::NAME) buf = &inputBuffer;
            else if (state.modalFocus == InputState::ModalFocus::CATEGORY) buf = &categoryBuffer;
            else if (state.modalFocus == InputState::ModalFocus::DESCRIPTION) buf = &descriptionBuffer;
            else if (state.modalFocus == InputState::ModalFocus::TAGS) buf = &tagsBuffer;

            if (c == '\b') {
                if (buf && !buf->empty()) buf->pop_back();
            } else if (std::isalnum(c) || c == '_' || c == '-' || c == ' ' || c == ',' || c == '.') {
                if (buf && buf->size() < 200) *buf += c;
            }
        } else if (isLoadModal && state.modalFocus == InputState::ModalFocus::SEARCH) {
            if (c == '\b') {
                if (!searchBuffer.empty()) searchBuffer.pop_back();
            } else if (std::isalnum(c) || c == ' ' || c == '_' || c == '-') {
                if (searchBuffer.size() < 50) searchBuffer += c;
            }
        }
    }
}

void WorkbenchInput::handleMouseWheel(const sf::Event::MouseWheelScrolled& event,
                                      std::vector<WorkbenchProject::ProjectInfo>& projectInfos,
                                      int& listScroll) {
    if (event.wheel == sf::Mouse::Wheel::Vertical && !projectInfos.empty()) {
        static constexpr int LoadVisibleRows = 6;
        int maxScroll = std::max(0, (int)projectInfos.size() - LoadVisibleRows);
        listScroll = std::clamp(listScroll + (event.delta > 0 ? -1 : 1), 0, maxScroll);
    }
}

sf::Vector2f WorkbenchInput::snapToGrid(sf::Vector2f pos, bool enable) const {
    if (!enable) return pos;
    return sf::Vector2f(
        std::round(pos.x / Config::GridSize) * Config::GridSize,
        std::round(pos.y / Config::GridSize) * Config::GridSize
    );
}

std::shared_ptr<Gate> WorkbenchInput::createGateFromType(GateType type, const sf::Vector2f& pos, const sf::Font& font) const {
    switch (type) {
        case GateType::AND: return std::make_shared<ANDGate>(pos, font);
        case GateType::OR: return std::make_shared<ORGate>(pos, font);
        case GateType::NOT: return std::make_shared<NOTGate>(pos, font);
        case GateType::XOR: return std::make_shared<XORGate>(pos, font);
        case GateType::NAND: return std::make_shared<NANDGate>(pos, font);
        case GateType::NOR: return std::make_shared<NORGate>(pos, font);
        case GateType::XNOR: return std::make_shared<XNORGate>(pos, font);
        case GateType::CLOCK: return std::make_shared<ClockGate>(pos, font);
        case GateType::POWER: return std::make_shared<PowerSource>(pos, font);
        case GateType::SWITCH: return std::make_shared<Switch>(pos, font);
        case GateType::BUTTON: return std::make_shared<Button>(pos, font);
        case GateType::LED: return std::make_shared<LED>(pos, font);
        case GateType::ROM: return std::make_shared<ROM>(pos, font);
        case GateType::SEG7: return std::make_shared<Seg7Display>(pos, font);
        case GateType::LCD: return std::make_shared<LCDDisplay>(pos, font);
        case GateType::COUNTER: return std::make_shared<Counter>(pos, font);
        case GateType::REGISTER: return std::make_shared<Register>(pos, font);
        default: return nullptr;
    }
}
