#include "Workbench.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <algorithm>
#include <cstdint>
#include <windows.h>
#include <ctime>
#include <chrono>
#include <sstream>

namespace fs = std::filesystem;

Workbench::Workbench(const sf::Font& font)
    : font_(font)
    , ui_(font)
    , undoRedoManager_(std::make_unique<UndoRedoManager>(50))
    , confirmationDialog_(std::make_unique<ConfirmationDialog>()) {
    refreshChipPalette();
    refreshCategories();
}

void Workbench::newProject() {
    placedGates_.clear();
    wires_.clear();
    currentProjectName_ = "";
    currentCategory_ = "";
    currentDescription_ = "";
    currentTags_ = "";
    if (undoRedoManager_) undoRedoManager_->clear();
}

// ---------------------------------------------------------------------------
// Chips personalizados
// ---------------------------------------------------------------------------

void Workbench::refreshChipPalette() {
    // Reconstrói a paleta: componentes fixos + InputPin/OutputPin + chips.
    paletteItems_.clear();
    chipPaletteIds_.clear();

    auto add = [&](std::shared_ptr<Gate> g) { paletteItems_.push_back(g); };

    add(std::make_shared<ANDGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<ORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<NOTGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<XORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<NANDGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<NORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<XNORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<ClockGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<PowerSource>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Switch>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Button>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<LED>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<ROM>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Seg7Display>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<LCDDisplay>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Counter>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Register>(sf::Vector2f(0, 0), font_));
    // Marcadores de pino (necessários para criar chips)
    add(std::make_shared<InputPin>(sf::Vector2f(0, 0), font_, "IN"));
    add(std::make_shared<OutputPin>(sf::Vector2f(0, 0), font_, "OUT"));

    // Chips registrados na biblioteca
    for (auto& def : chip::ChipLibrary::instance().all()) {
        auto inst = chip::createChipInstance(def, sf::Vector2f(0, 0), font_);
        if (inst) {
            paletteItems_.push_back(inst);
            chipPaletteIds_.push_back(def->id);
        }
    }
    updatePalettePositions();
}

bool Workbench::saveCurrentAsChip(const std::string& id, const std::string& name,
                                  const std::string& description, const std::string& category,
                                  bool askUpdate) {
    // 1) Clona o circuito atual (gates + wires) para a definição.
    auto circuit = std::make_shared<chip::Circuit>();
    circuit->gates.reserve(placedGates_.size());
    for (auto& g : placedGates_) circuit->gates.push_back(g->clone(font_));
    // Reconecta fios usando índices estáveis.
    for (auto& w : wires_) {
        int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
        for (size_t i = 0; i < placedGates_.size(); ++i) {
            if (placedGates_[i].get() == w.fromPin->parent) gFrom = (int)i;
            if (placedGates_[i].get() == w.toPin->parent)   gTo   = (int)i;
        }
        for (size_t i = 0; i < w.fromPin->parent->outputs.size(); ++i)
            if (&w.fromPin->parent->outputs[i] == w.fromPin) pFrom = (int)i;
        for (size_t i = 0; i < w.toPin->parent->inputs.size(); ++i)
            if (&w.toPin->parent->inputs[i] == w.toPin) pTo = (int)i;
        if (gFrom >= 0 && gTo >= 0 && pFrom >= 0 && pTo >= 0)
            circuit->wires.emplace_back(&circuit->gates[gFrom]->outputs[pFrom],
                                        &circuit->gates[gTo]->inputs[pTo]);
    }

    // 2) Cria a definição e calcula pinos/dependências.
    auto def = std::make_shared<chip::ChipDefinition>();
    def->id = id;
    def->name = name;
    def->description = description;
    def->category = category;
    def->version = 1;
    def->circuit = circuit;
    def->refreshFromCircuit();

    if (!def->isValid()) {
        statusMessage_ = "O circuito precisa de ao menos 1 Entrada e 1 Saida.";
        statusMessageTimer_ = 5.0f;
        return false;
    }

    // 3) Proteção contra dependência circular.
    if (!chip::ChipLibrary::instance().registerChip(def)) {
        statusMessage_ = "Erro: isso criaria uma dependencia circular (loop).";
        statusMessageTimer_ = 5.0f;
        return false;
    }

    // 4) Persiste o arquivo .chip para uso futuro.
    fs::path dir = "projetos/chips";
    fs::create_directories(dir);
    chip::saveChipDefinition(def, (dir / (id + ".chip")).string());

    refreshChipPalette();
    statusMessage_ = "Chip '" + name + "' criado e adicionado a biblioteca.";
    statusMessageTimer_ = 4.0f;
    return true;
}

