#pragma once
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"

struct ImageExportResult {
    bool success = false;
    std::string errorMessage;
    std::string filePath;
    int width = 0;
    int height = 0;
};

class ImageExporter {
public:
    ImageExporter() = default;

    ImageExportResult exportToPNG(
        const std::string& filePath,
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires,
        int width = 1200,
        int height = 800,
        const sf::Color& backgroundColor = sf::Color(30, 30, 45)
    );

    ImageExportResult exportToSVG(
        const std::string& filePath,
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires,
        int width = 1200,
        int height = 800,
        const sf::Color& backgroundColor = sf::Color(30, 30, 45)
    );

    static sf::FloatRect calculateBoundingBox(
        const std::vector<std::shared_ptr<Gate>>& gates,
        float padding = 50.0f
    );

    void setScale(float scale) {
        scale_ = scale;
    }
    void setShowGrid(bool show) {
        showGrid_ = show;
    }
    void setGridSize(int size) {
        gridSize_ = size;
    }

private:
    float scale_ = 1.0f;
    bool showGrid_ = false;
    int gridSize_ = 20;

    sf::Image renderToImage(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires,
        int width, int height,
        const sf::Color& backgroundColor
    );

    std::string generateSVGContent(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires,
        int width, int height,
        const sf::Color& backgroundColor
    );

    std::string svgGate(const Gate& gate);
    std::string svgWire(const Wire& wire);
    std::string svgRect(float x, float y, float w, float h,
                        const sf::Color& fill, const sf::Color& stroke, float strokeWidth);
    std::string svgCircle(float cx, float cy, float r,
                        const sf::Color& fill, const sf::Color& stroke, float strokeWidth);
    std::string svgLine(float x1, float y1, float x2, float y2,
                        const sf::Color& stroke, float strokeWidth);
    std::string svgText(float x, float y, const std::string& text,
                        const sf::Color& fill, int fontSize);
    std::string colorToString(const sf::Color& color);
};
