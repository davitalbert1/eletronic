#include "DigitalLogicSimImporter.h"
#include "LogicEntities.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <string>

// Inicialização do mapa estático de componentes
std::map<std::string, GateType> DigitalLogicSimImporter::componentMap_ = DigitalLogicSimImporter::buildComponentMap();

std::map<std::string, GateType> DigitalLogicSimImporter::buildComponentMap() {
    std::map<std::string, GateType> m;

    // Portas lógicas básicas
    m["AND"] = GateType::AND;
    m["OR"] = GateType::OR;
    m["NOT"] = GateType::NOT;
    m["XOR"] = GateType::XOR;
    m["NAND"] = GateType::NAND;
    m["NOR"] = GateType::NOR;
    m["XNOR"] = GateType::XNOR;

    // Entradas e saídas
    m["Input"] = GateType::POWER;
    m["IN"] = GateType::POWER;
    m["Output"] = GateType::LED;
    m["OUT"] = GateType::LED;

    // Clock
    m["Clock"] = GateType::CLOCK;

    // Chaves e botões
    m["Switch"] = GateType::SWITCH;
    m["Button"] = GateType::BUTTON;

    // Display
    m["LED"] = GateType::LED;
    m["7-Segment"] = GateType::SEG7;
    m["7SEG"] = GateType::SEG7;
    m["SevenSegment"] = GateType::SEG7;
    m["LCD"] = GateType::LCD;

    // Memória e contadores
    m["ROM"] = GateType::ROM;
    m["Counter"] = GateType::COUNTER;
    m["Register"] = GateType::REGISTER;

    return m;
}

std::vector<int> DigitalLogicSimImporter::supportedVersions() {
    return {1, 2};
}

std::string DigitalLogicSimImporter::formatName() {
    return "Digital Logic Sim";
}

bool DigitalLogicSimImporter::validateJson(const chip::JsonValue& root, std::string& errorMsg) {
    // Verifica se é um objeto
    if (!root.isObject()) {
        errorMsg = "O arquivo JSON não contém um objeto válido.";
        return false;
    }

    // Verifica campos obrigatórios
    if (!root.has("components") && !root.has("gates")) {
        errorMsg = "Formato inválido: campo 'components' ou 'gates' não encontrado.";
        return false;
    }

    // Verifica versão (se presente)
    if (root.has("version")) {
        int ver = root.get("version").asInt(0);
        auto supported = supportedVersions();
        if (std::find(supported.begin(), supported.end(), ver) == supported.end()) {
            errorMsg = "Versão " + std::to_string(ver) + " do formato Digital Logic Sim não é suportada. "
                       "Versões suportadas: 1, 2.";
            return false;
        }
    }

    return true;
}

bool DigitalLogicSimImporter::adaptVersion(DLSProject& project, int version) {
    // Versão 1: formato básico
    // Versão 2: pode ter campos adicionais como 'rotation' em graus
    // Por enquanto, o parser já lida com ambos os formatos.
    // Futuras adaptações podem ser adicionadas aqui.
    (void)project;
    (void)version;
    return true;
}

