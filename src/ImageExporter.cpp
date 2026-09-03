#include "ImageExporter.h"
#include "Constants.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

ImageExportResult ImageExporter::exportToPNG(
    const std::string& filePath,
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires,
    int width, int height,
    const sf::Color& backgroundColor) {
    ImageExportResult result;
    sf::Image image = renderToImage(gates, wires, width, height, backgroundColor);
    if (!image.saveToFile(filePath)) {
        result.errorMessage = "Failed to save PNG file: " + filePath;
        return result;
    }
    result.success = true;
    result.filePath = filePath;
    result.width = width;
    result.height = height;
    return result;
}

ImageExportResult ImageExporter::exportToSVG(
    const std::string& filePath,
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires,
    int width, int height,
    const sf::Color& backgroundColor) {
    ImageExportResult result;
    std::string svgContent = generateSVGContent(gates, wires, width, height, backgroundColor);
    std::ofstream file(filePath);
    if (!file.is_open()) {
        result.errorMessage = "Failed to create SVG file: " + filePath;
        return result;
    }
    file << svgContent;
    file.close();
    result.success = true;
    result.filePath = filePath;
    result.width = width;
    result.height = height;
    return result;
}

sf::FloatRect ImageExporter::calculateBoundingBox(const std::vector<std::shared_ptr<Gate>>& gates, float padding) {
    if (gates.empty()) return sf::FloatRect({0, 0}, {200, 200});
    float minX = gates[0]->getPosition().x;
    float minY = gates[0]->getPosition().y;
    float maxX = minX + Config::GateWidth;
    float maxY = minY + Config::GateHeight;
    for (const auto& gate : gates) {
        sf::Vector2f pos = gate->getPosition();
        minX = std::min(minX, pos.x);
        minY = std::min(minY, pos.y);
        maxX = std::max(maxX, pos.x + Config::GateWidth);
        maxY = std::max(maxY, pos.y + Config::GateHeight);
    }
    return sf::FloatRect({minX - padding, minY - padding},
                         {maxX - minX + 2 * padding, maxY - minY + 2 * padding});
}

sf::Image ImageExporter::renderToImage(
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires,
    int width, int height,
    const sf::Color& backgroundColor) {
    sf::RenderTexture texture;
    if (!texture.resize({static_cast<unsigned>(width), static_cast<unsigned>(height)})) {
        return sf::Image();
    }
    texture.clear(backgroundColor);
    if (showGrid_) {
        sf::VertexArray gridLines(sf::PrimitiveType::Lines);
        for (int x = 0; x < width; x += gridSize_) {
            gridLines.append(sf::Vertex{{static_cast<float>(x), 0.0f}, sf::Color(255, 255, 255, 20)});
            gridLines.append(sf::Vertex{{static_cast<float>(x), static_cast<float>(height)}, sf::Color(255, 255, 255, 20)});
        }
        for (int y = 0; y < height; y += gridSize_) {
            gridLines.append(sf::Vertex{{0.0f, static_cast<float>(y)}, sf::Color(255, 255, 255, 20)});
            gridLines.append(sf::Vertex{{static_cast<float>(width), static_cast<float>(y)}, sf::Color(255, 255, 255, 20)});
        }
        texture.draw(gridLines);
    }
    for (const auto& wire : wires) wire.draw(texture);
    for (const auto& gate : gates) gate->draw(texture);
    texture.display();
    return texture.getTexture().copyToImage();
}

std::string ImageExporter::generateSVGContent(
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires,
    int width, int height,
    const sf::Color& backgroundColor) {
    std::ostringstream svg;
    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" ";
    svg << "width=\"" << width << "\" height=\"" << height << "\" ";
    svg << "viewBox=\"0 0 " << width << " " << height << "\">\n";
    svg << "  <rect width=\"100%\" height=\"100%\" fill=\"" << colorToString(backgroundColor) << "\"/>\n";
    svg << "  <g id=\"wires\">\n";
    for (const auto& wire : wires) svg << "    " << svgWire(wire) << "\n";
    svg << "  </g>\n";
    svg << "  <g id=\"gates\">\n";
    for (const auto& gate : gates) svg << "    " << svgGate(*gate) << "\n";
    svg << "  </g>\n";
    svg << "</svg>\n";
    return svg.str();
}