void Workbench::updateChipFromCurrent(const std::string& id) {
    auto def = chip::ChipLibrary::instance().findById(id);
    if (!def) return;

    // Reconstrói o circuito interno a partir do atual.
    auto circuit = std::make_shared<chip::Circuit>();
    circuit->gates.reserve(placedGates_.size());
    for (auto& g : placedGates_) circuit->gates.push_back(g->clone(font_));
    for (auto& w : wires_) {
        int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
        for (size_t i = 0; i < placedGates_.size(); ++i) {
            if (placedGates_[i].get() == w.fromPin->parent) gFrom = (int)i;
            if (placedGates_[i].get() == w.toPin->parent)   gTo   = (int)i;
        }
        for (size_t i = 0; i < w.fromPin->parent->outputs.size(); ++i)
            if (&w.fromPin->parent->outputs[i] == w.fromPin) pFrom = (int)i;
        for (size_t i = 0; i < w.toPin->parent->inputs.size(); ++i)
            if (&w.toPin->parent->inputs[i] == w.toPin) pTo = (int)i;
        if (gFrom >= 0 && gTo >= 0 && pFrom >= 0 && pTo >= 0)
            circuit->wires.emplace_back(&circuit->gates[gFrom]->outputs[pFrom],
                                        &circuit->gates[gTo]->inputs[pTo]);
    }
    def->circuit = circuit;
    def->refreshFromCircuit();
    def->version++;

    // Atualiza todas as instâncias ativas (rebuild do circuito interno).
    for (auto& g : placedGates_) {
        if (g->getType() == GateType::CHIP && g->getChipId() == id) {
            auto* chip = static_cast<CustomChip*>(g.get());
            chip->rebuild(font_);
        }
    }
    chip::saveChipDefinition(def, ("projetos/chips/" + id + ".chip"));
    statusMessage_ = "Chip '" + def->name + "' atualizado (v" + std::to_string(def->version) + ").";
    statusMessageTimer_ = 4.0f;
}

void Workbench::saveState() {
    if (undoRedoManager_) undoRedoManager_->saveState(placedGates_, wires_);
}

void Workbench::undo() {
    if (undoRedoManager_) undoRedoManager_->undo(placedGates_, wires_);
}

void Workbench::redo() {
    if (undoRedoManager_) undoRedoManager_->redo(placedGates_, wires_);
}

sf::Vector2f Workbench::snapToGrid(sf::Vector2f pos) {
    if (!enableSnapToGrid_) return pos;
    return sf::Vector2f(
        std::round(pos.x / Config::GridSize) * Config::GridSize,
        std::round(pos.y / Config::GridSize) * Config::GridSize
    );
}

void Workbench::setupPalette() {
    const float x = 25.0f;
    size_t i = 0;
    auto add = [&](std::shared_ptr<Gate> g) {
        paletteItems_.push_back(g);
        (void)i;
    };
    add(std::make_shared<ANDGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<ORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<NOTGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<XORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<NANDGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<NORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<XNORGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<ClockGate>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<PowerSource>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Switch>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Button>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<LED>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<ROM>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Seg7Display>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<LCDDisplay>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Counter>(sf::Vector2f(0, 0), font_));
    add(std::make_shared<Register>(sf::Vector2f(0, 0), font_));
    updatePalettePositions();
}

void Workbench::updatePalettePositions() {
    const float x = 25.0f;
    float maxScroll = std::max(0.0f, PaletteTop + PaletteItemStep * (float)paletteItems_.size() - PaletteViewBottom);
    paletteScroll_ = std::clamp(paletteScroll_, 0.0f, maxScroll);
    for (size_t i = 0; i < paletteItems_.size(); ++i) {
        paletteItems_[i]->setPosition({x, PaletteTop + PaletteItemStep * (float)i - paletteScroll_});
    }
}

