#ifndef LOGIC_ENTITIES_HPP
#define LOGIC_ENTITIES_HPP
#include "Gate.h"
#include <array>
#include <cstdint>

// Forward declarations para os tipos de chip (definidos em include/chip/).
namespace chip {
    class ChipDefinition;
    class Circuit;
}

class ANDGate : public Gate {
public:
    ANDGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::AND, pos, font) {
        label.setString("AND");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = inputs[0].state && inputs[1].state;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<ANDGate>(getPosition(), font);
    }
};

class ORGate : public Gate {
public:
    ORGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::OR, pos, font) {
        label.setString("OR");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = inputs[0].state || inputs[1].state;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<ORGate>(getPosition(), font);
    }
};

class NOTGate : public Gate {
public:
    NOTGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::NOT, pos, font) {
        label.setString("NOT");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 35), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = !inputs[0].state;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<NOTGate>(getPosition(), font);
    }
};

class PowerSource : public Gate {
public:
    PowerSource(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::POWER, pos, font) {
        label.setString("POWER: OFF");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void interact() override {
        setState(!outputs[0].state);
    }
    void setState(bool on) {
        outputs[0].state = on;
        label.setString(on ? "POWER: ON" : "POWER: OFF");
        label.setFillColor(on ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    float getExtraData() const override {
        return outputs[0].state ? 1.0f : 0.0f;
    }
    void setExtraData(float data) override {
        setState(data > 0.5f);
    }
    void updateLogic() override {} // State is manual
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<PowerSource>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
};

class ClockGate : public Gate {
public:
    ClockGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::CLOCK, pos, font) {
        label.setString("CLOCK");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
        clock.restart();
    }
    void interact() override {
        toggleInterval();
    }
    void updateLogic() override {
        if (clock.getElapsedTime().asSeconds() >= interval) {
            outputs[0].state = !outputs[0].state;
            clock.restart();
        }
    }
    void toggleInterval() {
        if (interval >= 2.0f) interval = 0.1f;
        else if (interval >= 1.0f) interval = 2.0f;
        else if (interval >= 0.5f) interval = 1.0f;
        else interval = 0.5f;
        updateLabel();
    }
    void updateLabel() {
        label.setString("CLOCK: " + std::to_string(interval).substr(0, 3) + "s");
        updateLabelPos();
    }
    float getExtraData() const override {
        return interval;
    }
    void setExtraData(float data) override {
        interval = data;
        updateLabel();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<ClockGate>(getPosition(), font);
        g->setExtraData(interval);
        return g;
    }
private:
    sf::Clock clock;
    float interval = 1.0f;
};

class XORGate : public Gate {
public:
    XORGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::XOR, pos, font) {
        label.setString("XOR");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = inputs[0].state != inputs[1].state;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<XORGate>(getPosition(), font);
    }
};

class NANDGate : public Gate {
public:
    NANDGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::NAND, pos, font) {
        label.setString("NAND");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = !(inputs[0].state && inputs[1].state);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<NANDGate>(getPosition(), font);
    }
};

class NORGate : public Gate {
public:
    NORGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::NOR, pos, font) {
        label.setString("NOR");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = !(inputs[0].state || inputs[1].state);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<NORGate>(getPosition(), font);
    }
};

class XNORGate : public Gate {
public:
    XNORGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::XNOR, pos, font) {
        label.setString("XNOR");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = !(inputs[0].state != inputs[1].state);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<XNORGate>(getPosition(), font);
    }
};

// Memória ROM: 4 entradas de endereço (A0-A3) e 4 saídas de dados (D0-D3)
class ROM : public Gate {
public:
    ROM(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::ROM, pos, font) {
        body.setSize({Config::GateWidth, 160.0f});
        label.setString("ROM");
        for (int i = 0; i < 4; ++i)
            inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20 + i * 30), this);
        for (int i = 0; i < 4; ++i)
            outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20 + i * 30), this);
        setPosition(pos);
        loadPreset(0);
    }
    void interact() override {
        presetIndex = (presetIndex + 1) % (int)ROM_PRESETS.size();
        loadPreset(presetIndex);
    }
    void updateLogic() override {
        int addr = 0;
        for (int i = 0; i < 4; ++i) if (inputs[i].state) addr |= (1 << i);
        uint8_t data = contents[addr & 0xF];
        for (int i = 0; i < 4; ++i) outputs[i].state = (data >> i) & 1;
    }
    float getExtraData() const override { return (float)presetIndex; }
    void setExtraData(float data) override { loadPreset((int)data); }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<ROM>(getPosition(), font);
        g->setExtraData((float)presetIndex);
        return g;
    }
private:
    int presetIndex = 0;
    std::array<uint8_t, 16> contents{};
    void loadPreset(int idx) {
        presetIndex = idx;
        if (idx >= 0 && idx < (int)ROM_PRESETS.size()) contents = ROM_PRESETS[idx];
    }
    static constexpr std::array<std::array<uint8_t, 16>, 4> ROM_PRESETS = {{
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
        {1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0},
        {15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    }};
};

