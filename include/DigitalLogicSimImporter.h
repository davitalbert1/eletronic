#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"
#include "chip/Json.h"

struct DLSComponent {
    std::string type;
    std::string label;
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;// RotaÃ§Ã£o em graus
    std::map<std::string, std::string> properties;
    int id = -1;
};

// Estrutura que representa uma conexÃ£o no formato DLS
struct DLSConnection {
    int fromComponent = -1; // ID do componente de origem
    int fromPin = 0; // Ãndice do pino de saÃ­da
    int toComponent = -1; // ID do componente de destino
    int toPin = 0; // Ãndice do pino de entrada
};

// Estrutura que representa um projeto DLS completo
struct DLSProject {
    std::string name;
    std::string description;
    int version = 1;
    std::vector<DLSComponent> components;
    std::vector<DLSConnection> connections;
};

// Resultado da importaÃ§Ã£o
struct ImportResult {
    bool success = false;
    std::string errorMessage;
    std::vector<std::shared_ptr<Gate>> gates;
    std::vector<Wire> wires;
    std::string projectName;
    std::vector<std::string> unsupportedComponents; // Componentes nÃ£o convertidos
    int totalComponents = 0;
    int convertedComponents = 0;
    int totalConnections = 0;
    int convertedConnections = 0;
};

class DigitalLogicSimImporter {
public:
    DigitalLogicSimImporter() = default;
    ImportResult importFromFile(const std::string& filePath, const sf::Font& font);
    ImportResult importFromJson(const std::string& jsonText, const sf::Font& font);
    static std::vector<int> supportedVersions();
    static std::string formatName();

private:
    bool validateJson(const chip::JsonValue& root, std::string& errorMsg);
    bool adaptVersion(DLSProject& project, int version);
    DLSProject parseProject(const chip::JsonValue& root);
    GateType mapComponentType(const DLSComponent& dlsComp, std::string& outLabel, float& outExtra);
    bool createGates(const DLSProject& project,
                    std::vector<std::shared_ptr<Gate>>& gates,
                    std::vector<std::string>& unsupported,
                    const sf::Font& font);
    bool createWires(const DLSProject& project,
                    const std::vector<std::shared_ptr<Gate>>& gates,
                    std::vector<Wire>& wires,
                    int& convertedConnections);

    // Mapa de nomes DLS â†’ GateType (extensÃ­vel)
    static std::map<std::string, GateType> buildComponentMap();
    static std::map<std::string, GateType> componentMap_;

    std::map<int, size_t> dlsIdToGateIndex_;
};