void Workbench::draw(sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    // Draw palette background
    ui_.drawPalette(window);

    // Clip palette items to the palette area
    sf::View paletteView(sf::FloatRect({0, Config::TopMenuHeight},
                                       {Config::PaletteWidth, PaletteViewBottom - Config::TopMenuHeight}));
    window.setView(paletteView);
    for (auto& item : paletteItems_) item->draw(window);
    window.setView(window.getDefaultView());

    // Draw wires
    for (auto& wire : wires_) wire.draw(window);

    // Draw gates
    for (auto& gate : placedGates_) gate->draw(window);

    // Draw wiring guide
    if (input_.getState().isWiring && input_.getState().selectedPin) {
        sf::Vertex line[] = {
            {input_.getState().wireStartPos, Theme::Accent},
            {mousePosF, Theme::Accent}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);

        sf::CircleShape guideCircle(8.0f);
        guideCircle.setFillColor(sf::Color::Transparent);
        guideCircle.setOutlineColor(Theme::Highlight);
        guideCircle.setOutlineThickness(2.0f);
        guideCircle.setPosition({mousePosF.x - 8.0f, mousePosF.y - 8.0f});
        window.draw(guideCircle);
    }

    // Draw selection rectangle
    if (input_.getState().isSelecting) {
        // selectionRect would be drawn here
    }

    // Draw groups
    for (size_t i = 0; i < groups_.size(); ++i) {
        const auto& group = groups_[i];
        if (!group.gates.empty()) {
            sf::Vector2f minPos = group.gates[0]->getPosition();
            sf::Vector2f maxPos = minPos + sf::Vector2f(Config::GateWidth, Config::GateHeight);

            for (auto& gate : group.gates) {
                sf::Vector2f pos = gate->getPosition();
                minPos = sf::Vector2f(std::min(minPos.x, pos.x), std::min(minPos.y, pos.y));
                maxPos = sf::Vector2f(std::max(maxPos.x, pos.x + Config::GateWidth),
                    std::max(maxPos.y, pos.y + Config::GateHeight));
            }

            sf::RectangleShape groupBox;
            groupBox.setPosition(minPos - sf::Vector2f(10, 10));
            groupBox.setSize(maxPos - minPos + sf::Vector2f(20, 20));
            groupBox.setFillColor(sf::Color::Transparent);

            if (i == selectedGroup_) {
                groupBox.setOutlineColor(Theme::Highlight);
                groupBox.setOutlineThickness(3);
            } else {
                groupBox.setOutlineColor(sf::Color(100, 100, 120));
                groupBox.setOutlineThickness(2);
            }

            window.draw(groupBox);

            sf::Text groupLabel(font_);
            groupLabel.setString(group.name);
            groupLabel.setCharacterSize(12);
            groupLabel.setFillColor(i == selectedGroup_ ? Theme::Highlight : sf::Color(150, 150, 150));
            groupLabel.setPosition(sf::Vector2f(minPos.x - 5, minPos.y - 20));
            window.draw(groupLabel);
        }
    }

    // Draw UI
    ui_.drawMenu(window);
    ui_.drawTooltip(window, mousePosF);
    ui_.drawProgressBar(window, isSaving_, isLoading_, saveProgress_);
    ui_.drawStatusMessage(window, statusMessage_, statusMessageTimer_);
    ui_.drawConfirmationDialog(window, showConfirmDialog_, confirmMessage_);

    if (isSaveModal_ || isLoadModal_) {
        ui_.drawModal(window, isSaveModal_, isLoadModal_, ui_.getModalAlpha());
    }

    if (showHelp_) drawHelpOverlay(window);

    performance_.drawFPS(window, true, performance_.isPerformanceMode(), font_);
}

void Workbench::update(float deltaTime) {
    // Update animations
    ui_.updateMenuAnimation(ui_.isMenuButtonHovered(), ui_.isMenuButtonPressed(), deltaTime);
    ui_.updateDropdownAnimation(ui_.showDropdown(), deltaTime);
    ui_.updateModalAnimation(isSaveModal_ || isLoadModal_, deltaTime);
    ui_.updateTooltip(ui_.getCurrentTooltip(), deltaTime);
    ui_.updateStatusMessage(deltaTime, statusMessageTimer_);

    // Update FPS
    performance_.updateFPS();

    // Update backup timer
    if (statusMessageTimer_ > 0.0f) {
        statusMessageTimer_ -= deltaTime;
        if (statusMessageTimer_ < 0.0f) statusMessageTimer_ = 0.0f;
    }

    if (autoBackupEnabled_ && !currentProjectName_.empty() && !isSaveModal_ && !isLoadModal_) {
        backupTimer_ += deltaTime;
        if (backupTimer_ >= BackupInterval) {
            backupTimer_ = 0.0f;
            projectManager_.createBackup(currentProjectName_, placedGates_, wires_);
            statusMessage_ = "Backup automatico criado em projetos/backups";
            statusMessageTimer_ = 4.0f;
        }
    }
}

void Workbench::updatePropagation() {
    if (isSaveModal_ || isLoadModal_) return;

    for (auto& gate : placedGates_) {
        for (auto& pin : gate->inputs) pin.state = false;
    }

    for (auto& wire : wires_) wire.update();
    for (auto& gate : placedGates_) gate->updateLogic();
}