// Display de 7 segmentos: 4 entradas (D0-D3) decodificam dígito 0-15
class Seg7Display : public Gate {
public:
    Seg7Display(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::SEG7, pos, font) {
        body.setSize({Config::GateWidth, 160.0f});
        label.setString("7-SEG");
        for (int i = 0; i < 4; ++i)
            inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20 + i * 30), this);
        setPosition(pos);
    }
    void updateLogic() override {
        int v = 0;
        for (int i = 0; i < 4; ++i) if (inputs[i].state) v |= (1 << i);
        seg = SEG_MAP[v & 0xF];
    }
    void draw(sf::RenderWindow& window) override {
        drawBase(window);
        const float x = position.x + 25, y = position.y + 70, w = 50, h = 12, g = 4;
        auto segRect = [&](float sx, float sy, float sw, float sh) {
            sf::RectangleShape r(sf::Vector2f(sw, sh));
            r.setPosition({sx, sy});
            r.setFillColor(Theme::SignalOn);
            return r;
        };

        if (seg & 1) window.draw(segRect(x + w/2 - h/2, y, w, h)); // a (top)
        if (seg & 2) window.draw(segRect(x + w - h, y + h/2 + g/2, h, h)); // b (top-right)
        if (seg & 4) window.draw(segRect(x + w - h, y + h + g + h/2 + g/2, h, h)); // c (bottom-right)
        if (seg & 8) window.draw(segRect(x + w/2 - h/2, y + 2*h + 2*g, w, h)); // d (bottom)
        if (seg & 16) window.draw(segRect(x, y + h + g + h/2 + g/2, h, h)); // e (bottom-left)
        if (seg & 32) window.draw(segRect(x, y + h/2 + g/2, h, h)); // f (top-left)
        if (seg & 64) window.draw(segRect(x + w/2 - h/2, y + h + g, w, h)); // g (middle)
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<Seg7Display>(getPosition(), font);
    }
private:
    int seg = 0;
    static constexpr std::array<int, 16> SEG_MAP = {
        0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70,
        0x7F, 0x7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47
    };
};

// Display LCD: 8 entradas de dados (D0-D7) exibem caractere ASCII
class LCDDisplay : public Gate {
public:
    LCDDisplay(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::LCD, pos, font), charText_(font) {
        body.setSize({Config::GateWidth, 160.0f});
        label.setString("LCD");
        for (int i = 0; i < 8; ++i)
            inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 12 + i * 17), this);
        setPosition(pos);
        charText_.setCharacterSize(40);
        charText_.setFillColor(Theme::SignalOn);
    }
    void updateLogic() override {
        int v = 0;
        for (int i = 0; i < 8; ++i) if (inputs[i].state) v |= (1 << i);
        value = v;
    }
    void draw(sf::RenderWindow& window) override {
        drawBase(window);
        sf::RectangleShape screen(sf::Vector2f(Config::GateWidth - 20, 50));
        screen.setPosition({position.x + 10, position.y + 90});
        screen.setFillColor(sf::Color(10, 20, 10));
        screen.setOutlineThickness(2);
        screen.setOutlineColor(Theme::GateOutline);
        window.draw(screen);
        char c = (value >= 32 && value < 127) ? (char)value : '?';
        charText_.setString(std::string(1, c));
        charText_.setPosition({position.x + Config::GateWidth/2 - 12, position.y + 92});
        window.draw(charText_);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<LCDDisplay>(getPosition(), font);
    }
private:
    int value = 0;
    sf::Text charText_;
};

// LED indicador: 1 entrada
class LED : public Gate {
public:
    LED(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::LED, pos, font) {
        label.setString("LED");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        lit = inputs[0].state;
    }
    void draw(sf::RenderWindow& window) override {
        drawBase(window);
        sf::CircleShape bulb(18);
        bulb.setPosition({position.x + Config::GateWidth/2 - 18, position.y + 45});
        bulb.setFillColor(lit ? Theme::SignalOn : sf::Color(60, 60, 70));
        bulb.setOutlineThickness(3);
        bulb.setOutlineColor(Theme::GateOutline);
        window.draw(bulb);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<LED>(getPosition(), font);
    }
private:
    bool lit = false;
};

