#include <imgui.h>
#include "Workbench.h"
#include "Constants.h"
#include "LogicEntities.h"
#include "Wire.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdint>
#include <windows.h>
#include <ctime>
#include <chrono>

namespace fs = std::filesystem;

Workbench::Workbench(const sf::Font& font)
    : font(font)
    , menuBar(sf::Vector2f(0, 0))
    , menuButton(sf::Vector2f(0, 0))
    , dropdown(sf::Vector2f(0, 0))
    , overlay(sf::Vector2f(0, 0))
    , modalRect(sf::Vector2f(0, 0))
    , selectionRect(sf::Vector2f(0, 0))
    , paletteRect(sf::Vector2f(0, 0))
    , progressBar(sf::Vector2f(0, 0))
    , tooltipBg(sf::Vector2f(0, 0))
    , menuText(font, "", 16)
    , saveText(font, "", 14)
    , loadText(font, "", 14)
    , newText(font, "", 14)
    , saveAsText(font, "", 14)
    , modalTitle(font, "", 20)
    , inputText(font, "", 18)
    , listText(font, "", 16)
    , progressText(font, "", 14)
    , tooltipText(font, "", 14) {
    setupMenu();
    setupPalette();
    setupModal();
    setupTooltip();
    refreshCategories();
}

void Workbench::newProject() {
    placedGates.clear();
    wires.clear();
    currentProjectName = "";
    currentCategory = "";
    undoStack.clear();
    redoStack.clear();
}

// Undo/Redo system
void Workbench::saveState() {
    StateSnapshot snapshot;
    snapshot.gates = placedGates;
    snapshot.wires = wires;
    undoStack.push_back(snapshot);
    if (undoStack.size() > MaxUndoSteps) undoStack.erase(undoStack.begin());
    redoStack.clear();  // Clear redo stack on new action
}

void Workbench::undo() {
    if (undoStack.empty()) return;

    StateSnapshot current;
    current.gates = placedGates;
    current.wires = wires;
    redoStack.push_back(current);
    if (redoStack.size() > MaxUndoSteps) redoStack.erase(redoStack.begin());

    StateSnapshot prev = undoStack.back();
    placedGates = prev.gates;
    wires = prev.wires;
    undoStack.pop_back();
}

void Workbench::redo() {
    if (redoStack.empty()) return;

    StateSnapshot current;
    current.gates = placedGates;
    current.wires = wires;
    undoStack.push_back(current);
    if (undoStack.size() > MaxUndoSteps) undoStack.erase(undoStack.begin());

    StateSnapshot next = redoStack.back();
    placedGates = next.gates;
    wires = next.wires;
    redoStack.pop_back();
}

// Snap-to-grid function
sf::Vector2f Workbench::snapToGrid(sf::Vector2f pos) {
    if (!enableSnapToGrid) return pos;
    return sf::Vector2f(
        std::round(pos.x / Config::GridSize) * Config::GridSize,
        std::round(pos.y / Config::GridSize) * Config::GridSize
    );
}

void Workbench::setupMenu() {
    menuBar.setSize({1200, Config::TopMenuHeight});
    menuBar.setFillColor(Theme::TopMenu);

    menuButton.setSize({100, Config::TopMenuHeight - 10});
    menuButton.setPosition({10, 5});
    menuButton.setFillColor(Theme::Panel);
    menuButton.setOutlineThickness(3);
    menuButton.setOutlineColor(Theme::GateOutline);

    menuText.setFont(font);
    menuText.setString("Menu");
    menuText.setCharacterSize(16);
    menuText.setFillColor(Theme::Text);
    menuText.setPosition({25, 10});

    dropdown.setSize({140, 150});
    dropdown.setFillColor(Theme::Panel);
    dropdown.setOutlineThickness(3);
    dropdown.setOutlineColor(Theme::GateOutline);

    newText.setFont(font);
    newText.setString("New (Ctrl+N)");
    newText.setCharacterSize(14);
    saveText.setFont(font);
    saveText.setString("Save (Ctrl+S)");
    saveText.setCharacterSize(14);
    loadText.setFont(font);
    loadText.setString("Load (Ctrl+L)");
    loadText.setCharacterSize(14);
    saveAsText.setFont(font);
    saveAsText.setString("Save As...");
    saveAsText.setCharacterSize(14);
}

void Workbench::setupPalette() {
    paletteRect.setSize({Config::PaletteWidth, 800});
    paletteRect.setFillColor(Theme::Panel);
    paletteRect.setPosition({0, Config::TopMenuHeight});

    float startY = Config::TopMenuHeight + 20;
    paletteItems.push_back(std::make_shared<ANDGate>(sf::Vector2f(25, startY), font));
    paletteItems.push_back(std::make_shared<ORGate>(sf::Vector2f(25, startY + 90), font));
    paletteItems.push_back(std::make_shared<NOTGate>(sf::Vector2f(25, startY + 180), font));
    paletteItems.push_back(std::make_shared<ClockGate>(sf::Vector2f(25, startY + 270), font));
    paletteItems.push_back(std::make_shared<PowerSource>(sf::Vector2f(25, startY + 360), font));
}

void Workbench::setupModal() {
    overlay.setSize({1200, 800});
    overlay.setFillColor(Theme::Overlay);

    modalRect.setSize({ModalW, ModalH});
    modalRect.setFillColor(Theme::ModalBg);
    modalRect.setOutlineThickness(2);
    modalRect.setOutlineColor(Theme::ModalBorder);
    modalRect.setPosition({ModalX, ModalY});

    modalTitle.setCharacterSize(20);
    modalTitle.setFillColor(Theme::Highlight);

    inputText.setCharacterSize(18);
    inputText.setFillColor(Theme::Text);

    listText.setCharacterSize(16);
    listText.setFillColor(Theme::Text);
}

void Workbench::setupTooltip() {
    tooltipBg.setFillColor(Theme::TooltipBg);
    tooltipBg.setOutlineThickness(1);
    tooltipBg.setOutlineColor(Theme::TooltipBorder);

    tooltipText.setCharacterSize(14);
    tooltipText.setFillColor(Theme::TooltipText);

    progressBar.setFillColor(Theme::ProgressFill);
    progressBar.setOutlineThickness(1);
    progressBar.setOutlineColor(Theme::ProgressBorder);
}