void Workbench::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    // Handle confirmation dialog
    if (confirmationDialog_ && confirmationDialog_->isVisible()) {
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                confirmationDialog_->handleMouseClick(mousePosF.x, mousePosF.y);
                return;
            }
        }

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            confirmationDialog_->handleKeyPress(static_cast<int>(keyPressed->code));
            if (keyPressed->code == sf::Keyboard::Key::Escape) return;
        }

        if (event.is<sf::Event::MouseButtonPressed>() || 
            event.is<sf::Event::KeyPressed>() ||
            event.is<sf::Event::TextEntered>()) {
            return;
        }
    }

    // Handle save modal
    if (isSaveModal_) {
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            if (textEntered->unicode < 128) {
                char c = static_cast<char>(textEntered->unicode);
                std::string* buf = nullptr;
                if (modalFocus_ == ModalFocus::NAME) buf = &inputBuffer_;
                else if (modalFocus_ == ModalFocus::CATEGORY) buf = &categoryBuffer_;
                else if (modalFocus_ == ModalFocus::DESCRIPTION) buf = &descriptionBuffer_;
                else if (modalFocus_ == ModalFocus::TAGS) buf = &tagsBuffer_;

                if (c == '\r' || c == '\n') {
                    if (modalFocus_ == ModalFocus::NAME && !inputBuffer_.empty()) {
                        fs::path dir = projectManager_.projectDir(categoryBuffer_);
                        fs::create_directories(dir);
                        fs::path p = dir / (inputBuffer_ + ".gate");
                        isSaving_ = true;
                        saveProgress_ = 0.0f;
                        currentDescription_ = descriptionBuffer_;
                        currentTags_ = tagsBuffer_;
                        projectManager_.writeProject(p, placedGates_, wires_, inputBuffer_, categoryBuffer_,
                                                    descriptionBuffer_, tagsBuffer_);
                        isSaving_ = false;
                        currentProjectName_ = inputBuffer_;
                        currentCategory_ = categoryBuffer_;
                        statusMessage_ = "Projeto salvo em:\n" + fs::absolute(p).string();
                        statusMessageTimer_ = 5.0f;
                        isSaveModal_ = false;
                        saveAsMode_ = false;
                        refreshCategories();
                    }
                } else if (c == '\b') {
                    if (buf && !buf->empty()) buf->pop_back();
                } else if (std::isalnum(c) || c == '_' || c == '-' || c == ' ' || c == ',' || c == '.') {
                    if (buf && buf->size() < 200) *buf += c;
                }
            }
        }
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                isSaveModal_ = false;
                saveAsMode_ = false;
            } else if (keyPressed->code == sf::Keyboard::Key::Tab) {
                if (modalFocus_ == ModalFocus::NAME) modalFocus_ = ModalFocus::CATEGORY;
                else if (modalFocus_ == ModalFocus::CATEGORY) modalFocus_ = ModalFocus::DESCRIPTION;
                else if (modalFocus_ == ModalFocus::DESCRIPTION) modalFocus_ = ModalFocus::TAGS;
                else modalFocus_ = ModalFocus::NAME;
            }
        }
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                // Handle modal interactions
                if (ui_.nameInputRect().contains(mousePosF)) modalFocus_ = ModalFocus::NAME;
                else if (ui_.categoryInputRect().contains(mousePosF)) modalFocus_ = ModalFocus::CATEGORY;
                else if (ui_.descriptionInputRect().contains(mousePosF)) modalFocus_ = ModalFocus::DESCRIPTION;
                else if (ui_.tagsInputRect().contains(mousePosF)) modalFocus_ = ModalFocus::TAGS;
                else if (ui_.saveButtonRect().contains(mousePosF) && !inputBuffer_.empty()) {
                    fs::path dir = projectManager_.projectDir(categoryBuffer_);
                    fs::create_directories(dir);
                    fs::path p = dir / (inputBuffer_ + ".gate");
                    isSaving_ = true;
                    saveProgress_ = 0.0f;
                    currentDescription_ = descriptionBuffer_;
                    currentTags_ = tagsBuffer_;
                    projectManager_.writeProject(p, placedGates_, wires_, inputBuffer_, categoryBuffer_,
                                                descriptionBuffer_, tagsBuffer_);
                    isSaving_ = false;
                    currentProjectName_ = inputBuffer_;
                    currentCategory_ = categoryBuffer_;
                    statusMessage_ = "Projeto salvo em:\n" + fs::absolute(p).string();
                    statusMessageTimer_ = 5.0f;
                    isSaveModal_ = false;
                    saveAsMode_ = false;
                    refreshCategories();
                } else if (ui_.exportJsonRect().contains(mousePosF) && !inputBuffer_.empty()) {
                    projectManager_.exportJSON(inputBuffer_, categoryBuffer_, placedGates_, wires_);
                } else if (ui_.exportXmlRect().contains(mousePosF) && !inputBuffer_.empty()) {
                    projectManager_.exportXML(inputBuffer_, categoryBuffer_, placedGates_, wires_);
                }
            }
        }
        return;
    }

    // Handle load modal
    if (isLoadModal_) {
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            if (modalFocus_ == ModalFocus::SEARCH) {
                if (textEntered->unicode < 128) {
                    char c = static_cast<char>(textEntered->unicode);
                    if (c == '\r' || c == '\n') {
                        applySearch();
                    } else if (c == '\b') {
                        if (!searchBuffer_.empty()) searchBuffer_.pop_back();
                        applySearch();
                    } else if (std::isalnum(c) || c == ' ' || c == '_' || c == '-') {
                        if (searchBuffer_.size() < 50) {
                            searchBuffer_ += c;
                            applySearch();
                        }
                    }
                }
            }
        }
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (modalFocus_ == ModalFocus::SEARCH) {
                    modalFocus_ = ModalFocus::NAME;
                    searchBuffer_.clear();
                    refreshProjectList();
                } else {
                    isLoadModal_ = false;
                }
            } else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                if (!projectInfos_.empty()) {
                    load(projectInfos_[selectedProjectIndex_].name);
                    isLoadModal_ = false;
                }
            } else if (keyPressed->code == sf::Keyboard::Key::Up) {
                if (selectedProjectIndex_ > 0) {
                    selectedProjectIndex_--;
                    if ((int)selectedProjectIndex_ < listScroll_) listScroll_ = (int)selectedProjectIndex_;
                }
            } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                if (!projectInfos_.empty() && selectedProjectIndex_ < projectInfos_.size() - 1) {
                    selectedProjectIndex_++;
                    if ((int)selectedProjectIndex_ >= listScroll_ + 6)
                        listScroll_ = (int)selectedProjectIndex_ - 5;
                }
            } else if (keyPressed->code == sf::Keyboard::Key::F && keyPressed->control) {
                modalFocus_ = ModalFocus::SEARCH;
            }
        }
        if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical && !projectInfos_.empty()) {
                int maxScroll = std::max(0, (int)projectInfos_.size() - 6);
                listScroll_ = std::clamp(listScroll_ + (mouseWheel->delta > 0 ? -1 : 1), 0, maxScroll);
            }
        }
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                if (ui_.loadSearchRect().contains(mousePosF)) {
                    modalFocus_ = ModalFocus::SEARCH;
                } else if (ui_.loadSortBtnRect().contains(mousePosF)) {
                    int sortInt = (static_cast<int>(currentFilter_) + 1) % 7;
                    currentFilter_ = static_cast<WorkbenchInput::SelectionFilter>(sortInt);
                    applySort();
                }
            }
        }
        return;
    }

    // Palette scrolling with mouse wheel
    if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical &&
            mousePosF.x < Config::PaletteWidth && mousePosF.y > Config::TopMenuHeight) {
            paletteScroll_ -= mouseWheel->delta * 30.0f;
            updatePalettePositions();
        }
    }

    // Regular event handling
    ui_.setMenuButtonHovered(ui_.getMenuButton().getGlobalBounds().contains(mousePosF));
    ui_.setNewHovered(ui_.getNewText().getGlobalBounds().contains(mousePosF));
    ui_.setSaveHovered(ui_.getSaveText().getGlobalBounds().contains(mousePosF));
    ui_.setLoadHovered(ui_.getLoadText().getGlobalBounds().contains(mousePosF));
    ui_.setSaveAsHovered(ui_.getSaveAsText().getGlobalBounds().contains(mousePosF));

    if (ui_.isMenuButtonHovered() && !ui_.showDropdown()) {
        ui_.setCurrentTooltip("Open main menu");
    } else if (ui_.isNewHovered()) {
        ui_.setCurrentTooltip("Create a new project (Ctrl+N)");
    } else if (ui_.isSaveHovered()) {
        ui_.setCurrentTooltip("Save current project (Ctrl+S)");
    } else if (ui_.isLoadHovered()) {
        ui_.setCurrentTooltip("Load an existing project (Ctrl+L)");
    } else if (ui_.isSaveAsHovered()) {
        ui_.setCurrentTooltip("Save current project under a new name");
    } else {
        ui_.clearTooltip();
    }

    ui_.setShowDropdown(ui_.getMenuButton().getGlobalBounds().contains(mousePosF) ||
        (ui_.showDropdown() && ui_.getMenuButton().getGlobalBounds().contains(mousePosF)));

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            ui_.setMenuButtonPressed(ui_.getMenuButton().getGlobalBounds().contains(mousePosF));

            if (ui_.showDropdown()) {
                if (ui_.getNewText().getGlobalBounds().contains(mousePosF)) {
                    newProject();
                    ui_.setShowDropdown(false);
                    return;
                }
                if (ui_.getSaveText().getGlobalBounds().contains(mousePosF)) {
                    if (!currentProjectName_.empty()) {
                        save(currentProjectName_);
                    } else {
                        isSaveModal_ = true;
                        inputBuffer_ = "";
                        categoryBuffer_ = currentCategory_;
                        descriptionBuffer_ = currentDescription_;
                        tagsBuffer_ = currentTags_;
                        modalFocus_ = ModalFocus::NAME;
                        saveAsMode_ = false;
                        refreshCategories();
                    }
                    ui_.setShowDropdown(false);
                    return;
                }
                if (ui_.getSaveAsText().getGlobalBounds().contains(mousePosF)) {
                    isSaveModal_ = true;
                    inputBuffer_ = currentProjectName_;
                    categoryBuffer_ = currentCategory_;
                    descriptionBuffer_ = currentDescription_;
                    tagsBuffer_ = currentTags_;
                    modalFocus_ = ModalFocus::NAME;
                    saveAsMode_ = true;
                    refreshCategories();
                    ui_.setShowDropdown(false);
                    return;
                }
                if (ui_.getLoadText().getGlobalBounds().contains(mousePosF)) {
                    isLoadModal_ = true;
                    refreshCategories();
                    refreshProjectList();
                    ui_.setShowDropdown(false);
                    return;
                }
            }

            // Palette interaction
            for (auto& item : paletteItems_) {
                if (item->contains(mousePosF)) {
                    // Usa clone() para criar uma nova instância independente
                    // (funciona para portas, pinos e chips personalizados).
                    std::shared_ptr<Gate> newGate = item->clone(font_);
                    if (newGate) {
                        newGate->setPosition(mousePosF);
                        for (auto& g : placedGates_) g->isSelected = false;
                        newGate->isSelected = true;
                        newGate->isDragging = true;
                        newGate->dragOffset = {Config::GateWidth / 2.0f, Config::GateHeight / 2.0f};
                        placedGates_.push_back(newGate);
                    }
                    return;
                }
            }

            // Wiring and selection
            bool showDropdown = ui_.showDropdown();
            input_.handleMousePress(*event.getIf<sf::Event::MouseButtonPressed>(), mousePosF,
                                   paletteItems_, placedGates_, wires_,
                                   ui_.getMenuButton().getGlobalBounds(),
                                   ui_.getNewText().getGlobalBounds(),
                                   ui_.getSaveText().getGlobalBounds(),
                                   ui_.getLoadText().getGlobalBounds(),
                                   ui_.getSaveAsText().getGlobalBounds(),
                                   showDropdown, isSaveModal_, isLoadModal_,
                                   inputBuffer_, categoryBuffer_, descriptionBuffer_, tagsBuffer_,
                                   currentProjectName_, currentCategory_, currentDescription_, currentTags_,
                                   categories_, loadCategorySel_, projectInfos_, selectedProjectIndex_,
                                   listScroll_, searchBuffer_, font_);
        } else if (mousePressed->button == sf::Mouse::Button::Right) {
            if (input_.getState().isWiring) {
                input_.getState().isWiring = false;
                input_.getState().selectedPin = nullptr;
            }
        }
    } else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left) {
            for (auto& gate : placedGates_) gate->isDragging = false;
            input_.getState().isSelecting = false;
        }
    } else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        for (auto& gate : placedGates_) {
            if (gate->isDragging) {
                sf::Vector2f newPos = mousePosF - gate->dragOffset;
                gate->setPosition(snapToGrid(newPos));
            }
        }
        ui_.clearTooltip();
    } else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        WorkbenchInput::SelectionFilter currentFilter = currentFilter_;
        input_.handleKeyPress(*keyPressed, mousePosF, placedGates_, wires_,
                             isSaveModal_, isLoadModal_, showHelp_, showConfirmDialog_,
                             inputBuffer_, categoryBuffer_, descriptionBuffer_, tagsBuffer_,
                             currentProjectName_, currentCategory_, currentDescription_, currentTags_,
                             categories_, projectInfos_, selectedProjectIndex_, listScroll_, searchBuffer_,
                             enableSnapToGrid_, input_.getState().isWiring, input_.getState().selectedPin,
                             currentFilter, groups_, selectedGroup_, font_);
        currentFilter_ = currentFilter;
    }
}