DLSProject DigitalLogicSimImporter::parseProject(const chip::JsonValue& root) {
    DLSProject project;
    project.name = root.get("name").asString("Imported Project");
    project.description = root.get("description").asString("");
    project.version = root.get("version").asInt(1);

    // O DLS pode usar "components" ou "gates" como nome do array
    const chip::JsonValue& compArray = root.has("components") ? root.get("components") : root.get("gates");

    if (compArray.isArray()) {
        for (size_t i = 0; i < compArray.arr.size(); ++i) {
            const auto& jc = compArray.arr[i];
            DLSComponent comp;

            // ID do componente (pode ser "id" ou "index")
            comp.id = jc.get("id").asInt(jc.get("index").asInt(-1));
            if (comp.id < 0) comp.id = static_cast<int>(i);

            // Tipo do componente
            comp.type = jc.get("type").asString(jc.get("componentType").asString("Unknown"));

            // Rótulo
            comp.label = jc.get("label").asString(jc.get("name").asString(""));

            // Posição
            comp.x = static_cast<float>(jc.get("x").asNumber(0.0));
            comp.y = static_cast<float>(jc.get("y").asNumber(0.0));

            // Rotação (pode estar em "rotation" ou "orientation")
            comp.rotation = static_cast<float>(jc.get("rotation").asNumber(jc.get("orientation").asNumber(0.0)));

            // Propriedades adicionais
            if (jc.has("properties") && jc.get("properties").isObject()) {
                const auto& props = jc.get("properties");
                for (const auto& kv : props.obj) {
                    comp.properties[kv.first] = kv.second.asString();
                }
            }

            // Propriedades individuais comuns
            if (jc.has("value")) comp.properties["value"] = std::to_string(jc.get("value").asInt(0));
            if (jc.has("state")) comp.properties["state"] = jc.get("state").asString("off");
            if (jc.has("bits")) comp.properties["bits"] = std::to_string(jc.get("bits").asInt(1));
            if (jc.has("frequency")) comp.properties["frequency"] = std::to_string(jc.get("frequency").asNumber(1.0));

            project.components.push_back(comp);
        }
    }

    // Conexões (podem estar em "connections" ou "wires")
    const chip::JsonValue& connArray = root.has("connections") ? root.get("connections") : root.get("wires");

    if (connArray.isArray()) {
        for (size_t i = 0; i < connArray.arr.size(); ++i) {
            const auto& jw = connArray.arr[i];
            DLSConnection conn;

            if (jw.has("from") && jw.get("from").isObject()) {
                conn.fromComponent = jw.get("from").get("component").asInt(jw.get("from").get("id").asInt(-1));
                conn.fromPin = jw.get("from").get("pin").asInt(jw.get("from").get("port").asInt(0));
            } else if (jw.has("source") && jw.get("source").isObject()) {
                conn.fromComponent = jw.get("source").get("id").asInt(-1);
                conn.fromPin = jw.get("source").get("port").asInt(0);
            } else {
                conn.fromComponent = jw.get("fromComponent").asInt(jw.get("fromGate").asInt(-1));
                conn.fromPin = jw.get("fromPin").asInt(0);
            }

            if (jw.has("to") && jw.get("to").isObject()) {
                conn.toComponent = jw.get("to").get("component").asInt(jw.get("to").get("id").asInt(-1));
                conn.toPin = jw.get("to").get("pin").asInt(jw.get("to").get("port").asInt(0));
            } else if (jw.has("target") && jw.get("target").isObject()) {
                conn.toComponent = jw.get("target").get("id").asInt(-1);
                conn.toPin = jw.get("target").get("port").asInt(0);
            } else {
                conn.toComponent = jw.get("toComponent").asInt(
                    jw.get("toGate").asInt(-1));
                conn.toPin = jw.get("toPin").asInt(0);
            }

            project.connections.push_back(conn);
        }
    }

    return project;
}

GateType DigitalLogicSimImporter::mapComponentType(const DLSComponent& dlsComp, std::string& outLabel, float& outExtra) {
    outLabel = dlsComp.label;
    outExtra = 0.0f;

    // Busca no mapa de componentes
    auto it = componentMap_.find(dlsComp.type);
    if (it != componentMap_.end()) {
        GateType gt = it->second;

        // Configura extra data baseado no tipo e propriedades
        if (gt == GateType::POWER) {
            // DLS Input: verifica se tem estado inicial
            auto stateIt = dlsComp.properties.find("state");
            if (stateIt != dlsComp.properties.end()) {
                outExtra = (stateIt->second == "on" || stateIt->second == "1" ||
                           stateIt->second == "true") ? 1.0f : 0.0f;
            }
            // Se tiver label, usa como nome
            if (!dlsComp.label.empty()) {
                outLabel = dlsComp.label;
            } else {
                outLabel = "POWER";
            }
        } else if (gt == GateType::LED) {
            // DLS Output: verifica estado inicial
            auto stateIt = dlsComp.properties.find("state");
            if (stateIt != dlsComp.properties.end()) {
                outExtra = (stateIt->second == "on" || stateIt->second == "1") ? 1.0f : 0.0f;
            }
            if (!dlsComp.label.empty()) {
                outLabel = dlsComp.label;
            } else {
                outLabel = "LED";
            }
        } else if (gt == GateType::CLOCK) {
            // DLS Clock: frequência → intervalo
            auto freqIt = dlsComp.properties.find("frequency");
            if (freqIt != dlsComp.properties.end()) {
                float freq = std::stof(freqIt->second);
                if (freq > 0.0f) outExtra = 1.0f / freq; // Converte frequência para intervalo
            }
            auto valIt = dlsComp.properties.find("value");
            if (valIt != dlsComp.properties.end()) outExtra = std::stof(valIt->second);
        } else if (gt == GateType::SWITCH) {
            auto stateIt = dlsComp.properties.find("state");
            if (stateIt != dlsComp.properties.end()) {
                outExtra = (stateIt->second == "on" || stateIt->second == "1") ? 1.0f : 0.0f;
            }
        } else if (gt == GateType::COUNTER || gt == GateType::REGISTER) {
            auto valIt = dlsComp.properties.find("value");
            if (valIt != dlsComp.properties.end()) outExtra = std::stof(valIt->second);
        } else if (gt == GateType::ROM) {
            auto valIt = dlsComp.properties.find("value");
            if (valIt != dlsComp.properties.end()) outExtra = std::stof(valIt->second);
        }

        return gt;
    }

    // Se não encontrou, retorna GateType::AND como fallback e marca como não suportado
    outLabel = dlsComp.type + " (unsupported)";
    return GateType::AND;
}