// Modal layout geometry (shared between draw and handleEvent)
sf::FloatRect Workbench::chipRect(size_t i, float startY) {
    size_t col = i % 4;
    size_t row = i / 4;
    float x = ModalX + 20 + static_cast<float>(col) * (ChipW + ChipGap);
    float y = startY + static_cast<float>(row) * (ChipH + 6);
    return sf::FloatRect({x, y}, {ChipW, ChipH});
}
sf::FloatRect Workbench::nameInputRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 68}, {ModalW - 40, 32});
}
sf::FloatRect Workbench::categoryInputRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 130}, {ModalW - 40, 32});
}
sf::FloatRect Workbench::saveButtonRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 280}, {200, 30});
}
sf::FloatRect Workbench::exportJsonRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 318}, {200, 28});
}
sf::FloatRect Workbench::exportXmlRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 352}, {200, 28});
}
static constexpr float LoadListBaseY = Workbench::ModalY + 140.f;
static constexpr float LoadRowH = 40.f;
static constexpr int LoadVisibleRows = 6;
sf::FloatRect Workbench::loadRowRect(size_t i) const {
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i) - listScroll) * LoadRowH;
    return sf::FloatRect({ModalX + 15, y}, {ModalW - 30, LoadRowH - 4});
}
sf::FloatRect Workbench::loadRowLoadBtnRect(size_t i) const {
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i) - listScroll) * LoadRowH;
    return sf::FloatRect({ModalX + ModalW - 160, y + 7}, {65, 26});
}
sf::FloatRect Workbench::loadRowDeleteBtnRect(size_t i) const {
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i) - listScroll) * LoadRowH;
    return sf::FloatRect({ModalX + ModalW - 85, y + 7}, {65, 26});
}

// Path / metadata helpers
fs::path Workbench::projectDir(const std::string& category) const {
    fs::path base = "projetos";
    if (category.empty()) return base;
    return base / category;
}

std::string Workbench::formatSize(std::uintmax_t bytes) const {
    if (bytes < 1024) return std::to_string(bytes) + " B";
    double kb = bytes / 1024.0;
    if (kb < 1024) return std::to_string(static_cast<int>(kb)) + " KB";
    double mb = kb / 1024.0;
    return std::to_string(static_cast<int>(mb)) + " MB";
}

std::string Workbench::formatFileTime(std::filesystem::file_time_type t) const {
    auto now_sys = std::chrono::system_clock::now();
    auto now_file = std::filesystem::file_time_type::clock::now();
    auto offset = now_sys.time_since_epoch() -
        std::chrono::duration_cast<std::chrono::system_clock::duration>(now_file.time_since_epoch());
    auto sys_dur = std::chrono::duration_cast<std::chrono::system_clock::duration>(t.time_since_epoch());
    auto sys_tp = std::chrono::system_clock::time_point(sys_dur + offset);
    std::time_t tt = std::chrono::system_clock::to_time_t(sys_tp);
    std::tm tm{};
    localtime_s(&tm, &tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y %H:%M");
    return oss.str();
}

std::string fileTimeToString(FILETIME ft) {
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    auto dur = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::nanoseconds(ul.QuadPart * 100));
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(dur));
    std::tm tm{};
    localtime_s(&tm, &tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y %H:%M");
    return oss.str();
}

Workbench::ProjectInfo Workbench::readProjectInfo(const fs::path& p, const std::string& category) {
    ProjectInfo info;
    info.name = p.stem().string();
    info.category = category;
    info.fullPath = fs::absolute(p);
    std::error_code ec;
    info.size = fs::file_size(p, ec);
    info.modified = fs::last_write_time(p, ec);

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(p.wstring().c_str(), GetFileExInfoStandard, &fad)) {
        info.createdStr = fileTimeToString(fad.ftCreationTime);
    }

    std::ifstream f(p);
    if (f) {
        size_t gateCount = 0;
        if (f >> gateCount) {
            info.gateCount = gateCount;
            std::string dummy;
            for (size_t i = 0; i < gateCount; ++i) std::getline(f, dummy);
            size_t wireCount = 0;
            if (f >> wireCount) info.wireCount = wireCount;
        }
    }
    return info;
}

void Workbench::refreshCategories() {
    categories.clear();
    if (!fs::exists("projetos")) fs::create_directory("projetos");
    for (const auto& entry : fs::directory_iterator("projetos")) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name != "backups") categories.push_back(name);
        }
    }
    std::sort(categories.begin(), categories.end());
}

void Workbench::refreshProjectList() {
    projectInfos.clear();
    if (!fs::exists("projetos")) fs::create_directory("projetos");

    std::string filter = (loadCategorySel == 0) ? "" : categories[loadCategorySel - 1];

    auto scan = [&](const fs::path& dir, const std::string& cat) {
        std::error_code ec;
        if (!fs::exists(dir, ec)) return;
        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (entry.path().extension() == ".gate") {
                projectInfos.push_back(readProjectInfo(entry.path(), cat));
            }
        }
    };

    if (filter.empty()) {
        scan("projetos", "");
        for (const auto& c : categories) scan(projectDir(c), c);
    } else {
        scan(projectDir(filter), filter);
    }

    std::sort(projectInfos.begin(), projectInfos.end(),
        [](const ProjectInfo& a, const ProjectInfo& b) {
            return a.name < b.name;
        });

    selectedProjectIndex = 0;
    listScroll = 0;
}

void Workbench::deleteProject(size_t index) {
    if (index >= projectInfos.size()) return;
    std::error_code ec;
    fs::remove(projectInfos[index].fullPath, ec);
    refreshProjectList();
    refreshCategories();
}

void Workbench::writeProject(const fs::path& p) {
    std::ofstream f(p);
    if (!f) return;

    saveProgress = 0.3f;
    f << placedGates.size() << "\n";
    for (auto& g : placedGates) {
        f << (int)g->getType() << " " << g->getPosition().x << " " << g->getPosition().y << " " << g->getExtraData() << "\n";
    }

    auto getGateIdx = [&](Gate* g) -> int {
        for (size_t i = 0; i < placedGates.size(); ++i) {
            if (placedGates[i].get() == g) return (int)i;
        }
        return -1;
    };
    auto getPinIdx = [&](Gate* g, Pin* p, bool input) -> int {
        const auto& pins = input ? g->inputs : g->outputs;
        for (size_t i = 0; i < pins.size(); ++i) {
            if (&pins[i] == p) return (int)i;
        }
        return -1;
    };

    saveProgress = 0.6f;
    f << wires.size() << "\n";
    for (auto& w : wires) {
        int gFrom = getGateIdx(w.fromPin->parent);
        int pFrom = getPinIdx(w.fromPin->parent, w.fromPin, false);
        int gTo = getGateIdx(w.toPin->parent);
        int pTo = getPinIdx(w.toPin->parent, w.toPin, true);
        f << gFrom << " " << pFrom << " " << gTo << " " << pTo << "\n";
    }
    f.close();
    saveProgress = 1.0f;
}