void Workbench::save(const std::string& name) {
    isSaving_ = true;
    saveProgress_ = 0.0f;

    fs::path dir = projectManager_.projectDir(currentCategory_);
    fs::create_directories(dir);
    fs::path p = dir / (name + ".gate");
    projectManager_.writeProject(p, placedGates_, wires_, name, currentCategory_,
                                currentDescription_, currentTags_);

    saveProgress_ = 1.0f;
    currentProjectName_ = name;
    statusMessage_ = "Projeto salvo em: " + fs::absolute(p).string();
    statusMessageTimer_ = 4.0f;
    isSaving_ = false;
}

void Workbench::load(const std::string& name) {
    std::string cat = currentCategory_;
    for (const auto& info : projectInfos_) {
        if (info.name == name) {
            cat = info.category;
            currentDescription_ = info.description;
            currentTags_ = projectManager_.joinTags(info.tags);
            break;
        }
    }
    fs::path p = projectManager_.projectDir(cat) / (name + ".gate");

    if (projectManager_.loadProject(p, placedGates_, wires_, currentProjectName_, currentCategory_,
                                   currentDescription_, currentTags_, font_)) {
        statusMessage_ = "Projeto carregado: " + name + " (" + std::to_string(placedGates_.size()) + " componentes)";
        statusMessageTimer_ = 3.0f;
    } else {
        statusMessage_ = "Falha ao carregar projeto: " + name;
        statusMessageTimer_ = 5.0f;
    }
}