bool DigitalLogicSimImporter::createGates(const DLSProject& project,
                                           std::vector<std::shared_ptr<Gate>>& gates,
                                           std::vector<std::string>& unsupported,
                                           const sf::Font& font) {
    dlsIdToGateIndex_.clear();

    for (const auto& dlsComp : project.components) {
        std::string label;
        float extra = 0.0f;
        GateType gt = mapComponentType(dlsComp, label, extra);

        // Cria o gate na posição do DLS
        sf::Vector2f pos(dlsComp.x, dlsComp.y);
        std::shared_ptr<Gate> gate;

        switch (gt) {
            case GateType::AND:
                gate = std::make_shared<ANDGate>(pos, font);
                break;
            case GateType::OR:
                gate = std::make_shared<ORGate>(pos, font);
                break;
            case GateType::NOT:
                gate = std::make_shared<NOTGate>(pos, font);
                break;
            case GateType::XOR:
                gate = std::make_shared<XORGate>(pos, font);
                break;
            case GateType::NAND:
                gate = std::make_shared<NANDGate>(pos, font);
                break;
            case GateType::NOR:
                gate = std::make_shared<NORGate>(pos, font);
                break;
            case GateType::XNOR:
                gate = std::make_shared<XNORGate>(pos, font);
                break;
            case GateType::CLOCK:
                gate = std::make_shared<ClockGate>(pos, font);
                break;
            case GateType::POWER:
                gate = std::make_shared<PowerSource>(pos, font);
                break;
            case GateType::SWITCH:
                gate = std::make_shared<Switch>(pos, font);
                break;
            case GateType::BUTTON:
                gate = std::make_shared<Button>(pos, font);
                break;
            case GateType::LED:
                gate = std::make_shared<LED>(pos, font);
                break;
            case GateType::ROM:
                gate = std::make_shared<ROM>(pos, font);
                break;
            case GateType::SEG7:
                gate = std::make_shared<Seg7Display>(pos, font);
                break;
            case GateType::LCD:
                gate = std::make_shared<LCDDisplay>(pos, font);
                break;
            case GateType::COUNTER:
                gate = std::make_shared<Counter>(pos, font);
                break;
            case GateType::REGISTER:
                gate = std::make_shared<Register>(pos, font);
                break;
            default:
                // Componente não suportado: adiciona à lista e usa AND como fallback
                unsupported.push_back(dlsComp.type + " (\"" + dlsComp.label + "\")");
                gate = std::make_shared<ANDGate>(pos, font);
                break;
        }

        if (gate) {
            // Aplica rotação
            if (dlsComp.rotation != 0.0f) {
                gate->setRotation(dlsComp.rotation);
            }

            // Aplica dados extras (estado, valor, etc.)
            gate->setExtraData(extra);

            // Se o componente DLS tem label, tenta preservar
            if (!dlsComp.label.empty()) {
                // Para PowerSource e Switch, o label é gerenciado internamente
                if (gt != GateType::POWER && gt != GateType::SWITCH) {
                    // Não sobrescrevemos labels internos para não quebrar a lógica
                }
            }

            gates.push_back(gate);

            // Mapeia ID DLS → índice no vetor de gates
            if (dlsComp.id >= 0) {
                dlsIdToGateIndex_[dlsComp.id] = gates.size() - 1;
            }
        }
    }

    return true;
}