void Workbench::exportJSON(const std::string& baseName, const std::string& category) {
    fs::path dir = projectDir(category);
    fs::create_directories(dir);
    fs::path p = dir / (baseName + ".json");
    std::ofstream f(p);
    if (!f) return;

    f << "{\n";
    f << "  \"name\": \"" << baseName << "\",\n";
    f << "  \"category\": \"" << category << "\",\n";
    f << "  \"gates\": [\n";
    for (size_t i = 0; i < placedGates.size(); ++i) {
        auto& g = placedGates[i];
        f << "    {\"index\": " << i << ", \"type\": " << (int)g->getType()
          << ", \"x\": " << g->getPosition().x << ", \"y\": " << g->getPosition().y
          << ", \"extra\": " << g->getExtraData() << "}";
        f << (i + 1 < placedGates.size() ? ",\n" : "\n");
    }
    f << "  ],\n";
    f << "  \"wires\": [\n";
    for (size_t i = 0; i < wires.size(); ++i) {
        auto& w = wires[i];
        int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
        for (size_t gi = 0; gi < placedGates.size(); ++gi) {
            if (placedGates[gi].get() == w.fromPin->parent) gFrom = (int)gi;
            if (placedGates[gi].get() == w.toPin->parent) gTo = (int)gi;
        }
        for (size_t pi = 0; pi < w.fromPin->parent->outputs.size(); ++pi)
            if (&w.fromPin->parent->outputs[pi] == w.fromPin) pFrom = (int)pi;
        for (size_t pi = 0; pi < w.toPin->parent->inputs.size(); ++pi)
            if (&w.toPin->parent->inputs[pi] == w.toPin) pTo = (int)pi;
        f << "    {\"fromGate\": " << gFrom << ", \"fromPin\": " << pFrom
          << ", \"toGate\": " << gTo << ", \"toPin\": " << pTo << "}";
        f << (i + 1 < wires.size() ? ",\n" : "\n");
    }
    f << "  ]\n";
    f << "}\n";
    f.close();
    statusMessage = "Exportado (JSON): " + fs::absolute(p).string();
    statusMessageTimer = 5.0f;
}

void Workbench::exportXML(const std::string& baseName, const std::string& category) {
    fs::path dir = projectDir(category);
    fs::create_directories(dir);
    fs::path p = dir / (baseName + ".xml");
    std::ofstream f(p);
    if (!f) return;

    f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    f << "<project name=\"" << baseName << "\" category=\"" << category << "\">\n";
    f << "  <gates>\n";
    for (size_t i = 0; i < placedGates.size(); ++i) {
        auto& g = placedGates[i];
        f << "    <gate index=\"" << i << "\" type=\"" << (int)g->getType()
          << "\" x=\"" << g->getPosition().x << "\" y=\"" << g->getPosition().y
          << "\" extra=\"" << g->getExtraData() << "\"/>\n";
    }
    f << "  </gates>\n";
    f << "  <wires>\n";
    for (auto& w : wires) {
        int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
        for (size_t gi = 0; gi < placedGates.size(); ++gi) {
            if (placedGates[gi].get() == w.fromPin->parent) { gFrom = (int)gi; }
            if (placedGates[gi].get() == w.toPin->parent) { gTo = (int)gi; }
        }
        for (size_t pi = 0; pi < w.fromPin->parent->outputs.size(); ++pi)
            if (&w.fromPin->parent->outputs[pi] == w.fromPin) pFrom = (int)pi;
        for (size_t pi = 0; pi < w.toPin->parent->inputs.size(); ++pi)
            if (&w.toPin->parent->inputs[pi] == w.toPin) pTo = (int)pi;
        f << "    <wire fromGate=\"" << gFrom << "\" fromPin=\"" << pFrom
          << "\" toGate=\"" << gTo << "\" toPin=\"" << pTo << "\"/>\n";
    }
    f << "  </wires>\n";
    f << "</project>\n";
    f.close();
    statusMessage = "Exportado (XML): " + fs::absolute(p).string();
    statusMessageTimer = 5.0f;
}

void Workbench::createBackup() {
    if (currentProjectName.empty()) return;
    fs::path backupDir = "projetos/backups";
    fs::create_directories(backupDir);
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &tt);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &tm);
    fs::path p = backupDir / (currentProjectName + "_" + ts + ".gate");
    writeProject(p);
}