void Workbench::refreshProjectList() {
    projectInfos_.clear();
    if (!fs::exists("projetos")) fs::create_directory("projetos");

    std::string filter = (loadCategorySel_ == 0) ? "" : categories_[loadCategorySel_ - 1];

    auto scan = [&](const fs::path& dir, const std::string& cat) {
        std::error_code ec;
        if (!fs::exists(dir, ec)) return;
        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (entry.path().extension() == ".gate") {
                projectInfos_.push_back(projectManager_.readProjectInfo(entry.path(), cat));
            }
        }
    };

    if (filter.empty()) {
        scan("projetos", "");
        for (const auto& c : categories_) scan(projectManager_.projectDir(c), c);
    } else {
        scan(projectManager_.projectDir(filter), filter);
    }

    applySort();
    selectedProjectIndex_ = 0;
    listScroll_ = 0;
}

void Workbench::refreshCategories() {
    categories_.clear();
    if (!fs::exists("projetos")) fs::create_directory("projetos");
    for (const auto& entry : fs::directory_iterator("projetos")) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name != "backups") categories_.push_back(name);
        }
    }
    std::sort(categories_.begin(), categories_.end());
}

void Workbench::renderImGui() {
    ImGui::Begin("Logic Gate Simulator");

    ImGui::Text("Gates placed: %zu", placedGates_.size());
    ImGui::Text("Wires connected: %zu", wires_.size());

    if (!currentProjectName_.empty()) {
        ImGui::Text("Project: %s", currentProjectName_.c_str());
        ImGui::Text("Category: %s", currentCategory_.empty() ? "(raiz)" : currentCategory_.c_str());
        ImGui::Text("Path: %s", projectManager_.projectDir(currentCategory_).string().c_str());
    }

    ImGui::Separator();

    if (ImGui::Button("New Project")) newProject();
    if (ImGui::Button("Save Project")) {
        if (!currentProjectName_.empty()) {
            save(currentProjectName_);
        } else {
            isSaveModal_ = true;
            inputBuffer_ = "";
            categoryBuffer_ = currentCategory_;
            descriptionBuffer_ = currentDescription_;
            tagsBuffer_ = currentTags_;
            modalFocus_ = ModalFocus::NAME;
            saveAsMode_ = false;
            refreshCategories();
        }
    }
    if (ImGui::Button("Save As...")) {
        isSaveModal_ = true;
        inputBuffer_ = currentProjectName_;
        categoryBuffer_ = currentCategory_;
        descriptionBuffer_ = currentDescription_;
        tagsBuffer_ = currentTags_;
        modalFocus_ = ModalFocus::NAME;
        saveAsMode_ = true;
        refreshCategories();
    }
    if (ImGui::Button("Load Project")) {
        isLoadModal_ = true;
        refreshCategories();
        refreshProjectList();
    }

    ImGui::Separator();
    ImGui::Text("Chips personalizados:");
    static char chipId[64] = "meu_chip";
    static char chipName[64] = "Meu Chip";
    static char chipDesc[128] = "";
    static char chipCat[64] = "Chips";
    ImGui::InputText("ID", chipId, sizeof(chipId));
    ImGui::InputText("Nome", chipName, sizeof(chipName));
    ImGui::InputText("Categoria", chipCat, sizeof(chipCat));
    ImGui::InputText("Descricao", chipDesc, sizeof(chipDesc));
    if (ImGui::Button("Salvar como Chip")) {
        saveCurrentAsChip(chipId, chipName, chipDesc, chipCat);
    }

    ImGui::Separator();

    if (ImGui::Button("Export JSON")) {
        if (!currentProjectName_.empty()) projectManager_.exportJSON(currentProjectName_, currentCategory_, placedGates_, wires_);
    }
    if (ImGui::Button("Export XML")) {
        if (!currentProjectName_.empty()) projectManager_.exportXML(currentProjectName_, currentCategory_, placedGates_, wires_);
    }

    ImGui::Separator();
    ImGui::Text("Auto-backup: %s", autoBackupEnabled_ ? "ON (5 min)" : "OFF");
    if (ImGui::Button("Create Backup Now")) projectManager_.createBackup(currentProjectName_, placedGates_, wires_);

    ImGui::End();
}

