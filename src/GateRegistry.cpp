#include "GateRegistry.h"
#include "LogicEntities.h"
#include "chip/ChipFactory.h"
#include <algorithm>
#include <cctype>

GateRegistry& GateRegistry::instance() {
    static GateRegistry reg;
    return reg;
}

GateRegistry::GateRegistry() {
    registerType(GateType::AND, "AND", "AND", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ANDGate>(pos, font);
    });
    registerType(GateType::OR, "OR", "OR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ORGate>(pos, font);
    });
    registerType(GateType::NOT, "NOT", "NOT", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<NOTGate>(pos, font);
    });
    registerType(GateType::CLOCK, "CLOCK", "CLOCK", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ClockGate>(pos, font);
    });
    registerType(GateType::POWER, "POWER", "POWER", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
            return std::make_shared<PowerSource>(pos, font);
        });
    registerType(GateType::XOR, "XOR", "XOR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<XORGate>(pos, font);
    });
    registerType(GateType::NAND, "NAND", "NAND", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<NANDGate>(pos, font);
    });
    registerType(GateType::NOR, "NOR", "NOR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<NORGate>(pos, font);
    });
    registerType(GateType::XNOR, "XNOR", "XNOR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<XNORGate>(pos, font);
    });
    registerType(GateType::ROM, "ROM", "ROM", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ROM>(pos, font);
    });
    registerType(GateType::SEG7, "7SEG", "7SEG", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Seg7Display>(pos, font);
    });
    nameToTypeMap_["SEG7"] = GateType::SEG7;
    registerType(GateType::LCD, "LCD", "LCD", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<LCDDisplay>(pos, font);
    });
    registerType(GateType::LED, "LED", "LED", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<LED>(pos, font);
    });
    registerType(GateType::SWITCH, "SWITCH", "SWITCH", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Switch>(pos, font);
    });
    registerType(GateType::BUTTON, "BUTTON", "BUTTON", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Button>(pos, font);
    });
    registerType(GateType::COUNTER, "COUNTER", "COUNTER", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Counter>(pos, font);
    });
    registerType(GateType::REGISTER, "REGISTER", "REG", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Register>(pos, font);
    });
    nameToTypeMap_["REG"] = GateType::REGISTER;
    registerType(GateType::INPUT_PIN, "INPUT_PIN", "IN", [](sf::Vector2f pos, const sf::Font& font, const std::string& pinName) {
        return std::make_shared<InputPin>(pos, font, pinName.empty() ? "IN" : pinName);
    });
    nameToTypeMap_["INPUT"] = GateType::INPUT_PIN;
    nameToTypeMap_["IN"] = GateType::INPUT_PIN;
    registerType(GateType::OUTPUT_PIN, "OUTPUT_PIN", "OUT", [](sf::Vector2f pos, const sf::Font& font, const std::string& pinName) {
        return std::make_shared<OutputPin>(pos, font, pinName.empty() ? "OUT" : pinName);
    });
    nameToTypeMap_["OUTPUT"] = GateType::OUTPUT_PIN;
    nameToTypeMap_["OUT"] = GateType::OUTPUT_PIN;
    registerType(GateType::CHIP, "CHIP", "CHIP", [](sf::Vector2f pos, const sf::Font& font, const std::string& chipId) {
        return chip::createChipInstance(chipId, pos, font);
    });
}

void GateRegistry::registerType(GateType type, const std::string& codeName, const std::string& displayName,
                                std::function<std::shared_ptr<Gate>(sf::Vector2f, const sf::Font&, const std::string&)> factory) {
    GateTypeInfo info;
    info.type = type;
    info.codeName = codeName;
    info.displayName = displayName;
    info.factory = factory;
    typeMap_[type] = info;

    std::string upperCode = codeName;
    std::transform(upperCode.begin(), upperCode.end(), upperCode.begin(), ::toupper);
    nameToTypeMap_[upperCode] = type;
}

std::string GateRegistry::getLabel(GateType type) const {
    auto it = typeMap_.find(type);
    if (it != typeMap_.end()) return it->second.displayName;
    return "GATE";
}

std::string GateRegistry::getCodeName(GateType type) const {
    auto it = typeMap_.find(type);
    if (it != typeMap_.end()) return it->second.codeName;
    return "UNKNOWN";
}

GateType GateRegistry::stringToType(const std::string& name) const {
    std::string upper = name;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    auto it = nameToTypeMap_.find(upper);
    if (it != nameToTypeMap_.end()) return it->second;
    return GateType::AND;
}

std::shared_ptr<Gate> GateRegistry::createGate(GateType type, sf::Vector2f pos, const sf::Font& font, const std::string& extraParam) const {
    auto it = typeMap_.find(type);
    if (it != typeMap_.end() && it->second.factory) {
        return it->second.factory(pos, font, extraParam);
    }
    return nullptr;
}

std::shared_ptr<Gate> GateRegistry::createGate(const std::string& name, sf::Vector2f pos, const sf::Font& font, const std::string& extraParam) const {
    GateType t = stringToType(name);
    return createGate(t, pos, font, extraParam);
}

std::vector<std::string> GateRegistry::getAllCodeNames() const {
    std::vector<std::string> names;
    for (const auto& kv : typeMap_) names.push_back(kv.second.codeName);
    return names;
}
