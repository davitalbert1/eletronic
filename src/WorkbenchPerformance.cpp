#include "WorkbenchPerformance.h"
#include "Constants.h"

namespace fs = std::filesystem;

void WorkbenchPerformance::updateFPS() {
    frameCount_++;
    fpsUpdateTimer_ += fpsClock_.restart();

    if (fpsUpdateTimer_ >= fpsUpdateInterval_) {
        currentFPS_ = frameCount_ / fpsUpdateTimer_.asSeconds();
        frameCount_ = 0;
        fpsUpdateTimer_ = sf::Time::Zero;
    }
}

void WorkbenchPerformance::drawFPS(sf::RenderWindow& window, bool show, bool performanceMode, const sf::Font& font) {
    if (!show) return;

    sf::Text fpsText(font);
    fpsText.setString("FPS: " + std::to_string(static_cast<int>(currentFPS_)));
    fpsText.setCharacterSize(14);
    fpsText.setFillColor(performanceMode ? sf::Color(255, 255, 0) : sf::Color(100, 255, 100));
    fpsText.setPosition({1100, 10});
    window.draw(fpsText);

    if (performanceMode) {
        sf::Text perfText(font);
        perfText.setString("PERFORMANCE MODE");
        perfText.setCharacterSize(12);
        perfText.setFillColor(sf::Color(255, 200, 0));
        perfText.setPosition({1100, 30});
        window.draw(perfText);
    }
}

void WorkbenchPerformance::resetFPS() {
    frameCount_ = 0;
    currentFPS_ = 0.0f;
    fpsUpdateTimer_ = sf::Time::Zero;
}

void WorkbenchPerformance::togglePerformanceMode() {
    performanceMode_ = !performanceMode_;
    updatePerformanceSettings();
}

void WorkbenchPerformance::updatePerformanceSettings() {
    if (performanceMode_) {
        enableShadows = false;
        enableEffects = false;
        enableLOD = true;
    } else {
        enableShadows = Config::EnableShadows;
        enableEffects = Config::EnableEffects;
        enableLOD = Config::EnableLOD;
    }
}

bool WorkbenchPerformance::useLOD(size_t componentCount) const {
    if (!enableLOD) return false;
    return componentCount > Config::LODComponentThreshold;
}

int WorkbenchPerformance::getLODLevel(sf::Vector2f gatePos, sf::Vector2f viewCenter) const {
    if (!useLOD(100)) return 3; // Full detail

    float distance = std::abs(gatePos.x - viewCenter.x) + std::abs(gatePos.y - viewCenter.y);

    if (distance < Config::LODDistanceNear) return 3; // Full detail
    else if (distance < Config::LODDistanceFar) return 2; // Medium detail
    else return 1; // Minimal detail
}

void WorkbenchPerformance::updateLODSettings() {
    // LOD settings are automatically updated based on component count and zoom
}

void WorkbenchPerformance::handleError(const std::string& error) {
    hasError_ = true;
    lastError_ = error;
    std::cerr << "ERROR: " << error << std::endl;
}

void WorkbenchPerformance::clearError() {
    hasError_ = false;
    lastError_.clear();
}

bool WorkbenchPerformance::validateProjectData(const std::vector<std::shared_ptr<Gate>>& placedGates,
                                               const std::vector<Wire>& wires) const {
    if (placedGates.size() > 10000) return false;

    for (const auto& wire : wires) {
        if (!wire.fromPin || !wire.toPin) return false;

        bool fromFound = false;
        bool toFound = false;

        for (const auto& gate : placedGates) {
            for (const auto& pin : gate->outputs) {
                if (&pin == wire.fromPin) fromFound = true;
            }
            for (const auto& pin : gate->inputs) {
                if (&pin == wire.toPin) toFound = true;
            }
        }

        if (!fromFound || !toFound) return false;
    }

    return true;
}

bool WorkbenchPerformance::attemptRecovery(std::vector<std::shared_ptr<Gate>>& placedGates,
                                           std::vector<Wire>& wires) {
    if (!hasError_) return true;

    if (!validateProjectData(placedGates, wires)) {
        wires.erase(
            std::remove_if(wires.begin(), wires.end(), [](const Wire& w) {
                return !w.fromPin || !w.toPin;
            }),
            wires.end()
        );

        placedGates.erase(
            std::remove_if(placedGates.begin(), placedGates.end(), [](const auto& g) {
                return !g;
            }),
            placedGates.end()
        );

        return true;
    }

    return false;
}

void WorkbenchPerformance::saveEmergencyBackup(const std::string& projectName,
                                               const std::vector<std::shared_ptr<Gate>>& placedGates,
                                               const std::vector<Wire>& wires) {
    if (projectName.empty()) return;

    try {
        fs::path backupDir = "projetos/backups";
        fs::create_directories(backupDir);

        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_s(&tm, &tt);
        char ts[32];
        std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S_EMERGENCY", &tm);

        fs::path p = backupDir / (projectName + "_" + ts + ".gate");
        std::ofstream f(p);
        if (!f) return;

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

        f << wires.size() << "\n";
        for (auto& w : wires) {
            int gFrom = getGateIdx(w.fromPin->parent);
            int pFrom = getPinIdx(w.fromPin->parent, w.fromPin, false);
            int gTo = getGateIdx(w.toPin->parent);
            int pTo = getPinIdx(w.toPin->parent, w.toPin, true);
            f << gFrom << " " << pFrom << " " << gTo << " " << pTo << "\n";
        }
        f.close();
    } catch (const std::exception& e) {
        std::cerr << "Failed to save emergency backup: " << e.what() << std::endl;
    }
}