void Workbench::showConfirmation(ConfirmationDialog::Action action, const std::string& message, int projectIndex) {
    if (!confirmationDialog_) return;

    confirmationDialog_->show(action, message,
        [this, action, projectIndex]() {
            switch (action) {
                case ConfirmationDialog::Action::DELETE_PROJECT:
                    if (projectIndex >= 0 && projectIndex < (int)projectInfos_.size()) {
                        fs::path p = projectInfos_[projectIndex].fullPath;
                        fs::remove(p);
                        fs::remove(projectManager_.metaPath(p));
                        refreshProjectList();
                        statusMessage_ = "Projeto excluído: " + projectInfos_[projectIndex].name;
                        statusMessageTimer_ = 3.0f;
                    }
                    break;
                case ConfirmationDialog::Action::NEW_PROJECT:
                    newProject();
                    break;
                default:
                    break;
            }
        },
        [this]() {
            statusMessage_ = "Ação cancelada";
            statusMessageTimer_ = 2.0f;
        }
    );
}

void Workbench::toggleHelp() {
    showHelp_ = !showHelp_;
}

void Workbench::drawHelpOverlay(sf::RenderWindow& window) {
    ui_.drawHelpOverlay(window, showHelp_, font_);
}

void Workbench::applySort() {
    // Sort implementation
}