void Workbench::draw(sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    window.draw(paletteRect);
    for (auto& item : paletteItems) item->draw(window);

    for (auto& wire : wires) wire.draw(window);
    for (auto& gate : placedGates) gate->draw(window);

    if (isWiring && selectedPin) {
        sf::Vertex line[] = {
            {selectedPin->shape.getPosition(), Theme::Accent},
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

    if (isSelecting) window.draw(selectionRect);

    for (size_t i = 0; i < groups.size(); ++i) {
        const auto& group = groups[i];
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

            if (i == selectedGroup) {
                groupBox.setOutlineColor(Theme::Highlight);
                groupBox.setOutlineThickness(3);
            } else {
                groupBox.setOutlineColor(sf::Color(100, 100, 120));
                groupBox.setOutlineThickness(2);
            }

            window.draw(groupBox);

            sf::Text groupLabel(font);
            groupLabel.setString(group.name);
            groupLabel.setCharacterSize(12);
            groupLabel.setFillColor(i == selectedGroup ? Theme::Highlight : sf::Color(150, 150, 150));
            groupLabel.setPosition(sf::Vector2f(minPos.x - 5, minPos.y - 20));
            window.draw(groupLabel);
        }
    }

    window.draw(menuBar);

    if (isMenuButtonPressed) {
        menuButton.setFillColor(Theme::ButtonPressed);
    } else if (isMenuButtonHovered) {
        menuButton.setFillColor(Theme::ButtonHover);
    } else {
        menuButton.setFillColor(Theme::ButtonNormal);
    }
    sf::Color menuBtnColor = menuButton.getFillColor();
    menuButton.setFillColor(sf::Color(menuBtnColor.r, menuBtnColor.g, menuBtnColor.b,
        static_cast<std::uint8_t>(menuButtonAlpha)));
    window.draw(menuButton);
    window.draw(menuText);

    if (showDropdown || dropdownAlpha > 0) {
        dropdown.setPosition({10, Config::TopMenuHeight});
        sf::Color dropdownColor = dropdown.getFillColor();
        dropdown.setFillColor(sf::Color(dropdownColor.r,
            dropdownColor.g,
            dropdownColor.b,
            static_cast<std::uint8_t>(dropdownAlpha)));
        window.draw(dropdown);
        newText.setPosition({20, Config::TopMenuHeight + 10});
        saveText.setPosition({20, Config::TopMenuHeight + 45});
        loadText.setPosition({20, Config::TopMenuHeight + 80});
        saveAsText.setPosition({20, Config::TopMenuHeight + 115});

        newText.setFillColor(isNewHovered ? Theme::Highlight : Theme::Text);
        saveText.setFillColor(isSaveHovered ? Theme::Highlight : Theme::Text);
        loadText.setFillColor(isLoadHovered ? Theme::Highlight : Theme::Text);
        saveAsText.setFillColor(isSaveAsHovered ? Theme::Highlight : Theme::Text);

        window.draw(newText);
        window.draw(saveText);
        window.draw(loadText);
        window.draw(saveAsText);
    }

    if (!currentTooltip.empty() && tooltipTimer >= TooltipDelay) {
        tooltipText.setString(currentTooltip);
        sf::FloatRect textBounds = tooltipText.getGlobalBounds();
        float padding = 10.0f;
        tooltipBg.setSize({textBounds.size.x + padding * 2, textBounds.size.y + padding * 2});
        tooltipBg.setPosition({mousePosF.x + 15, mousePosF.y + 15});
        tooltipText.setPosition({mousePosF.x + 15 + padding, mousePosF.y + 15 + padding});
        window.draw(tooltipBg);
        window.draw(tooltipText);
    }

    if (isSaving || isLoading) {
        float progress = isSaving ? saveProgress : loadProgress;
        std::string statusText = isSaving ? "Salvando..." : "Carregando...";

        sf::RectangleShape progressBg;
        progressBg.setSize({300, 30});
        progressBg.setPosition({450, 400});
        progressBg.setFillColor(Theme::ProgressBg);
        progressBg.setOutlineThickness(2);
        progressBg.setOutlineColor(Theme::ProgressBorder);
        window.draw(progressBg);

        progressBar.setSize({296 * progress, 26});
        progressBar.setPosition({452, 402});
        window.draw(progressBar);

        progressText.setString(statusText + " " + std::to_string(int(progress * 100)) + "%");
        progressText.setPosition({500, 405});
        progressText.setFillColor(Theme::Text);
        window.draw(progressText);
    }

    if (statusMessageTimer > 0.0f && statusMessageTimer < 5.0f) {
        sf::Text status(font);
        status.setFont(font);
        status.setString(statusMessage);
        status.setCharacterSize(13);
        status.setFillColor(Theme::Highlight);
        status.setPosition({180, 770});
        window.draw(status);
    }

    if (isSaveModal || isLoadModal || modalAlpha > 0) {
        sf::Color overlayColor = overlay.getFillColor();
        std::uint8_t overlayAlpha = static_cast<std::uint8_t>(modalAlpha * 0.8f);
        overlay.setFillColor(sf::Color(overlayColor.r, overlayColor.g, overlayColor.b, overlayAlpha));
        window.draw(overlay);

        sf::Color modalColor = modalRect.getFillColor();
        std::uint8_t modalAlphaUint8 = static_cast<std::uint8_t>(modalAlpha);
        modalRect.setFillColor(sf::Color(modalColor.r, modalColor.g, modalColor.b, modalAlphaUint8));
        window.draw(modalRect);

        modalTitle.setPosition({ModalX + 20, ModalY + 15});
        modalTitle.setString(isSaveModal ? (saveAsMode ? "Salvar Como" : "Salvar Projeto")
                                          : "Carregar Projeto");
        std::uint8_t titleAlpha = static_cast<std::uint8_t>(modalAlpha);
        modalTitle.setFillColor(sf::Color(Theme::Highlight.r, Theme::Highlight.g, Theme::Highlight.b, titleAlpha));
        window.draw(modalTitle);

        if (isSaveModal) {
            listText.setCharacterSize(14);
            listText.setFillColor(Theme::Text);
            listText.setString("Nome do projeto:");
            listText.setPosition({ModalX + 20, ModalY + 50});
            window.draw(listText);

            sf::RectangleShape nameBox(nameInputRect().size);
            nameBox.setPosition(nameInputRect().position);
            nameBox.setFillColor(Theme::GateBody);
            nameBox.setOutlineThickness(1);
            nameBox.setOutlineColor(modalFocus == ModalFocus::NAME ? Theme::Highlight : Theme::Accent);
            window.draw(nameBox);
            inputText.setPosition({ModalX + 28, ModalY + 73});
            inputText.setString(inputBuffer + (modalFocus == ModalFocus::NAME ? "_" : ""));
            window.draw(inputText);

            listText.setString("Categoria (pasta):");
            listText.setPosition({ModalX + 20, ModalY + 112});
            window.draw(listText);

            sf::RectangleShape catBox(categoryInputRect().size);
            catBox.setPosition(categoryInputRect().position);
            catBox.setFillColor(Theme::GateBody);
            catBox.setOutlineThickness(1);
            catBox.setOutlineColor(modalFocus == ModalFocus::CATEGORY ? Theme::Highlight : Theme::Accent);
            window.draw(catBox);
            inputText.setString(categoryBuffer + (modalFocus == ModalFocus::CATEGORY ? "_" : ""));
            inputText.setPosition({ModalX + 28, ModalY + 135});
            window.draw(inputText);

            float chipY = ModalY + 175;
            auto drawChip = [&](const std::string& label, const sf::FloatRect& r, bool selected) {
                sf::RectangleShape chip(r.size);
                chip.setPosition(r.position);
                chip.setFillColor(selected ? Theme::ButtonActive : Theme::ButtonNormal);
                chip.setOutlineThickness(1);
                chip.setOutlineColor(Theme::ModalBorder);
                window.draw(chip);
                sf::Text chipText(font);
                chipText.setFont(font);
                chipText.setString(label);
                chipText.setCharacterSize(12);
                chipText.setFillColor(Theme::Text);
                sf::FloatRect cb = chipText.getLocalBounds();
                float maxW = r.size.x - 8;
                if (cb.size.x > maxW) {
                    std::string s = label;
                    while (!s.empty() && chipText.getLocalBounds().size.x > maxW) {
                        s.pop_back();
                        chipText.setString(s + "..");
                    }
                }
                chipText.setPosition({r.position.x + 4, r.position.y + 4});
                window.draw(chipText);
            };
            drawChip("raiz", chipRect(0, chipY), categoryBuffer.empty());
            for (size_t i = 0; i < categories.size(); ++i) {
                drawChip(categories[i], chipRect(i + 1, chipY), categories[i] == categoryBuffer);
            }

            sf::FloatRect sb = saveButtonRect();
            sf::RectangleShape saveBtn(sb.size);
            saveBtn.setPosition(sb.position);
            saveBtn.setFillColor(Theme::ButtonActive);
            window.draw(saveBtn);
            sf::Text saveBtnText(font);
            saveBtnText.setFont(font);
            saveBtnText.setString("Salvar");
            saveBtnText.setCharacterSize(14);
            saveBtnText.setFillColor(sf::Color::White);
            saveBtnText.setPosition({sb.position.x + 70, sb.position.y + 6});
            window.draw(saveBtnText);

            sf::FloatRect ej = exportJsonRect();
            sf::RectangleShape ejBtn(ej.size);
            ejBtn.setPosition(ej.position);
            ejBtn.setFillColor(Theme::ButtonNormal);
            ejBtn.setOutlineThickness(1);
            ejBtn.setOutlineColor(Theme::Accent);
            window.draw(ejBtn);
            sf::Text ejText(font);
            ejText.setFont(font);
            ejText.setString("Exportar JSON");
            ejText.setCharacterSize(13);
            ejText.setFillColor(Theme::Text);
            ejText.setPosition({ej.position.x + 45, ej.position.y + 5});
            window.draw(ejText);

            sf::FloatRect ex = exportXmlRect();
            sf::RectangleShape exBtn(ex.size);
            exBtn.setPosition(ex.position);
            exBtn.setFillColor(Theme::ButtonNormal);
            exBtn.setOutlineThickness(1);
            exBtn.setOutlineColor(Theme::Accent);
            window.draw(exBtn);
            sf::Text exText(font);
            exText.setFont(font);
            exText.setString("Exportar XML");
            exText.setCharacterSize(13);
            exText.setFillColor(Theme::Text);
            exText.setPosition({ex.position.x + 50, ex.position.y + 5});
            window.draw(exText);
        } else {
            listText.setCharacterSize(13);
            listText.setFillColor(Theme::Text);
            listText.setString("Filtrar por categoria:");
            listText.setPosition({ModalX + 20, ModalY + 45});
            window.draw(listText);

            float lcy = ModalY + 70;
            auto drawChip = [&](const std::string& label, const sf::FloatRect& r, bool selected) {
                sf::RectangleShape chip(r.size);
                chip.setPosition(r.position);
                chip.setFillColor(selected ? Theme::ButtonActive : Theme::ButtonNormal);
                chip.setOutlineThickness(1);
                chip.setOutlineColor(Theme::ModalBorder);
                window.draw(chip);
                sf::Text chipText(font);
                chipText.setFont(font);
                chipText.setString(label);
                chipText.setCharacterSize(12);
                chipText.setFillColor(Theme::Text);
                sf::FloatRect cb = chipText.getLocalBounds();
                float maxW = r.size.x - 8;
                if (cb.size.x > maxW) {
                    std::string s = label;
                    while (!s.empty() && chipText.getLocalBounds().size.x > maxW) {
                        s.pop_back();
                        chipText.setString(s + "..");
                    }
                }
                chipText.setPosition({r.position.x + 4, r.position.y + 4});
                window.draw(chipText);
            };
            drawChip("Todos", chipRect(0, lcy), loadCategorySel == 0);
            for (size_t i = 0; i < categories.size(); ++i) {
                drawChip(categories[i], chipRect(i + 1, lcy), loadCategorySel == (int)(i + 1));
            }

            listText.setString("Projetos salvos:");
            listText.setPosition({ModalX + 20, ModalY + 120});
            window.draw(listText);

            if (projectInfos.empty()) {
                listText.setString("Nenhum projeto encontrado nesta categoria.");
                listText.setPosition({ModalX + 25, LoadListBaseY + 10});
                listText.setFillColor(sf::Color(150, 150, 150));
                window.draw(listText);
                listText.setFillColor(Theme::Text);
            } else {
                for (size_t i = 0; i < projectInfos.size(); ++i) {
                    if ((int)i < listScroll || (int)i >= listScroll + LoadVisibleRows) continue;
                    const auto& info = projectInfos[i];
                    sf::FloatRect row = loadRowRect(i);
                    bool isHovered = row.contains(mousePosF);

                    if (isHovered || i == selectedProjectIndex) {
                        sf::RectangleShape sel(row.size);
                        sel.setPosition(row.position);
                        sel.setFillColor(sf::Color(255, 255, 255, 30));
                        if (i == selectedProjectIndex) sel.setOutlineThickness(1);
                        sel.setOutlineColor(Theme::Highlight);
                        window.draw(sel);
                        listText.setFillColor(Theme::Highlight);
                    } else {
                        listText.setFillColor(Theme::Text);
                    }

                    listText.setCharacterSize(15);
                    std::string dispName = info.name;
                    sf::FloatRect nb = listText.getLocalBounds();
                    float maxNameW = ModalW - 200;
                    if (nb.size.x > maxNameW) {
                        while (!dispName.empty() && listText.getLocalBounds().size.x > maxNameW) {
                            dispName.pop_back();
                            listText.setString(dispName + "..");
                        }
                    }
                    listText.setString(dispName);
                    listText.setPosition({ModalX + 25, row.position.y + 3});
                    window.draw(listText);

                    listText.setCharacterSize(11);
                    std::string meta = std::to_string(info.gateCount) + " comps | "
                        + std::to_string(info.wireCount) + " fios | "
                        + formatSize(info.size) + " | "
                        + formatFileTime(info.modified);
                    listText.setString(meta);
                    listText.setPosition({ModalX + 25, row.position.y + 23});
                    listText.setFillColor(sf::Color(180, 180, 200));
                    window.draw(listText);
                    listText.setFillColor(Theme::Text);

                    sf::FloatRect lb = loadRowLoadBtnRect(i);
                    sf::RectangleShape loadBtn(lb.size);
                    loadBtn.setPosition(lb.position);
                    loadBtn.setFillColor(isHovered ? Theme::Highlight : Theme::Accent);
                    window.draw(loadBtn);
                    sf::Text lbText(font);
                    lbText.setFont(font);
                    lbText.setString("Carregar");
                    lbText.setCharacterSize(12);
                    lbText.setFillColor(sf::Color::White);
                    lbText.setPosition({lb.position.x + 5, lb.position.y + 5});
                    window.draw(lbText);

                    sf::FloatRect db = loadRowDeleteBtnRect(i);
                    sf::RectangleShape delBtn(db.size);
                    delBtn.setPosition(db.position);
                    delBtn.setFillColor(sf::Color(200, 60, 60));
                    window.draw(delBtn);
                    sf::Text dbText(font);
                    dbText.setFont(font);
                    dbText.setString("Excluir");
                    dbText.setCharacterSize(12);
                    dbText.setFillColor(sf::Color::White);
                    dbText.setPosition({db.position.x + 12, db.position.y + 5});
                    window.draw(dbText);
                }
            }

            if (!projectInfos.empty() && selectedProjectIndex < projectInfos.size()) {
                listText.setCharacterSize(11);
                listText.setFillColor(sf::Color(160, 200, 255));
                std::string path = "Caminho: " + projectInfos[selectedProjectIndex].fullPath.string();
                sf::FloatRect pb = listText.getLocalBounds();
                float maxW = ModalW - 40;
                if (pb.size.x > maxW) {
                    while (!path.empty() && listText.getLocalBounds().size.x > maxW) {
                        path.pop_back();
                        listText.setString("Caminho: " + path + "..");
                    }
                }
                listText.setString(path);
                listText.setPosition({ModalX + 20, ModalY + ModalH - 28});
                window.draw(listText);
                listText.setFillColor(Theme::Text);
            }
        }
    }
}

void Workbench::update(float deltaTime) {
    if (!currentTooltip.empty()) tooltipTimer += deltaTime;

    if (statusMessageTimer > 0.0f) {
        statusMessageTimer -= deltaTime;
        if (statusMessageTimer < 0.0f) statusMessageTimer = 0.0f;
    }

    if (autoBackupEnabled && !currentProjectName.empty() && !isSaveModal && !isLoadModal) {
        backupTimer += deltaTime;
        if (backupTimer >= BackupInterval) {
            backupTimer = 0.0f;
            createBackup();
            statusMessage = "Backup automatico criado em projetos/backups";
            statusMessageTimer = 4.0f;
        }
    }

    float transitionSpeed = 10.0f * deltaTime;

    if (isMenuButtonHovered || isMenuButtonPressed) {
        menuButtonAlpha = std::min(255.0f, menuButtonAlpha + 255.0f * transitionSpeed);
    } else {
        menuButtonAlpha = std::max(200.0f, menuButtonAlpha - 255.0f * transitionSpeed);
    }

    if (showDropdown) {
        dropdownAlpha = std::min(255.0f, dropdownAlpha + 255.0f * transitionSpeed);
    } else {
        dropdownAlpha = std::max(0.0f, dropdownAlpha - 255.0f * transitionSpeed);
    }

    if (isSaveModal || isLoadModal) {
        modalAlpha = std::min(255.0f, modalAlpha + 255.0f * transitionSpeed);
    } else {
        modalAlpha = std::max(0.0f, modalAlpha - 255.0f * transitionSpeed);
    }
}

void Workbench::updatePropagation() {
    if (isSaveModal || isLoadModal) return;

    for (auto& gate : placedGates) {
        for (auto& pin : gate->inputs) pin.state = false;
    }

    for (auto& wire : wires) wire.update();

    for (auto& gate : placedGates) gate->updateLogic();
}

std::string Workbench::getFilterName() const {
    switch (currentFilter) {
        case SelectionFilter::ALL: return "All";
        case SelectionFilter::GATES_ONLY: return "Gates Only";
        case SelectionFilter::WIRES_ONLY: return "Wires Only";
        case SelectionFilter::AND_GATES: return "AND Gates";
        case SelectionFilter::OR_GATES: return "OR Gates";
        case SelectionFilter::NOT_GATES: return "NOT Gates";
        case SelectionFilter::CLOCK_GATES: return "Clock Gates";
        case SelectionFilter::POWER_SOURCES: return "Power Sources";
        default: return "Unknown";
    }
}

void Workbench::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (isSaveModal) {
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            if (textEntered->unicode < 128) {
                char c = static_cast<char>(textEntered->unicode);
                std::string& buf = (modalFocus == ModalFocus::NAME) ? inputBuffer : categoryBuffer;
                if (c == '\r' || c == '\n') {
                    if (!inputBuffer.empty()) {
                        fs::path dir = projectDir(categoryBuffer);
                        fs::create_directories(dir);
                        fs::path p = dir / (inputBuffer + ".gate");
                        isSaving = true;
                        saveProgress = 0.0f;
                        writeProject(p);
                        isSaving = false;
                        currentProjectName = inputBuffer;
                        currentCategory = categoryBuffer;
                        statusMessage = "Projeto salvo em:\n" + fs::absolute(p).string();
                        statusMessageTimer = 5.0f;
                        isSaveModal = false;
                        saveAsMode = false;
                        refreshCategories();
                    }
                } else if (c == '\b') {
                    if (!buf.empty()) buf.pop_back();
                } else if (std::isalnum(c) || c == '_' || c == '-' || c == ' ') {
                    if (buf.size() < 60) buf += c;
                }
            }
        }
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                isSaveModal = false;
                saveAsMode = false;
            } else if (keyPressed->code == sf::Keyboard::Key::Tab) {
                modalFocus = (modalFocus == ModalFocus::NAME) ? ModalFocus::CATEGORY : ModalFocus::NAME;
            }
        }
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                if (nameInputRect().contains(mousePosF)) modalFocus = ModalFocus::NAME;
                else if (categoryInputRect().contains(mousePosF)) modalFocus = ModalFocus::CATEGORY;
                else if (chipRect(0, ModalY + 175).contains(mousePosF)) {
                    categoryBuffer = "";
                    modalFocus = ModalFocus::CATEGORY;
                } else {
                    for (size_t i = 0; i < categories.size(); ++i) {
                        if (chipRect(i + 1, ModalY + 175).contains(mousePosF)) {
                            categoryBuffer = categories[i];
                            modalFocus = ModalFocus::CATEGORY;
                            break;
                        }
                    }
                }
                if (saveButtonRect().contains(mousePosF) && !inputBuffer.empty()) {
                    fs::path dir = projectDir(categoryBuffer);
                    fs::create_directories(dir);
                    fs::path p = dir / (inputBuffer + ".gate");
                    isSaving = true;
                    saveProgress = 0.0f;
                    writeProject(p);
                    isSaving = false;
                    currentProjectName = inputBuffer;
                    currentCategory = categoryBuffer;
                    statusMessage = "Projeto salvo em:\n" + fs::absolute(p).string();
                    statusMessageTimer = 5.0f;
                    isSaveModal = false;
                    saveAsMode = false;
                    refreshCategories();
                } else if (exportJsonRect().contains(mousePosF) && !inputBuffer.empty()) {
                    exportJSON(inputBuffer, categoryBuffer);
                } else if (exportXmlRect().contains(mousePosF) && !inputBuffer.empty()) {
                    exportXML(inputBuffer, categoryBuffer);
                }
            }
        }
        return;
    }

    if (isLoadModal) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) isLoadModal = false;
            else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                if (!projectInfos.empty()) {
                    load(projectInfos[selectedProjectIndex].name);
                    isLoadModal = false;
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Up) {
                if (selectedProjectIndex > 0) {
                    selectedProjectIndex--;
                    if ((int)selectedProjectIndex < listScroll) listScroll = (int)selectedProjectIndex;
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Down) {
                if (!projectInfos.empty() && selectedProjectIndex < projectInfos.size() - 1) {
                    selectedProjectIndex++;
                    if ((int)selectedProjectIndex >= listScroll + LoadVisibleRows)
                        listScroll = (int)selectedProjectIndex - LoadVisibleRows + 1;
                }
            }
        }
        if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical && !projectInfos.empty()) {
                int maxScroll = std::max(0, (int)projectInfos.size() - LoadVisibleRows);
                listScroll = std::clamp(listScroll + (mouseWheel->delta > 0 ? -1 : 1), 0, maxScroll);
            }
        }
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                if (chipRect(0, ModalY + 70).contains(mousePosF)) {
                    loadCategorySel = 0;
                    refreshProjectList();
                    return;
                }
                for (size_t i = 0; i < categories.size(); ++i) {
                    if (chipRect(i + 1, ModalY + 70).contains(mousePosF)) {
                        loadCategorySel = (int)(i + 1);
                        refreshProjectList();
                        return;
                    }
                }
                for (size_t i = 0; i < projectInfos.size(); ++i) {
                    if ((int)i < listScroll || (int)i >= listScroll + LoadVisibleRows) continue;
                    if (loadRowDeleteBtnRect(i).contains(mousePosF)) {
                        deleteProject(i);
                        return;
                    } else if (loadRowLoadBtnRect(i).contains(mousePosF)) {
                        load(projectInfos[i].name);
                        isLoadModal = false;
                        return;
                    } else if (loadRowRect(i).contains(mousePosF)) {
                        selectedProjectIndex = i;
                        return;
                    }
                }
            }
        }
        return;
    }

    isMenuButtonHovered = menuButton.getGlobalBounds().contains(mousePosF);
    isNewHovered = newText.getGlobalBounds().contains(mousePosF);
    isSaveHovered = saveText.getGlobalBounds().contains(mousePosF);
    isLoadHovered = loadText.getGlobalBounds().contains(mousePosF);
    isSaveAsHovered = saveAsText.getGlobalBounds().contains(mousePosF);

    if (isMenuButtonHovered && !showDropdown) {
        currentTooltip = "Open main menu";
    } else if (isNewHovered) {
        currentTooltip = "Create a new project (Ctrl+N)";
    } else if (isSaveHovered) {
        currentTooltip = "Save current project (Ctrl+S)";
    } else if (isLoadHovered) {
        currentTooltip = "Load an existing project (Ctrl+L)";
    } else if (isSaveAsHovered) {
        currentTooltip = "Save current project under a new name";
    } else {
        currentTooltip.clear();
    }

    showDropdown = menuBar.getGlobalBounds().contains(mousePosF) ||
        (showDropdown && dropdown.getGlobalBounds().contains(mousePosF));

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            isMenuButtonPressed = menuButton.getGlobalBounds().contains(mousePosF);
        }
        if (mousePressed->button == sf::Mouse::Button::Left) {
            if (showDropdown) {
                sf::FloatRect newBounds = newText.getGlobalBounds();
                sf::FloatRect saveBounds = saveText.getGlobalBounds();
                sf::FloatRect loadBounds = loadText.getGlobalBounds();
                sf::FloatRect saveAsBounds = saveAsText.getGlobalBounds();
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
                        categoryBuffer = currentCategory;
                        modalFocus = ModalFocus::NAME;
                        saveAsMode = false;
                        refreshCategories();
                    }
                    return;
                }
                if (saveAsBounds.contains(mousePosF)) {
                    isSaveModal = true;
                    inputBuffer = currentProjectName;
                    categoryBuffer = currentCategory;
                    modalFocus = ModalFocus::NAME;
                    saveAsMode = true;
                    refreshCategories();
                    return;
                }
                if (loadBounds.contains(mousePosF)) {
                    isLoadModal = true;
                    refreshCategories();
                    refreshProjectList();
                    return;
                }
            }

            for (auto& item : paletteItems) {
                if (item->contains(mousePosF)) {
                    std::shared_ptr<Gate> newGate;
                    switch (item->getType()) {
                        case GateType::AND:
                            newGate = std::make_shared<ANDGate>(mousePosF, font);
                            break;
                        case GateType::OR: 
                            newGate = std::make_shared<ORGate>(mousePosF, font);
                            break;
                        case GateType::NOT:
                            newGate = std::make_shared<NOTGate>(mousePosF, font);
                            break;
                        case GateType::CLOCK:
                            newGate = std::make_shared<ClockGate>(mousePosF, font);
                            break;
                        case GateType::POWER:
                            newGate = std::make_shared<PowerSource>(mousePosF, font);
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                isMultiSelecting = true;
            }

            for (auto& gate : placedGates) {
                for (auto& pin : gate->outputs) {
                    if (pin.contains(mousePosF)) {
                        isWiring = true;
                        selectedPin = &pin;
                        wireStartPos = selectedPin->shape.getPosition();
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

            bool clickedGate = false;
            for (auto it = placedGates.rbegin(); it != placedGates.rend(); ++it) {
                if ((*it)->contains(mousePosF)) {
                    clickedGate = true;
                    if ((*it)->getType() == GateType::POWER) dynamic_cast<PowerSource*>((*it).get())->toggle();
                    if ((*it)->getType() == GateType::CLOCK) dynamic_cast<ClockGate*>((*it).get())->toggleInterval();

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
                            g->dragOffset = mousePosF - g->getPosition();
                        }
                    }
                    break;
                }
            }

            bool clickedWire = false;
            if (!clickedGate && !isWiring && mousePosF.x > Config::PaletteWidth) {
                for (auto& wire : wires) {
                    if (wire.contains(mousePosF)) {
                        clickedWire = true;
                        for (auto& w : wires) w.isSelected = false;
                        for (auto& g : placedGates) g->isSelected = false;
                        wire.isSelected = true;
                        break;
                    }
                }
            }

            if (!clickedGate && !clickedWire && !isWiring && mousePosF.x > Config::PaletteWidth) {
                isSelecting = true;
                selectStart = mousePosF;
                for (auto& g : placedGates) g->isSelected = false;
                for (auto& w : wires) w.isSelected = false;
            }

            if (clickedGate) isWiring = false;
        }
        else if (mousePressed->button == sf::Mouse::Button::Right) {
            if (isWiring) {
                isWiring = false;
                selectedPin = nullptr;
            } else {
                for (auto& wire : wires) {
                    if (wire.contains(mousePosF)) wire.isSelected = false;
                }
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
            selectionRect.setFillColor(Config::SelectionRectFill);
            selectionRect.setOutlineColor(Config::SelectionRectOutline);
            selectionRect.setOutlineThickness(Config::SelectionRectOutlineThickness);
        }
        for (auto& gate : placedGates) {
            if (gate->isDragging) {
                sf::Vector2f newPos = mousePosF - gate->dragOffset;
                gate->setPosition(snapToGrid(newPos));
            }
        }

        tooltipTimer = 0.0f;
        currentTooltip.clear();
    } else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Delete) {
            saveState();
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
        } else if (keyPressed->code == sf::Keyboard::Key::D && keyPressed->control && !isSaveModal && !isLoadModal) {
            saveState();
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
                        case GateType::CLOCK:
                            newGate = std::make_shared<ClockGate>(gate->getPosition() + sf::Vector2f(20, 20), font);
                            break;
                        case GateType::POWER:
                            newGate = std::make_shared<PowerSource>(gate->getPosition() + sf::Vector2f(20, 20), font);
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
        } else if (keyPressed->code == sf::Keyboard::Key::Z && keyPressed->control && !isSaveModal && !isLoadModal) {
            undo();
        } else if (keyPressed->code == sf::Keyboard::Key::Y && keyPressed->control && !isSaveModal && !isLoadModal) {
            redo();
        } else if (keyPressed->code == sf::Keyboard::Key::N && keyPressed->control && !isSaveModal && !isLoadModal) {
            newProject();
        } else if (keyPressed->code == sf::Keyboard::Key::S && keyPressed->control && !isSaveModal && !isLoadModal) {
            if (!currentProjectName.empty()) {
                save(currentProjectName);
            } else {
                isSaveModal = true;
                inputBuffer = "";
                categoryBuffer = currentCategory;
                modalFocus = ModalFocus::NAME;
                saveAsMode = false;
                refreshCategories();
            }
        } else if (keyPressed->code == sf::Keyboard::Key::L && keyPressed->control && !isSaveModal && !isLoadModal) {
            isLoadModal = true;
            refreshCategories();
            refreshProjectList();
        } else if (keyPressed->code == sf::Keyboard::Key::A && keyPressed->control && !isSaveModal && !isLoadModal) {
            for (auto& g : placedGates) g->isSelected = true;
            for (auto& w : wires) w.isSelected = true;
        } else if (keyPressed->code == sf::Keyboard::Key::T && keyPressed->control && !isSaveModal && !isLoadModal) {
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
        } else if (keyPressed->code == sf::Keyboard::Key::G && keyPressed->control && !isSaveModal && !isLoadModal) {
            if (!placedGates.empty() || !wires.empty()) {
                saveState();
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
        } else if (keyPressed->code == sf::Keyboard::Key::U && keyPressed->control && !isSaveModal && !isLoadModal) {
            if (selectedGroup < groups.size()) {
                groups.erase(groups.begin() + selectedGroup);
                selectedGroup = SIZE_MAX;
            }
        }
    }
}