std::string ImageExporter::svgGate(const Gate& gate) {
    std::ostringstream svg;
    sf::Vector2f pos = gate.getPosition();
    svg << svgRect(pos.x, pos.y, Config::GateWidth, Config::GateHeight,
                   Theme::GateBody, Theme::GateOutline, Config::GateOutlineThickness);
    for (const auto& pin : gate.inputs) {
        sf::Vector2f pinPos = pin.shape.getPosition();
        svg << svgCircle(pinPos.x, pinPos.y, Config::PinRadius,
                        pin.state ? Theme::SignalOn : Theme::SignalOff, Theme::PinOutline, 2.0f);
    }
    for (const auto& pin : gate.outputs) {
        sf::Vector2f pinPos = pin.shape.getPosition();
        svg << svgCircle(pinPos.x, pinPos.y, Config::PinRadius,
                        pin.state ? Theme::SignalOn : Theme::SignalOff, Theme::PinOutline, 2.0f);
    }
    return svg.str();
}

std::string ImageExporter::svgWire(const Wire& wire) {
    sf::Vector2f start = wire.fromPin->shape.getPosition();
    sf::Vector2f end = wire.toPin->shape.getPosition();
    sf::Color wireColor = wire.fromPin->state ? Theme::WireOn : Theme::WireOff;
    return svgLine(start.x, start.y, end.x, end.y, wireColor, Config::WireThickness);
}

std::string ImageExporter::svgRect(float x, float y, float w, float h,
                                    const sf::Color& fill, const sf::Color& stroke, float strokeWidth) {
    std::ostringstream svg;
    svg << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << w << "\" height=\"" << h << "\" "
        << "fill=\"" << colorToString(fill) << "\" stroke=\"" << colorToString(stroke) << "\" "
        << "stroke-width=\"" << strokeWidth << "\" rx=\"3\"/>";
    return svg.str();
}

std::string ImageExporter::svgCircle(float cx, float cy, float r,
                                      const sf::Color& fill, const sf::Color& stroke, float strokeWidth) {
    std::ostringstream svg;
    svg << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\"" << r << "\" "
        << "fill=\"" << colorToString(fill) << "\" stroke=\"" << colorToString(stroke) << "\" "
        << "stroke-width=\"" << strokeWidth << "\"/>";
    return svg.str();
}

std::string ImageExporter::svgLine(float x1, float y1, float x2, float y2,
                                    const sf::Color& stroke, float strokeWidth) {
    std::ostringstream svg;
    svg << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" "
        << "stroke=\"" << colorToString(stroke) << "\" stroke-width=\"" << strokeWidth << "\" "
        << "stroke-linecap=\"round\"/>";
    return svg.str();
}

std::string ImageExporter::svgText(float x, float y, const std::string& text,
                                    const sf::Color& fill, int fontSize) {
    std::ostringstream svg;
    svg << "<text x=\"" << x << "\" y=\"" << y << "\" "
        << "fill=\"" << colorToString(fill) << "\" font-size=\"" << fontSize << "\" "
        << "font-family=\"Arial, sans-serif\" text-anchor=\"middle\" dominant-baseline=\"central\">"
        << text << "</text>";
    return svg.str();
}

std::string ImageExporter::colorToString(const sf::Color& color) {
    std::ostringstream ss;
    ss << "rgb(" << static_cast<int>(color.r) << ","
       << static_cast<int>(color.g) << "," << static_cast<int>(color.b) << ")";
    return ss.str();
}