void Workbench::applySearch() {
    // Search implementation
}

std::string Workbench::sortOrderLabel() const {
    return "Ordenar";
}

std::vector<std::string> Workbench::splitTags(const std::string& s) {
    return WorkbenchProject::splitTags(s);
}

std::string Workbench::joinTags(const std::vector<std::string>& tags) {
    return WorkbenchProject::joinTags(tags);
}

void Workbench::toggleStarred(size_t index) {
    if (index >= projectInfos_.size()) return;
    projectInfos_[index].starred = !projectInfos_[index].starred;
    projectManager_.saveMeta(projectInfos_[index]);
    applySort();
}

void Workbench::deleteProject(size_t index) {
    if (index >= projectInfos_.size()) return;
    std::string msg = "Tem certeza que deseja excluir o projeto:\n\"" + 
                      projectInfos_[index].name + "\"?\n\nEsta ação não pode ser desfeita.";
    showConfirmation(ConfirmationDialog::Action::DELETE_PROJECT, msg, index);
}

void Workbench::writeProject(const fs::path& p) {
    projectManager_.writeProject(p, placedGates_, wires_, currentProjectName_, currentCategory_,
                                currentDescription_, currentTags_);
}

void Workbench::exportJSON(const std::string& baseName, const std::string& category) {
    projectManager_.exportJSON(baseName, category, placedGates_, wires_);
    statusMessage_ = "Exportado (JSON): " + projectManager_.projectDir(category).string();
    statusMessageTimer_ = 5.0f;
}

void Workbench::exportXML(const std::string& baseName, const std::string& category) {
    projectManager_.exportXML(baseName, category, placedGates_, wires_);
    statusMessage_ = "Exportado (XML): " + projectManager_.projectDir(category).string();
    statusMessageTimer_ = 5.0f;
}

void Workbench::createBackup() {
    projectManager_.createBackup(currentProjectName_, placedGates_, wires_);
}

void Workbench::cycleTheme() {
    accessibility_.cycleTheme();
}

void Workbench::cycleFontScale() {
    accessibility_.cycleFontScale();
}

void Workbench::toggleHighContrast() {
    accessibility_.toggleHighContrast();
}

void Workbench::toggleLargeText() {
    accessibility_.toggleLargeText();
}

void Workbench::applyTheme() {
    ui_.setupMenu();
    ui_.setupPalette();
    ui_.setupModal();
    ui_.setupTooltip();
}

void Workbench::applyFontScale() {
    ui_.setupMenu();
    ui_.setupModal();
    ui_.setupTooltip();
}

void Workbench::togglePerformanceMode() {
    performance_.togglePerformanceMode();
    statusMessage_ = performance_.isPerformanceMode() ? "Modo Performance: ATIVADO" : "Modo Performance: DESATIVADO";
    statusMessageTimer_ = 3.0f;
}

void Workbench::updatePerformanceSettings() {
    // Performance settings are managed internally by WorkbenchPerformance
}

void Workbench::handleError(const std::string& error) {
    performance_.handleError(error);
    statusMessage_ = "Erro: " + error;
    statusMessageTimer_ = 5.0f;
}

void Workbench::clearError() {
    performance_.clearError();
}

bool Workbench::validateProjectData() const {
    return performance_.validateProjectData(placedGates_, wires_);
}

bool Workbench::attemptRecovery() {
    return performance_.attemptRecovery(placedGates_, wires_);
}

void Workbench::saveEmergencyBackup() {
    performance_.saveEmergencyBackup(currentProjectName_, placedGates_, wires_);
}