// Chave (switch): 1 saída, clique alterna
class Switch : public Gate {
public:
    Switch(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::SWITCH, pos, font) {
        label.setString("SW: OFF");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void interact() override {
        on = !on;
        label.setString(on ? "SW: ON" : "SW: OFF");
        label.setFillColor(on ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void updateLogic() override { outputs[0].state = on; }
    float getExtraData() const override { return on ? 1.0f : 0.0f; }
    void setExtraData(float data) override {
        on = data > 0.5f;
        label.setString(on ? "SW: ON" : "SW: OFF");
        label.setFillColor(on ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Switch>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool on = false;
};

// Botão (pulsar): 1 saída, clique gera pulso
class Button : public Gate {
public:
    Button(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::BUTTON, pos, font) {
        label.setString("BTN");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void interact() override { pulse = 12; }
    void updateLogic() override {
        outputs[0].state = pulse > 0;
        if (pulse > 0) --pulse;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<Button>(getPosition(), font);
    }
private:
    int pulse = 0;
};

// Contador: clk (0), rst (1) -> 4 saídas (Q0-Q3)
class Counter : public Gate {
public:
    Counter(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::COUNTER, pos, font) {
        body.setSize({Config::GateWidth, 160.0f});
        label.setString("COUNTER");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);   // clk
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this);   // rst
        for (int i = 0; i < 4; ++i)
            outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20 + i * 30), this);
        setPosition(pos);
    }
    void updateLogic() override {
        bool clk = inputs[0].state;
        if (inputs[1].state) count = 0;
        else if (clk && !prevClk) count = (count + 1) & 0xF;
        prevClk = clk;
        for (int i = 0; i < 4; ++i) outputs[i].state = (count >> i) & 1;
    }
    float getExtraData() const override {
        return (float)count;
    }
    void setExtraData(float data) override {
        count = (int)data & 0xF;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Counter>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    int count = 0;
    bool prevClk = false;
};

// Registrador: clk (0) + 4 dados (D0-D3) -> 4 saídas (Q0-Q3)
class Register : public Gate {
public:
    Register(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::REGISTER, pos, font) {
        body.setSize({Config::GateWidth, 160.0f});
        label.setString("REG");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);   // clk
        for (int i = 0; i < 4; ++i)
            inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50 + i * 24), this); // D0-D3
        for (int i = 0; i < 4; ++i)
            outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20 + i * 30), this);
        setPosition(pos);
    }
    void updateLogic() override {
        bool clk = inputs[0].state;
        if (clk && !prevClk) {
            for (int i = 0; i < 4; ++i) stored[i] = inputs[1 + i].state;
        }
        prevClk = clk;
        for (int i = 0; i < 4; ++i) outputs[i].state = stored[i];
    }
    float getExtraData() const override {
        int v = 0;
        for (int i = 0; i < 4; ++i) if (stored[i]) v |= (1 << i);
        return (float)v;
    }
    void setExtraData(float data) override {
        int v = (int)data & 0xF;
        for (int i = 0; i < 4; ++i) stored[i] = (v >> i) & 1;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Register>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool stored[4] = {false, false, false, false};
    bool prevClk = false;
};

// Pino de ENTRADA: marca um pino externo do chip.
class InputPin : public Gate {
public:
    InputPin(sf::Vector2f pos, const sf::Font& font, const std::string& name = "IN")
        : Gate(GateType::INPUT_PIN, pos, font), pinName_(name) {
        label.setString(name);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        // O sinal já foi escrito em outputs[0].state pelo CustomChip antes
        // da simulação interna; aqui apenas propagamos (sem alteração).
    }
    std::string getPinName() const override { return pinName_; }
    void setPinName(const std::string& n) override {
        pinName_ = n;
        label.setString(n);
        updateLabelPos();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<InputPin>(getPosition(), font, pinName_);
    }
private:
    std::string pinName_;
};

// Pino de SAÍDA: marca um pino externo do chip.
class OutputPin : public Gate {
public:
    OutputPin(sf::Vector2f pos, const sf::Font& font, const std::string& name = "OUT")
        : Gate(GateType::OUTPUT_PIN, pos, font), pinName_(name) {
        label.setString(name);
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 35), this);
        setPosition(pos);
    }
    void updateLogic() override {
        // O valor de entrada é lido pelo CustomChip após a simulação interna.
    }
    std::string getPinName() const override { return pinName_; }
    void setPinName(const std::string& n) override {
        pinName_ = n;
        label.setString(n);
        updateLabelPos();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<OutputPin>(getPosition(), font, pinName_);
    }
private:
    std::string pinName_;
};

class CustomChip : public Gate {
public:
    // Constrói a instância a partir de uma definição.
    CustomChip(sf::Vector2f pos, const sf::Font& font, const std::shared_ptr<chip::ChipDefinition>& def);

    void updateLogic() override; // alimenta internos, simula, lê saídas
    void interact() override; // clica propaga para o circuito interno
    std::shared_ptr<Gate> clone(const sf::Font& font) const override;

    std::string getChipId() const override {
        return chipId_;
    }

    // Acesso à definição (para edição/atualização).
    std::shared_ptr<chip::ChipDefinition> getDefinition() const {
        return definition_;
    }

    // Recalcula entradas/saídas externas a partir da definição (usado após
    // atualizar o chip). Recria o circuito interno.
private:
    std::shared_ptr<chip::ChipDefinition> definition_;
    std::shared_ptr<chip::Circuit> internal_; // cópia independente do circuito
    std::string chipId_;

    // Mapeia índice de pino externo -> InputPin/OutputPin interno.
    std::vector<InputPin*> inputPins_; // [i] = pino interno da entrada i
    std::vector<OutputPin*> outputPins_; // [i] = pino interno da saída i
};

#endif