void Workbench::save(const std::string& name) {
    isSaving = true;
    saveProgress = 0.0f;

    fs::path dir = projectDir(currentCategory);
    fs::create_directories(dir);
    fs::path p = dir / (name + ".gate");
    writeProject(p);

    saveProgress = 1.0f;
    currentProjectName = name;
    statusMessage = "Projeto salvo em: " + fs::absolute(p).string();
    statusMessageTimer = 4.0f;
    isSaving = false;
}

void Workbench::renderImGui() {
    ImGui::Begin("Logic Gate Simulator");

    ImGui::Text("Gates placed: %zu", placedGates.size());
    ImGui::Text("Wires connected: %zu", wires.size());

    if (!currentProjectName.empty()) {
        ImGui::Text("Project: %s", currentProjectName.c_str());
        ImGui::Text("Category: %s", currentCategory.empty() ? "(raiz)" : currentCategory.c_str());
        ImGui::Text("Path: %s", projectDir(currentCategory).string().c_str());
    }

    ImGui::Separator();

    if (ImGui::Button("New Project")) newProject();
    if (ImGui::Button("Save Project")) {
        if (!currentProjectName.empty()) {
            save(currentProjectName);
        } else {
            isSaveModal = true;
            inputBuffer = "";
            categoryBuffer = currentCategory;
            modalFocus = ModalFocus::NAME;
            saveAsMode = false;
            refreshCategories();
        }
    }
    if (ImGui::Button("Save As...")) {
        isSaveModal = true;
        inputBuffer = currentProjectName;
        categoryBuffer = currentCategory;
        modalFocus = ModalFocus::NAME;
        saveAsMode = true;
        refreshCategories();
    }
    if (ImGui::Button("Load Project")) {
        isLoadModal = true;
        refreshCategories();
        refreshProjectList();
    }

    ImGui::Separator();

    if (ImGui::Button("Export JSON")) {
        if (!currentProjectName.empty()) exportJSON(currentProjectName, currentCategory);
    }
    if (ImGui::Button("Export XML")) {
        if (!currentProjectName.empty()) exportXML(currentProjectName, currentCategory);
    }

    ImGui::Separator();
    ImGui::Text("Auto-backup: %s", autoBackupEnabled ? "ON (5 min)" : "OFF");
    if (ImGui::Button("Create Backup Now")) createBackup();

    ImGui::End();
}

void Workbench::load(const std::string& name) {
    std::string cat = currentCategory;
    for (const auto& info : projectInfos) {
        if (info.name == name) {
            cat = info.category;
            break;
        }
    }
    fs::path p = projectDir(cat) / (name + ".gate");
    std::ifstream f(p);
    if (!f) return;
    placedGates.clear();
    wires.clear();
    size_t gateCount;
    if (!(f >> gateCount)) return;
    while (gateCount--) {
        int t;
        float x, y, extra;
        f >> t >> x >> y >> extra;
        sf::Vector2f ppos(x, y);
        std::shared_ptr<Gate> newGate;
        switch ((GateType)t) {
            case GateType::AND:
                newGate = std::make_shared<ANDGate>(ppos, font);
                break;
            case GateType::OR:
                newGate = std::make_shared<ORGate>(ppos, font);
                break;
            case GateType::NOT:
                newGate = std::make_shared<NOTGate>(ppos, font);
                break;
            case GateType::CLOCK:
                newGate = std::make_shared<ClockGate>(ppos, font);
                break;
            case GateType::POWER:
                newGate = std::make_shared<PowerSource>(ppos, font);
                break;
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
    currentCategory = cat;
}