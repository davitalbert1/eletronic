#include "PdfExporter.h"
#include "GateRegistry.h"
#include "Constants.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

PdfExportResult PdfExporter::exportToPDF(
    const std::string& filePath,
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires,
    const PdfExportOptions& options) {

    options_ = options;
    PdfExportResult result;
    std::string pdfContent = generatePdfContent(gates, wires);

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        result.errorMessage = "Failed to create PDF file: " + filePath;
        return result;
    }
    file << pdfContent;
    file.close();
    result.success = true;
    result.filePath = filePath;
    result.pageCount = 4;
    return result;
}

std::string PdfExporter::generatePdfContent(
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires) {

    std::ostringstream output;
    output << "%PDF-1.4\n";

    size_t obj1Offset = output.tellp();
    output << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";

    size_t obj2Offset = output.tellp();
    output << "2 0 obj\n<< /Type /Pages /Kids [3 0 R 5 0 R 7 0 R 9 0 R] /Count 4 >>\nendobj\n";

    std::string content1 = generateTitlePage(gates, wires);
    size_t obj3Offset = output.tellp();
    output << "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] "
           << "/Contents 4 0 R /Resources << /Font << /F1 11 0 R >> >> >>\nendobj\n";
    size_t obj4Offset = output.tellp();
    output << "4 0 obj\n<< /Length " << content1.size() << " >>\nstream\n"
           << content1 << "\nendstream\nendobj\n";

    std::string content2 = generateStatisticsPage(gates, wires);
    size_t obj5Offset = output.tellp();
    output << "5 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] "
           << "/Contents 6 0 R /Resources << /Font << /F1 11 0 R >> >> >>\nendobj\n";
    size_t obj6Offset = output.tellp();
    output << "6 0 obj\n<< /Length " << content2.size() << " >>\nstream\n"
           << content2 << "\nendstream\nendobj\n";

    std::string content3 = generateComponentListPage(gates);
    size_t obj7Offset = output.tellp();
    output << "7 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] "
           << "/Contents 8 0 R /Resources << /Font << /F1 11 0 R >> >> >>\nendobj\n";
    size_t obj8Offset = output.tellp();
    output << "8 0 obj\n<< /Length " << content3.size() << " >>\nstream\n"
           << content3 << "\nendstream\nendobj\n";

    std::string content4 = generateWireListPage(wires, gates);
    size_t obj9Offset = output.tellp();
    output << "9 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] "
           << "/Contents 10 0 R /Resources << /Font << /F1 11 0 R >> >> >>\nendobj\n";
    size_t obj10Offset = output.tellp();
    output << "10 0 obj\n<< /Length " << content4.size() << " >>\nstream\n"
           << content4 << "\nendstream\nendobj\n";

    size_t obj11Offset = output.tellp();
    output << "11 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n";

    size_t xrefOffset = output.tellp();
    output << "xref\n0 12\n";
    output << "0000000000 65535 f \n";

    size_t offsets[] = {0, obj1Offset, obj2Offset, obj3Offset, obj4Offset,
                        obj5Offset, obj6Offset, obj7Offset, obj8Offset,
                        obj9Offset, obj10Offset, obj11Offset};
    for (int i = 1; i <= 11; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "%010zu 00000 n \n", offsets[i]);
        output << buf;
    }

    output << "trailer\n<< /Size 12 /Root 1 0 R >>\nstartxref\n" << xrefOffset << "\n%%EOF";
    return output.str();
}

std::string PdfExporter::generateTitlePage(
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires) {

    std::ostringstream ss;
    ss << "BT /F1 24 Tf 50 750 Td (" << escapePdfString(options_.title) << ") Tj ET\n";
    ss << "BT /F1 14 Tf 50 700 Td (Digital Logic Circuit Documentation) Tj ET\n";

    if (!options_.description.empty()) {
        ss << "BT /F1 12 Tf 50 650 Td (Description: " << escapePdfString(options_.description) << ") Tj ET\n";
    }
    if (!options_.category.empty()) {
        ss << "BT /F1 12 Tf 50 620 Td (Category: " << escapePdfString(options_.category) << ") Tj ET\n";
    }
    if (!options_.tags.empty()) {
        std::string tagsStr;
        for (size_t i = 0; i < options_.tags.size(); i++) {
            if (i > 0) tagsStr += ", ";
            tagsStr += options_.tags[i];
        }
        ss << "BT /F1 12 Tf 50 590 Td (Tags: " << escapePdfString(tagsStr) << ") Tj ET\n";
    }
    if (options_.includeTimestamp) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        char timeStr[64];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tm);
        ss << "BT /F1 12 Tf 50 550 Td (Generated: " << timeStr << ") Tj ET\n";
    }
    ss << "BT /F1 12 Tf 50 520 Td (Author: " << escapePdfString(options_.author) << ") Tj ET\n";
    ss << "BT /F1 16 Tf 50 450 Td (Summary) Tj ET\n";
    ss << "BT /F1 12 Tf 50 420 Td (Total Components: " << std::to_string(gates.size()) << ") Tj ET\n";
    ss << "BT /F1 12 Tf 50 400 Td (Total Connections: " << std::to_string(wires.size()) << ") Tj ET\n";
    return ss.str();
}

