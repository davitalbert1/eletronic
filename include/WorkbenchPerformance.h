#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "LogicEntities.h"
#include "Wire.h"

class WorkbenchPerformance {
public:
    WorkbenchPerformance() = default;

    // FPS Counter
    void updateFPS();
    float getCurrentFPS() const {
        return currentFPS_;
    }
    void drawFPS(sf::RenderWindow& window, bool show, bool performanceMode, const sf::Font& font);
    void resetFPS();

    // Performance mode
    void togglePerformanceMode();
    void updatePerformanceSettings();
    bool isPerformanceMode() const {
        return performanceMode_;
    }
    void setPerformanceMode(bool enabled) {
        performanceMode_ = enabled;
    }

    // LOD (Level of Detail)
    bool useLOD(size_t componentCount) const;
    int getLODLevel(sf::Vector2f gatePos, sf::Vector2f viewCenter) const;
    void updateLODSettings();

    // Error handling
    bool hasError() const {
        return hasError_;
    }
    const std::string& getLastError() const {
        return lastError_;
    }
    void handleError(const std::string& error);
    void clearError();

    // Validation
    bool validateProjectData(const std::vector<std::shared_ptr<Gate>>& placedGates,
                            const std::vector<Wire>& wires) const;
    bool attemptRecovery(std::vector<std::shared_ptr<Gate>>& placedGates,
                        std::vector<Wire>& wires);

    // Emergency backup
    void saveEmergencyBackup(const std::string& projectName,
                            const std::vector<std::shared_ptr<Gate>>& placedGates,
                            const std::vector<Wire>& wires);

    // Settings
    bool enableShadows = true;
    bool enableEffects = true;
    bool enableLOD = true;

private:
    // FPS
    sf::Clock fpsClock_;
    sf::Time fpsUpdateInterval_ = sf::seconds(0.5f);
    int frameCount_ = 0;
    float currentFPS_ = 0.0f;
    sf::Time fpsUpdateTimer_ = sf::Time::Zero;
    bool showFPS_ = true;

    // Performance mode
    bool performanceMode_ = false;

    // LOD
    float currentZoom_ = 1.0f;
    sf::Vector2f viewCenter_;

    // Error handling
    std::string lastError_;
    bool hasError_ = false;
};
