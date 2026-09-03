#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"

struct PdfExportOptions {
    std::string title = "Circuit Documentation";
    std::string author = "Eletronic Simulator";
    std::string subject = "Digital Logic Circuit";
    std::string description;
    std::string category;
    std::vector<std::string> tags;
    bool includeStatistics = true;
    bool includeComponentList = true;
    bool includeWireList = true;
    bool includeCircuitImage = true;
    bool includeTimestamp = true;
    int imageWidth = 800;
    int imageHeight = 600;
};

struct PdfExportResult {
    bool success = false;
    std::string errorMessage;
    std::string filePath;
    int pageCount = 0;
};

class PdfExporter {
public:
    PdfExporter() = default;

    PdfExportResult exportToPDF(
        const std::string& filePath,
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires,
        const PdfExportOptions& options = PdfExportOptions()
    );

    void setOptions(const PdfExportOptions& options) {
        options_ = options;
    }

private:
    PdfExportOptions options_;

    std::string generatePdfContent(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires
    );

    std::string generateTitlePage(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires
    );
    std::string generateStatisticsPage(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires
    );
    std::string generateComponentListPage(
        const std::vector<std::shared_ptr<Gate>>& gates
    );
    std::string generateWireListPage(
        const std::vector<Wire>& wires,
        const std::vector<std::shared_ptr<Gate>>& gates
    );

    struct CircuitStatistics {
        int totalGates = 0;
        int totalWires = 0;
        int totalInputs = 0;
        int totalOutputs = 0;
        std::map<GateType, int> gateTypeCounts;
        float minX = 0, minY = 0, maxX = 0, maxY = 0;
    };

    CircuitStatistics calculateStatistics(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires
    );

    std::string gateTypeToString(GateType type);
    std::string escapePdfString(const std::string& str);
};