std::string PdfExporter::generateStatisticsPage(
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires) {

    std::ostringstream ss;
    ss << "BT /F1 20 Tf 50 780 Td (Circuit Statistics) Tj ET\n";

    CircuitStatistics stats = calculateStatistics(gates, wires);
    int y = 730;
    ss << "BT /F1 14 Tf 50 " << y << " Td (Gate Types:) Tj ET\n";
    y -= 25;

    for (const auto& pair : stats.gateTypeCounts) {
        ss << "BT /F1 11 Tf 70 " << y << " Td ("
           << escapePdfString(gateTypeToString(pair.first)) << ": "
           << std::to_string(pair.second) << ") Tj ET\n";
        y -= 18;
    }
    y -= 20;
    ss << "BT /F1 14 Tf 50 " << y << " Td (Totals:) Tj ET\n";
    y -= 25;
    ss << "BT /F1 11 Tf 70 " << y << " Td (Total Gates: " << std::to_string(stats.totalGates) << ") Tj ET\n";
    y -= 18;
    ss << "BT /F1 11 Tf 70 " << y << " Td (Total Inputs: " << std::to_string(stats.totalInputs) << ") Tj ET\n";
    y -= 18;
    ss << "BT /F1 11 Tf 70 " << y << " Td (Total Outputs: " << std::to_string(stats.totalOutputs) << ") Tj ET\n";
    y -= 18;
    ss << "BT /F1 11 Tf 70 " << y << " Td (Total Wires: " << std::to_string(stats.totalWires) << ") Tj ET\n";
    return ss.str();
}

std::string PdfExporter::generateComponentListPage(
    const std::vector<std::shared_ptr<Gate>>& gates) {

    std::ostringstream ss;
    ss << "BT /F1 20 Tf 50 780 Td (Component List) Tj ET\n";
    int y = 740;
    int index = 1;
    for (const auto& gate : gates) {
        if (y < 50) break;
        std::string typeStr = gateTypeToString(gate->getType());
        sf::Vector2f pos = gate->getPosition();
        ss << "BT /F1 9 Tf 50 " << y << " Td (" << std::to_string(index) << ". "
           << escapePdfString(typeStr) << " at (" << std::to_string(static_cast<int>(pos.x))
           << ", " << std::to_string(static_cast<int>(pos.y)) << ")) Tj ET\n";
        y -= 14;
        index++;
    }
    return ss.str();
}

std::string PdfExporter::generateWireListPage(
    const std::vector<Wire>& wires,
    const std::vector<std::shared_ptr<Gate>>& gates) {

    std::ostringstream ss;
    ss << "BT /F1 20 Tf 50 780 Td (Connection List) Tj ET\n";
    int y = 740;
    int index = 1;
    for (const auto& wire : wires) {
        if (y < 50) break;
        int fromIdx = -1, toIdx = -1;
        for (size_t i = 0; i < gates.size(); i++) {
            if (gates[i].get() == wire.fromPin->parent) fromIdx = static_cast<int>(i);
            if (gates[i].get() == wire.toPin->parent) toIdx = static_cast<int>(i);
        }
        ss << "BT /F1 9 Tf 50 " << y << " Td (" << std::to_string(index) << ". Gate["
           << std::to_string(fromIdx) << "] -> Gate[" << std::to_string(toIdx) << "]) Tj ET\n";
        y -= 14;
        index++;
    }
    return ss.str();
}

PdfExporter::CircuitStatistics PdfExporter::calculateStatistics(
    const std::vector<std::shared_ptr<Gate>>& gates,
    const std::vector<Wire>& wires) {

    CircuitStatistics stats;
    stats.totalGates = static_cast<int>(gates.size());
    stats.totalWires = static_cast<int>(wires.size());

    if (!gates.empty()) {
        stats.minX = gates[0]->getPosition().x;
        stats.minY = gates[0]->getPosition().y;
        stats.maxX = stats.minX + Config::GateWidth;
        stats.maxY = stats.minY + Config::GateHeight;
    }
    for (const auto& gate : gates) {
        GateType type = gate->getType();
        stats.gateTypeCounts[type]++;
        if (type == GateType::INPUT_PIN) stats.totalInputs++;
        if (type == GateType::OUTPUT_PIN) stats.totalOutputs++;
        sf::Vector2f pos = gate->getPosition();
        stats.minX = std::min(stats.minX, pos.x);
        stats.minY = std::min(stats.minY, pos.y);
        stats.maxX = std::max(stats.maxX, pos.x + Config::GateWidth);
        stats.maxY = std::max(stats.maxY, pos.y + Config::GateHeight);
    }
    return stats;
}

std::string PdfExporter::gateTypeToString(GateType type) {
    return GateRegistry::instance().getCodeName(type);
}

std::string PdfExporter::escapePdfString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '(':
              result += "\\(";
              break;
            case ')':
              result += "\\)";
              break;
            case '\\':
              result += "\\\\";
              break;
            default:
              result += c;
              break;
        }
    }
    return result;
}