// Reconstrução das conexões
bool DigitalLogicSimImporter::createWires(const DLSProject& project,
                                           const std::vector<std::shared_ptr<Gate>>& gates,
                                           std::vector<Wire>& wires,
                                           int& convertedConnections) {
    convertedConnections = 0;

    for (const auto& conn : project.connections) {
        // Busca os índices dos gates pelos IDs DLS
        auto fromIt = dlsIdToGateIndex_.find(conn.fromComponent);
        auto toIt = dlsIdToGateIndex_.find(conn.toComponent);

        if (fromIt == dlsIdToGateIndex_.end() || toIt == dlsIdToGateIndex_.end()) {
            continue; // Componente não encontrado, pula conexão
        }

        size_t fromIdx = fromIt->second;
        size_t toIdx = toIt->second;

        if (fromIdx >= gates.size() || toIdx >= gates.size()) {
            continue;
        }

        auto& fromGate = gates[fromIdx];
        auto& toGate = gates[toIdx];

        // Verifica se os pinos existem
        if (conn.fromPin >= 0 && conn.fromPin < (int)fromGate->outputs.size() &&
            conn.toPin >= 0 && conn.toPin < (int)toGate->inputs.size()) {
            Pin* fromPin = &fromGate->outputs[conn.fromPin];
            Pin* toPin = &toGate->inputs[conn.toPin];

            wires.emplace_back(fromPin, toPin);
            convertedConnections++;
        } else if (!fromGate->outputs.empty() && !toGate->inputs.empty()) {
            // Tenta conectar ao primeiro pino disponível se o índice não existir
            // Tenta mapeamento inteligente: conecta saída 0 à entrada 0
            int fromPinIdx = std::min(conn.fromPin, (int)fromGate->outputs.size() - 1);
            int toPinIdx = std::min(conn.toPin, (int)toGate->inputs.size() - 1);

            if (fromPinIdx >= 0 && toPinIdx >= 0) {
                Pin* fromPin = &fromGate->outputs[fromPinIdx];
                Pin* toPin = &toGate->inputs[toPinIdx];
                wires.emplace_back(fromPin, toPin);
                convertedConnections++;
            }
        }
    }

    return true;
}

// Importação principal
ImportResult DigitalLogicSimImporter::importFromFile(const std::string& filePath,
                                                      const sf::Font& font) {
    ImportResult result;

    // Lê o arquivo
    std::ifstream f(filePath);
    if (!f) {
        result.errorMessage = "Não foi possível abrir o arquivo: " + filePath;
        return result;
    }

    std::string jsonText((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());
    f.close();

    return importFromJson(jsonText, font);
}

ImportResult DigitalLogicSimImporter::importFromJson(const std::string& jsonText,
                                                      const sf::Font& font) {
    ImportResult result;

    // Parse do JSON
    chip::JsonValue root;
    try {
        root = chip::jsonParse(jsonText);
    } catch (const std::exception& e) {
        result.errorMessage = "Erro ao analisar JSON: " + std::string(e.what());
        return result;
    } catch (...) {
        result.errorMessage = "Erro desconhecido ao analisar JSON.";
        return result;
    }

    // Validação
    std::string validationError;
    if (!validateJson(root, validationError)) {
        result.errorMessage = validationError;
        return result;
    }

    // Parse do projeto DLS
    DLSProject project = parseProject(root);

    // Adaptação por versão
    if (!adaptVersion(project, project.version)) {
        result.errorMessage = "Falha na adaptação da versão " + std::to_string(project.version);
        return result;
    }

    result.projectName = project.name;
    result.totalComponents = static_cast<int>(project.components.size());
    result.totalConnections = static_cast<int>(project.connections.size());

    // Cria os gates
    std::vector<std::string> unsupported;
    if (!createGates(project, result.gates, unsupported, font)) {
        result.errorMessage = "Falha ao criar componentes.";
        return result;
    }

    result.unsupportedComponents = unsupported;
    result.convertedComponents = static_cast<int>(result.gates.size()) -
                                 static_cast<int>(unsupported.size());

    // Cria as conexões
    if (!createWires(project, result.gates, result.wires, result.convertedConnections)) {
        result.errorMessage = "Falha ao recriar conexões.";
        return result;
    }

    result.success = true;
    return result;
}