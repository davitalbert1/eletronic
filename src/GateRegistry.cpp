#include "GateRegistry.h"
#include "LogicEntities.h"
#include "ElectronicEntities.h"
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

    // Componentes Eletrônicos Básicos & Passivos
    registerType(GateType::RESISTOR, "RESISTOR", "RES", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Resistor>(pos, font);
    });
    nameToTypeMap_["R"] = GateType::RESISTOR;
    nameToTypeMap_["RES"] = GateType::RESISTOR;

    registerType(GateType::CAPACITOR, "CAPACITOR", "CAP", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Capacitor>(pos, font);
    });
    nameToTypeMap_["C"] = GateType::CAPACITOR;
    nameToTypeMap_["CAP"] = GateType::CAPACITOR;

    registerType(GateType::DIODE, "DIODE", "DIODE", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Diode>(pos, font);
    });
    nameToTypeMap_["D"] = GateType::DIODE;

    registerType(GateType::TRANSISTOR_NPN, "TRANSISTOR_NPN", "NPN", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<TransistorNPN>(pos, font);
    });
    nameToTypeMap_["NPN"] = GateType::TRANSISTOR_NPN;
    nameToTypeMap_["TRANSISTOR"] = GateType::TRANSISTOR_NPN;

    registerType(GateType::TRANSISTOR_PNP, "TRANSISTOR_PNP", "PNP", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<TransistorPNP>(pos, font);
    });
    nameToTypeMap_["PNP"] = GateType::TRANSISTOR_PNP;

    registerType(GateType::MOSFET_N, "MOSFET_N", "NMOS", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<MosfetN>(pos, font);
    });
    nameToTypeMap_["NMOS"] = GateType::MOSFET_N;
    nameToTypeMap_["MOSFET"] = GateType::MOSFET_N;

    registerType(GateType::MOSFET_P, "MOSFET_P", "PMOS", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<MosfetP>(pos, font);
    });
    nameToTypeMap_["PMOS"] = GateType::MOSFET_P;

    registerType(GateType::POTENTIOMETER, "POTENTIOMETER", "POT", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Potentiometer>(pos, font);
    });
    nameToTypeMap_["POT"] = GateType::POTENTIOMETER;

    registerType(GateType::TACTILE_BUTTON, "TACTILE_BUTTON", "TACT", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<TactileButton>(pos, font);
    });
    nameToTypeMap_["TACT"] = GateType::TACTILE_BUTTON;
    nameToTypeMap_["BOTAO_TATIL"] = GateType::TACTILE_BUTTON;

    registerType(GateType::TOGGLE_SWITCH, "TOGGLE_SWITCH", "CHAVE", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ToggleSwitch>(pos, font);
    });
    nameToTypeMap_["CHAVE"] = GateType::TOGGLE_SWITCH;

    // Sensores
    registerType(GateType::LDR, "LDR", "LDR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<LdrSensor>(pos, font);
    });
    nameToTypeMap_["LUZ"] = GateType::LDR;

    registerType(GateType::THERMISTOR, "THERMISTOR", "NTC", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Thermistor>(pos, font);
    });
    nameToTypeMap_["NTC"] = GateType::THERMISTOR;
    nameToTypeMap_["TEMPERATURA"] = GateType::THERMISTOR;

    registerType(GateType::REED_SWITCH, "REED_SWITCH", "REED", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ReedSwitch>(pos, font);
    });
    nameToTypeMap_["REED"] = GateType::REED_SWITCH;

    registerType(GateType::ULTRASONIC_SENSOR, "ULTRASONIC", "HC-SR04", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<UltrasonicSensor>(pos, font);
    });
    nameToTypeMap_["SR04"] = GateType::ULTRASONIC_SENSOR;
    nameToTypeMap_["HCSR04"] = GateType::ULTRASONIC_SENSOR;
    nameToTypeMap_["ULTRASOM"] = GateType::ULTRASONIC_SENSOR;

    registerType(GateType::DHT22_SENSOR, "DHT22", "DHT22", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Dht22Sensor>(pos, font);
    });
    nameToTypeMap_["DHT"] = GateType::DHT22_SENSOR;
    nameToTypeMap_["DHT11"] = GateType::DHT22_SENSOR;

    registerType(GateType::ACCELEROMETER, "ACCELEROMETER", "MPU6050", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Accelerometer>(pos, font);
    });
    nameToTypeMap_["MPU"] = GateType::ACCELEROMETER;
    nameToTypeMap_["MPU6050"] = GateType::ACCELEROMETER;
    nameToTypeMap_["ACELEROMETRO"] = GateType::ACCELEROMETER;

    registerType(GateType::PIR_SENSOR, "PIR", "PIR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<PirSensor>(pos, font);
    });
    nameToTypeMap_["PRESENCA"] = GateType::PIR_SENSOR;

    registerType(GateType::MQ2_GAS_SENSOR, "MQ2", "MQ-2", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Mq2GasSensor>(pos, font);
    });
    nameToTypeMap_["GAS"] = GateType::MQ2_GAS_SENSOR;
    nameToTypeMap_["FUMACA"] = GateType::MQ2_GAS_SENSOR;

    registerType(GateType::SOUND_SENSOR, "SOUND_SENSOR", "MIC", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<SoundSensor>(pos, font);
    });
    nameToTypeMap_["MIC"] = GateType::SOUND_SENSOR;
    nameToTypeMap_["SOM"] = GateType::SOUND_SENSOR;

    registerType(GateType::IR_SENSOR, "IR_SENSOR", "IR", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<IrSensor>(pos, font);
    });
    nameToTypeMap_["IR"] = GateType::IR_SENSOR;
    nameToTypeMap_["INFRAVERMELHO"] = GateType::IR_SENSOR;

    registerType(GateType::SOIL_MOISTURE_SENSOR, "SOIL_MOISTURE", "SOLO", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<SoilMoistureSensor>(pos, font);
    });
    nameToTypeMap_["SOLO"] = GateType::SOIL_MOISTURE_SENSOR;
    nameToTypeMap_["HIGROMETRO"] = GateType::SOIL_MOISTURE_SENSOR;

    // Atuadores & Saídas
    registerType(GateType::BUZZER, "BUZZER", "BUZZER", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Buzzer>(pos, font);
    });
    nameToTypeMap_["BEEP"] = GateType::BUZZER;

    registerType(GateType::RELAY, "RELAY", "RELE", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<Relay>(pos, font);
    });
    nameToTypeMap_["RELE"] = GateType::RELAY;

    registerType(GateType::SERVO_MOTOR, "SERVO", "SERVO", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<ServoMotor>(pos, font);
    });
    nameToTypeMap_["SERVOMOTOR"] = GateType::SERVO_MOTOR;

    registerType(GateType::DC_MOTOR, "DC_MOTOR", "MOTOR_DC", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<DCMotor>(pos, font);
    });
    nameToTypeMap_["MOTOR"] = GateType::DC_MOTOR;

    registerType(GateType::MOTOR_DRIVER_L298N, "L298N", "DRIVER_L298N", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<MotorDriverL298N>(pos, font);
    });
    nameToTypeMap_["DRIVER"] = GateType::MOTOR_DRIVER_L298N;
    nameToTypeMap_["PONTE_H"] = GateType::MOTOR_DRIVER_L298N;

    registerType(GateType::OLED_DISPLAY, "OLED", "OLED", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<OledDisplay>(pos, font);
    });
    nameToTypeMap_["SSD1306"] = GateType::OLED_DISPLAY;
    nameToTypeMap_["DISPLAY_OLED"] = GateType::OLED_DISPLAY;

    // Comunicação Sem Fio
    registerType(GateType::BLUETOOTH_MODULE, "BLUETOOTH", "HC-05_BT", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<BluetoothModule>(pos, font);
    });
    nameToTypeMap_["BT"] = GateType::BLUETOOTH_MODULE;
    nameToTypeMap_["HC05"] = GateType::BLUETOOTH_MODULE;

    registerType(GateType::WIFI_MODULE, "WIFI", "ESP8266_WIFI", [](sf::Vector2f pos, const sf::Font& font, const std::string&) {
        return std::make_shared<WifiModule>(pos, font);
    });
    nameToTypeMap_["ESP"] = GateType::WIFI_MODULE;
    nameToTypeMap_["ESP8266"] = GateType::WIFI_MODULE;
    nameToTypeMap_["ESP32"] = GateType::WIFI_MODULE;
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
