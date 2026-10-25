#pragma once
#include "Gate.h"
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

// COMPONENTES ELETRÔNICOS BÁSICOS & PASSIVOS
// 1. Resistor (Pass-through digital de sinal / limitador com parâmetro de resistência)
class Resistor : public Gate {
public:
    Resistor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::RESISTOR, pos, font) {
        body.setSize({Config::GateWidth, 40.0f});
        label.setString("R: 1k");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20), this);
        setPosition(pos);
    }
    void updateLogic() override {
        // Sinal passa com atenuação lógica se resistência for finita
        outputs[0].state = inputs[0].state;
    }
    void interact() override {
        // Cicla valores comuns: 220R, 1k, 10k, 100k
        if (resistance_ <= 220.0f) resistance_ = 1000.0f;
        else if (resistance_ <= 1000.0f) resistance_ = 10000.0f;
        else if (resistance_ <= 10000.0f) resistance_ = 100000.0f;
        else resistance_ = 220.0f;
        updateLabel();
    }
    float getExtraData() const override {
        return resistance_;
    }
    void setExtraData(float val) override {
        resistance_ = (val > 0.0f) ? val : 1000.0f;
        updateLabel();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Desenha zigue-zague ou faixas do resistor
        sf::Vertex line[] = {
            sf::Vertex{sf::Vector2f(position.x + 15, position.y + 20), Theme::Text},
            sf::Vertex{sf::Vector2f(position.x + 25, position.y + 12), Theme::Text},
            sf::Vertex{sf::Vector2f(position.x + 35, position.y + 28), Theme::Text},
            sf::Vertex{sf::Vector2f(position.x + 45, position.y + 12), Theme::Text},
            sf::Vertex{sf::Vector2f(position.x + 55, position.y + 28), Theme::Text},
            sf::Vertex{sf::Vector2f(position.x + 65, position.y + 12), Theme::Text},
            sf::Vertex{sf::Vector2f(position.x + 75, position.y + 20), Theme::Text}
        };
        target.draw(line, 7, sf::PrimitiveType::LineStrip);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Resistor>(getPosition(), font);
        g->setExtraData(resistance_);
        return g;
    }
private:
    float resistance_ = 1000.0f; // Ohms
    void updateLabel() {
        if (resistance_ >= 1000.0f) {
            int k = static_cast<int>(resistance_ / 1000.0f);
            label.setString("R: " + std::to_string(k) + "k");
        } else {
            label.setString("R: " + std::to_string(static_cast<int>(resistance_)) + "R");
        }
        updateLabelPos();
    }
};

// 2. Capacitor (Filtro passa-baixas / acumulador de carga)
class Capacitor : public Gate {
public:
    Capacitor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::CAPACITOR, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("CAP: 10uF");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 25), this);
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this);
        setPosition(pos);
    }
    void updateLogic() override {
        // Simulação de carga e descarga do capacitor
        float targetV = inputs[0].state ? 1.0f : 0.0f;
        float dt = 0.05f; // passo de tempo
        float tau = (capacitance_ * 1e-6f) * 1000.0f; // RC aproximado
        if (tau < 0.01f) tau = 0.01f;
        voltage_ += (targetV - voltage_) * (dt / (tau + dt));
        // Limiar digital Schmit-Trigger (0.7V alto, 0.3V baixo)
        if (voltage_ > 0.65f) outputs[0].state = true;
        else if (voltage_ < 0.35f) outputs[0].state = false;
    }
    void interact() override {
        // Cicla capacitâncias: 1uF, 10uF, 100uF
        if (capacitance_ <= 1.0f) capacitance_ = 10.0f;
        else if (capacitance_ <= 10.0f) capacitance_ = 100.0f;
        else capacitance_ = 1.0f;
        label.setString("CAP: " + std::to_string(static_cast<int>(capacitance_)) + "uF");
        updateLabelPos();
    }
    float getExtraData() const override {
        return capacitance_;
    }
    void setExtraData(float val) override {
        capacitance_ = (val > 0.0f) ? val : 10.0f;
        label.setString("CAP: " + std::to_string(static_cast<int>(capacitance_)) + "uF");
        updateLabelPos();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Duas placas paralelas
        sf::RectangleShape plate1(sf::Vector2f(4, 24));
        plate1.setFillColor(Theme::GateOutline);
        plate1.setPosition(sf::Vector2f(position.x + 40, position.y + 13));
        sf::RectangleShape plate2(sf::Vector2f(4, 24));
        plate2.setFillColor(Theme::GateOutline);
        plate2.setPosition(sf::Vector2f(position.x + 48, position.y + 13));
        target.draw(plate1);
        target.draw(plate2);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Capacitor>(getPosition(), font);
        g->setExtraData(capacitance_);
        return g;
    }
private:
    float capacitance_ = 10.0f; // uF
    float voltage_ = 0.0f;
};

// 3. Diodo (Condução unidirecional de sinal / queda de tensão)
class Diode : public Gate {
public:
    Diode(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::DIODE, pos, font) {
        body.setSize({Config::GateWidth, 40.0f});
        label.setString("DIODE");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this); // Ânodo
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20), this); // Cátodo
        setPosition(pos);
    }
    void updateLogic() override {
        // Conduz somente do anodo para o catodo
        outputs[0].state = inputs[0].state;
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Desenha triângulo do diodo e barra de cátodo
        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, sf::Vector2f(position.x + 36, position.y + 10));
        triangle.setPoint(1, sf::Vector2f(position.x + 36, position.y + 30));
        triangle.setPoint(2, sf::Vector2f(position.x + 54, position.y + 20));
        triangle.setFillColor(inputs[0].state ? Theme::SignalOn : Theme::GateOutline);
        target.draw(triangle);
        sf::RectangleShape bar(sf::Vector2f(3, 20));
        bar.setPosition(sf::Vector2f(position.x + 55, position.y + 10));
        bar.setFillColor(Theme::GateOutline);
        target.draw(bar);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<Diode>(getPosition(), font);
    }
};

// 4. Transistor NPN (Bipolar Junction Transistor)
// In[0]: Base, In[1]: Collector -> Out[0]: Emitter (Conduz coletor -> emissor se base estiver HIGH)
class TransistorNPN : public Gate {
public:
    TransistorNPN(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::TRANSISTOR_NPN, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("NPN");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 18), this); // Base (B)
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 42), this); // Collector (C)
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // Emitter (E)
        setPosition(pos);
    }
    void updateLogic() override {
        bool base = inputs[0].state;
        bool collector = inputs[1].state;
        outputs[0].state = base && collector;
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Ícone cartoon simples do transistor
        sf::RectangleShape lineB(sf::Vector2f(16, 3));
        lineB.setPosition(sf::Vector2f(position.x + 20, position.y + 28));
        lineB.setFillColor(Theme::Text);
        target.draw(lineB);
        sf::RectangleShape bar(sf::Vector2f(3, 26));
        bar.setPosition(sf::Vector2f(position.x + 36, position.y + 17));
        bar.setFillColor(inputs[0].state ? Theme::SignalOn : Theme::Text);
        target.draw(bar);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<TransistorNPN>(getPosition(), font);
    }
};

// 5. Transistor PNP (Invertido na base)
// In[0]: Base (ativa em LOW), In[1]: Emitter -> Out[0]: Collector
class TransistorPNP : public Gate {
public:
    TransistorPNP(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::TRANSISTOR_PNP, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("PNP");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 18), this); // Base (B - ativa em LOW)
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 42), this); // Emitter (E)
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // Collector (C)
        setPosition(pos);
    }
    void updateLogic() override {
        bool baseActive = !inputs[0].state;
        bool emitter = inputs[1].state;
        outputs[0].state = baseActive && emitter;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<TransistorPNP>(getPosition(), font);
    }
};

// 6. MOSFET Canal N (In[0]: Gate, In[1]: Drain -> Out[0]: Source)
class MosfetN : public Gate {
public:
    MosfetN(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::MOSFET_N, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("NMOS");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 18), this); // Gate (G)
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 42), this); // Drain (D)
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // Source (S)
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = inputs[0].state && inputs[1].state;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<MosfetN>(getPosition(), font);
    }
};

// 7. MOSFET Canal P (In[0]: Gate ativo baixo, In[1]: Source -> Out[0]: Drain)
class MosfetP : public Gate {
public:
    MosfetP(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::MOSFET_P, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("PMOS");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 18), this); // Gate (G, low conduz)
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 42), this); // Source (S)
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // Drain (D)
        setPosition(pos);
    }
    void updateLogic() override {
        outputs[0].state = (!inputs[0].state) && inputs[1].state;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<MosfetP>(getPosition(), font);
    }
};

// 8. Potenciômetro (Divisor de tensão / ajuste de valor analógico com saída digital parametrizável)
class Potentiometer : public Gate {
public:
    Potentiometer(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::POTENTIOMETER, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("POT: 50%");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 30), this); // Tensão de referência / VCC
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // Wiper
        setPosition(pos);
    }
    void interact() override {
        // Incrementa em passos de 25% (0 -> 25 -> 50 -> 75 -> 100 -> 0)
        wiper_ += 0.25f;
        if (wiper_ > 1.05f) wiper_ = 0.0f;
        updateLabel();
    }
    void updateLogic() override {
        // Se VCC fornecido (ou padrão 1 se desconectado) e wiper > 0.5f => digital HIGH
        bool ref = inputs[0].state;
        outputs[0].state = ref && (wiper_ >= 0.5f);
    }
    float getExtraData() const override {
        return wiper_;
    }
    void setExtraData(float val) override {
        wiper_ = std::clamp(val, 0.0f, 1.0f);
        updateLabel();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Desenha indicador circular do botão de rotação (knob)
        sf::CircleShape knob(12);
        knob.setFillColor(sf::Color(50, 50, 60));
        knob.setOutlineThickness(2);
        knob.setOutlineColor(Theme::GateOutline);
        knob.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 12, position.y + 32));
        target.draw(knob);
        // Ponteiro
        float angle = -135.0f + wiper_ * 270.0f;
        float rad = angle * 3.14159f / 180.0f;
        sf::Vertex pointer[] = {
            sf::Vertex{sf::Vector2f(position.x + Config::GateWidth/2, position.y + 44), Theme::SignalOn},
            sf::Vertex{sf::Vector2f(position.x + Config::GateWidth/2 + std::cos(rad) * 10,
                                    position.y + 44 + std::sin(rad) * 10), Theme::SignalOn}
        };
        target.draw(pointer, 2, sf::PrimitiveType::Lines);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Potentiometer>(getPosition(), font);
        g->setExtraData(wiper_);
        return g;
    }
private:
    float wiper_ = 0.5f; // 0.0 a 1.0
    void updateLabel() {
        int pct = static_cast<int>(wiper_ * 100.0f + 0.5f);
        label.setString("POT: " + std::to_string(pct) + "%");
        updateLabelPos();
    }
};

// 9. Botão Tátil (Push-Button momentâneo com clique)
class TactileButton : public Gate {
public:
    TactileButton(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::TACTILE_BUTTON, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("TACT BTN");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this);
        setPosition(pos);
    }
    void interact() override {
        ticksRemaining_ = 10; // Mantém HIGH por 10 ciclos de simulação
    }
    void updateLogic() override {
        outputs[0].state = (ticksRemaining_ > 0);
        if (ticksRemaining_ > 0) --ticksRemaining_;
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        sf::CircleShape btn(10);
        btn.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 10, position.y + 26));
        btn.setFillColor(ticksRemaining_ > 0 ? Theme::SignalOn : sf::Color(70, 70, 80));
        btn.setOutlineThickness(2);
        btn.setOutlineColor(Theme::GateOutline);
        target.draw(btn);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<TactileButton>(getPosition(), font);
    }
private:
    int ticksRemaining_ = 0;
};

// 10. Chave Gangorra / Alavanca (Toggle Switch)
class ToggleSwitch : public Gate {
public:
    ToggleSwitch(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::TOGGLE_SWITCH, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("CHAVE: OFF");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this);
        setPosition(pos);
    }
    void interact() override {
        active_ = !active_;
        label.setString(active_ ? "CHAVE: ON" : "CHAVE: OFF");
        label.setFillColor(active_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void updateLogic() override {
        outputs[0].state = active_;
    }
    float getExtraData() const override {
        return active_ ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        active_ = (val > 0.5f);
        label.setString(active_ ? "CHAVE: ON" : "CHAVE: OFF");
        label.setFillColor(active_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        sf::RectangleShape lever(sf::Vector2f(20, 10));
        lever.setPosition(sf::Vector2f(position.x + (active_ ? 50 : 25), position.y + 32));
        lever.setFillColor(active_ ? Theme::SignalOn : sf::Color(100, 100, 110));
        target.draw(lever);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<ToggleSwitch>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool active_ = false;
};

// SENSORES ELETRÔNICOS
// 11. Sensor LDR (Light Dependent Resistor / Sensor de Luminosidade)
class LdrSensor : public Gate {
public:
    LdrSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::LDR, pos, font) {
        body.setSize({Config::GateWidth, 55.0f});
        label.setString("LDR: 50%");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 27), this); // Saída digital (comparador / D0)
        setPosition(pos);
    }
    void interact() override {
        // Ajusta luz ambiente: 0% -> 25% -> 50% -> 75% -> 100%
        lightLevel_ += 0.25f;
        if (lightLevel_ > 1.05f) lightLevel_ = 0.0f;
        updateLabel();
    }
    void updateLogic() override {
        // Em alta luz, o LDR conduz (saída ativa em luz >= 50%)
        outputs[0].state = (lightLevel_ >= threshold_);
    }
    float getExtraData() const override {
        return lightLevel_;
    }
    void setExtraData(float val) override {
        lightLevel_ = std::clamp(val, 0.0f, 1.0f);
        updateLabel();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Ícone de sol / onda de luz
        sf::CircleShape sun(8);
        sun.setFillColor(outputs[0].state ? sf::Color(255, 220, 50) : sf::Color(80, 80, 80));
        sun.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 8, position.y + 32));
        target.draw(sun);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<LdrSensor>(getPosition(), font);
        g->setExtraData(lightLevel_);
        return g;
    }
private:
    float lightLevel_ = 0.5f; // 0.0 a 1.0
    float threshold_ = 0.5f;
    void updateLabel() {
        int pct = static_cast<int>(lightLevel_ * 100.0f + 0.5f);
        label.setString("LDR: " + std::to_string(pct) + "%");
        updateLabelPos();
    }
};

// 12. Termistor NTC (Sensor de Temperatura)
class Thermistor : public Gate {
public:
    Thermistor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::THERMISTOR, pos, font) {
        body.setSize({Config::GateWidth, 55.0f});
        label.setString("NTC: 25C");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 27), this); // Alarme térmico / threshold
        setPosition(pos);
    }
    void interact() override {
        // Cicla temperaturas: 0C -> 25C -> 50C -> 80C -> 100C -> 0C
        temperature_ += 25.0f;
        if (temperature_ > 105.0f) temperature_ = 0.0f;
        updateLabel();
    }
    void updateLogic() override {
        // Dispara alarme digital se temp >= 50°C
        outputs[0].state = (temperature_ >= 50.0f);
    }
    float getExtraData() const override {
        return temperature_;
    }
    void setExtraData(float val) override {
        temperature_ = std::clamp(val, -20.0f, 150.0f);
        updateLabel();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Thermistor>(getPosition(), font);
        g->setExtraData(temperature_);
        return g;
    }
private:
    float temperature_ = 25.0f; // Celsius
    void updateLabel() {
        label.setString("NTC: " + std::to_string(static_cast<int>(temperature_)) + "C");
        updateLabelPos();
    }
};

// 13. Reed Switch (Sensor Magnético)
class ReedSwitch : public Gate {
public:
    ReedSwitch(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::REED_SWITCH, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("REED: OFF");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this);
        setPosition(pos);
    }
    void interact() override {
        magnetPresent_ = !magnetPresent_;
        label.setString(magnetPresent_ ? "REED: MAG" : "REED: OFF");
        label.setFillColor(magnetPresent_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void updateLogic() override {
        outputs[0].state = magnetPresent_;
    }
    float getExtraData() const override {
        return magnetPresent_ ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        magnetPresent_ = (val > 0.5f);
        label.setString(magnetPresent_ ? "REED: MAG" : "REED: OFF");
        label.setFillColor(magnetPresent_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<ReedSwitch>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool magnetPresent_ = false;
};

// 14. Sensor Ultrassônico HC-SR04
// In[0]: Trigger -> Out[0]: Echo (Pulsado com largura proporcional à distância), Out[1]: Obstacle detected
class UltrasonicSensor : public Gate {
public:
    UltrasonicSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::ULTRASONIC_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 70.0f});
        label.setString("HC-SR04: 30cm");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 35), this); // TRIG
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this); // ECHO
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 50), this); // DETECT (<50cm)
        setPosition(pos);
    }
    void interact() override {
        // Cicla distâncias: 10cm -> 30cm -> 80cm -> 150cm -> 10cm
        distance_ += 30.0f;
        if (distance_ > 150.0f) distance_ = 10.0f;
        updateLabel();
    }
    void updateLogic() override {
        bool trig = inputs[0].state;
        if (trig) {
            echoTimer_ = static_cast<int>(distance_ / 10.0f) + 1;
        }
        outputs[0].state = (echoTimer_ > 0);
        if (echoTimer_ > 0) --echoTimer_;
        outputs[1].state = (distance_ < 50.0f);
    }
    float getExtraData() const override {
        return distance_;
    }
    void setExtraData(float val) override {
        distance_ = std::clamp(val, 2.0f, 400.0f);
        updateLabel();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Dois transdutores ultrassônicos circulares (olhos do sensor)
        sf::CircleShape transL(8), transR(8);
        transL.setPosition(sf::Vector2f(position.x + 18, position.y + 45));
        transR.setPosition(sf::Vector2f(position.x + 58, position.y + 45));
        transL.setFillColor(sf::Color(180, 180, 200));
        transR.setFillColor(sf::Color(180, 180, 200));
        transL.setOutlineThickness(2);
        transR.setOutlineThickness(2);
        transL.setOutlineColor(Theme::GateOutline);
        transR.setOutlineColor(Theme::GateOutline);
        target.draw(transL);
        target.draw(transR);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<UltrasonicSensor>(getPosition(), font);
        g->setExtraData(distance_);
        return g;
    }
private:
    float distance_ = 30.0f; // cm
    int echoTimer_ = 0;
    void updateLabel() {
        label.setString("SR04: " + std::to_string(static_cast<int>(distance_)) + "cm");
        updateLabelPos();
    }
};

// 15. Sensor de Temperatura & Umidade DHT22
// Out[0]: Dados Digitais / Barramento serial 1-wire, Out[1]: Alta Umidade (>60%)
class Dht22Sensor : public Gate {
public:
    Dht22Sensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::DHT22_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 70.0f});
        label.setString("DHT22: 24C 50%");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this); // DATA
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 50), this); // UMIDADE ALERTA
        setPosition(pos);
    }
    void interact() override {
        humidity_ += 20.0f;
        if (humidity_ > 95.0f) humidity_ = 30.0f;
        temperature_ += 5.0f;
        if (temperature_ > 40.0f) temperature_ = 15.0f;
        updateLabel();
    }
    void updateLogic() override {
        // Envia trem de pulso simulando protocolo digital
        pulseCounter_ = (pulseCounter_ + 1) % 10;
        outputs[0].state = (pulseCounter_ < 5);
        outputs[1].state = (humidity_ >= 60.0f);
    }
    float getExtraData() const override {
        return humidity_;
    }
    void setExtraData(float val) override {
        humidity_ = std::clamp(val, 0.0f, 100.0f);
        updateLabel();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Dht22Sensor>(getPosition(), font);
        g->setExtraData(humidity_);
        return g;
    }
private:
    float temperature_ = 24.0f;
    float humidity_ = 50.0f;
    int pulseCounter_ = 0;
    void updateLabel() {
        label.setString("DHT: " + std::to_string(static_cast<int>(temperature_)) + "C " +
                       std::to_string(static_cast<int>(humidity_)) + "%");
        updateLabelPos();
    }
};

// 16. Acelerômetro MPU6050 (Movimento / Tilt / Eixo X, Y, Z)
// Out[0]: Tilt X (detecção de inclinação X), Out[1]: Tilt Y, Out[2]: Queda Livre / Aceleração
class Accelerometer : public Gate {
public:
    Accelerometer(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::ACCELEROMETER, pos, font) {
        body.setSize({Config::GateWidth, 80.0f});
        label.setString("MPU: TILT");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20), this); // TILT X
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 45), this); // TILT Y
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 70), this); // SHAKE / MOTION
        setPosition(pos);
    }
    void interact() override {
        mode_ = (mode_ + 1) % 4;
        updateLabel();
    }
    void updateLogic() override {
        // mode 0: repouso, mode 1: inclinado X, mode 2: inclinado Y, mode 3: movimento brusco
        outputs[0].state = (mode_ == 1 || mode_ == 3);
        outputs[1].state = (mode_ == 2 || mode_ == 3);
        outputs[2].state = (mode_ == 3);
    }
    float getExtraData() const override {
        return static_cast<float>(mode_);
    }
    void setExtraData(float val) override {
        mode_ = static_cast<int>(val) % 4;
        updateLabel();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Accelerometer>(getPosition(), font);
        g->setExtraData(static_cast<float>(mode_));
        return g;
    }
private:
    int mode_ = 0;
    void updateLabel() {
        const char* titles[] = {"MPU: FLAT", "MPU: TILT-X", "MPU: TILT-Y", "MPU: SHAKE"};
        label.setString(titles[mode_]);
        updateLabelPos();
    }
};

// 17. Sensor de Presença PIR (Passive Infrared)
class PirSensor : public Gate {
public:
    PirSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::PIR_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 55.0f});
        label.setString("PIR: IDLE");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 27), this); // Presença detectada
        setPosition(pos);
    }
    void interact() override {
        detectionHold_ = 15; // Mantém ativo por 15 ciclos
    }
    void updateLogic() override {
        outputs[0].state = (detectionHold_ > 0);
        if (detectionHold_ > 0) {
            --detectionHold_;
            label.setString("PIR: MOV!");
            label.setFillColor(Theme::SignalOn);
        } else {
            label.setString("PIR: IDLE");
            label.setFillColor(Theme::Text);
        }
        updateLabelPos();
    }
    float getExtraData() const override {
        return detectionHold_ > 0 ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        detectionHold_ = (val > 0.5f) ? 15 : 0;
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Cúpula Fresnel
        sf::CircleShape dome(12);
        dome.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 12, position.y + 27));
        dome.setFillColor(detectionHold_ > 0 ? sf::Color(255, 100, 100) : sf::Color(220, 220, 230));
        dome.setOutlineThickness(2);
        dome.setOutlineColor(Theme::GateOutline);
        target.draw(dome);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<PirSensor>(getPosition(), font);
    }
private:
    int detectionHold_ = 0;
};

// 18. Sensor de Gás MQ-2 (Gás inflamável / Fumaça)
class Mq2GasSensor : public Gate {
public:
    Mq2GasSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::MQ2_GAS_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("MQ-2: LIMPO");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // Alarme digital D0
        setPosition(pos);
    }
    void interact() override {
        gasConcentration_ += 250.0f;
        if (gasConcentration_ > 1000.0f) gasConcentration_ = 50.0f;
        updateLabel();
    }
    void updateLogic() override {
        outputs[0].state = (gasConcentration_ >= 400.0f);
    }
    float getExtraData() const override {
        return gasConcentration_;
    }
    void setExtraData(float val) override {
        gasConcentration_ = std::clamp(val, 0.0f, 2000.0f);
        updateLabel();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<Mq2GasSensor>(getPosition(), font);
        g->setExtraData(gasConcentration_);
        return g;
    }
private:
    float gasConcentration_ = 100.0f; // ppm
    void updateLabel() {
        if (gasConcentration_ >= 400.0f) {
            label.setString("MQ2: GAS!");
            label.setFillColor(sf::Color(255, 80, 80));
        } else {
            label.setString("MQ2: " + std::to_string(static_cast<int>(gasConcentration_)) + "ppm");
            label.setFillColor(Theme::Text);
        }
        updateLabelPos();
    }
};

// 19. Sensor de Som / Microfone (Palma / Ruído)
class SoundSensor : public Gate {
public:
    SoundSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::SOUND_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("MIC: SILENT");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this); // D0 Sound Trigger
        setPosition(pos);
    }
    void interact() override {
        pulseRemaining_ = 8;
    }
    void updateLogic() override {
        outputs[0].state = (pulseRemaining_ > 0);
        if (pulseRemaining_ > 0) {
            --pulseRemaining_;
            label.setString("MIC: SOM!");
            label.setFillColor(Theme::SignalOn);
        } else {
            label.setString("MIC: SILENT");
            label.setFillColor(Theme::Text);
        }
        updateLabelPos();
    }
    float getExtraData() const override {
        return pulseRemaining_ > 0 ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        pulseRemaining_ = (val > 0.5f) ? 8 : 0;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<SoundSensor>(getPosition(), font);
    }
private:
    int pulseRemaining_ = 0;
};

// 20. Sensor Infravermelho (Obstáculo / Reflexão IR)
class IrSensor : public Gate {
public:
    IrSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::IR_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("IR: LIVRE");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this); // Ativo em LOW ou HIGH (configurável)
        setPosition(pos);
    }
    void interact() override {
        obstacle_ = !obstacle_;
        label.setString(obstacle_ ? "IR: OBST" : "IR: LIVRE");
        label.setFillColor(obstacle_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void updateLogic() override {
        outputs[0].state = obstacle_;
    }
    float getExtraData() const override {
        return obstacle_ ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        obstacle_ = (val > 0.5f);
        label.setString(obstacle_ ? "IR: OBST" : "IR: LIVRE");
        label.setFillColor(obstacle_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<IrSensor>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool obstacle_ = false;
};

// 21. Sensor de Umidade do Solo (Higrômetro)
class SoilMoistureSensor : public Gate {
public:
    SoilMoistureSensor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::SOIL_MOISTURE_SENSOR, pos, font) {
        body.setSize({Config::GateWidth, 55.0f});
        label.setString("SOLO: 40%");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 27), this); // Saída digital (Seco = HIGH)
        setPosition(pos);
    }
    void interact() override {
        moisture_ += 25.0f;
        if (moisture_ > 105.0f) moisture_ = 10.0f;
        updateLabel();
    }
    void updateLogic() override {
        // Dispara se solo estiver seco (< 30%)
        outputs[0].state = (moisture_ < 30.0f);
    }
    float getExtraData() const override {
        return moisture_;
    }
    void setExtraData(float val) override {
        moisture_ = std::clamp(val, 0.0f, 100.0f);
        updateLabel();
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<SoilMoistureSensor>(getPosition(), font);
        g->setExtraData(moisture_);
        return g;
    }
private:
    float moisture_ = 40.0f; // %
    void updateLabel() {
        label.setString("SOLO: " + std::to_string(static_cast<int>(moisture_)) + "%");
        updateLabelPos();
    }
};

// ATUADORES & SAÍDAS
// 22. Buzzer (Sinalizador sonoro piezoelétrico)
class Buzzer : public Gate {
public:
    Buzzer(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::BUZZER, pos, font) {
        body.setSize({Config::GateWidth, 50.0f});
        label.setString("BUZZER");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 25), this);
        setPosition(pos);
    }
    void updateLogic() override {
        sounding_ = inputs[0].state;
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Desenha autofalante / ondas sonoras
        sf::CircleShape disk(14);
        disk.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 14, position.y + 24));
        disk.setFillColor(sounding_ ? Theme::SignalOn : sf::Color(60, 60, 70));
        disk.setOutlineThickness(2);
        disk.setOutlineColor(Theme::GateOutline);
        target.draw(disk);
        if (sounding_) {
            // Ondas sonoras animadas ou arcos
            sf::CircleShape wave(20);
            wave.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 20, position.y + 18));
            wave.setFillColor(sf::Color::Transparent);
            wave.setOutlineThickness(2);
            wave.setOutlineColor(Theme::SignalOn);
            target.draw(wave);
        }
    }
    float getExtraData() const override {
        return sounding_ ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        sounding_ = (val > 0.5f);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<Buzzer>(getPosition(), font);
    }
private:
    bool sounding_ = false;
};

// 23. Relé (Relay eletromecânico)
// In[0]: Bobina (COIL) -> Out[0]: NA (Normalmente Aberto), Out[1]: NF (Normalmente Fechado)
class Relay : public Gate {
public:
    Relay(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::RELAY, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("RELE");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 30), this); // Coil
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 18), this); // NO (Normalmente Aberto)
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 42), this); // NC (Normalmente Fechado)
        setPosition(pos);
    }
    void updateLogic() override {
        bool coil = inputs[0].state;
        outputs[0].state = coil;
        outputs[1].state = !coil;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<Relay>(getPosition(), font);
    }
};

// 24. Servomotor (Controle angular via PWM ou entrada binária)
// In[0]: Pulso / Ângulo bit 0, In[1]: Ângulo bit 1 -> Out[0]: Posição alcançada
class ServoMotor : public Gate {
public:
    ServoMotor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::SERVO_MOTOR, pos, font) {
        body.setSize({Config::GateWidth, 65.0f});
        label.setString("SERVO: 90");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 22), this); // Posição A
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 44), this); // Posição B
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 32), this); // Feedback
        setPosition(pos);
    }
    void updateLogic() override {
        // 00 -> 0°, 01 -> 45°, 10 -> 90°, 11 -> 180°
        int bits = (inputs[0].state ? 1 : 0) | (inputs[1].state ? 2 : 0);
        switch (bits) {
            case 0:
                targetAngle_ = 0.0f;
                break;
            case 1:
                targetAngle_ = 45.0f;
                break;
            case 2:
                targetAngle_ = 90.0f;
                break;
            case 3:
                targetAngle_ = 180.0f;
                break;
        }
        currentAngle_ += (targetAngle_ - currentAngle_) * 0.2f;
        outputs[0].state = std::abs(currentAngle_ - targetAngle_) < 2.0f;
        updateLabel();
    }
    void interact() override {
        targetAngle_ += 45.0f;
        if (targetAngle_ > 185.0f) targetAngle_ = 0.0f;
        updateLabel();
    }
    float getExtraData() const override {
        return currentAngle_;
    }
    void setExtraData(float val) override {
        currentAngle_ = std::clamp(val, 0.0f, 180.0f);
        targetAngle_ = currentAngle_;
        updateLabel();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Braço do servomotor girando
        float rad = (currentAngle_ - 90.0f) * 3.14159f / 180.0f;
        sf::Vertex arm[] = {
            sf::Vertex{sf::Vector2f(position.x + Config::GateWidth/2, position.y + 40), Theme::SignalOn},
            sf::Vertex{sf::Vector2f(position.x + Config::GateWidth/2 + std::cos(rad) * 16,
                                    position.y + 40 + std::sin(rad) * 16), Theme::Highlight}
        };
        target.draw(arm, 2, sf::PrimitiveType::Lines);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<ServoMotor>(getPosition(), font);
        g->setExtraData(currentAngle_);
        return g;
    }
private:
    float currentAngle_ = 90.0f;
    float targetAngle_ = 90.0f;
    void updateLabel() {
        label.setString("SERVO: " + std::to_string(static_cast<int>(currentAngle_ + 0.5f)) + "\xC2\xB0");
        updateLabelPos();
    }
};

// 25. Motor DC (In[0]: Liga/PWM, In[1]: Direção -> Out[0]: Rotação ativa)
class DCMotor : public Gate {
public:
    DCMotor(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::DC_MOTOR, pos, font) {
        body.setSize({Config::GateWidth, 60.0f});
        label.setString("MOTOR DC");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20), this); // ENABLE
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 42), this); // DIR
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 30), this); // RUNNING
        setPosition(pos);
    }
    void updateLogic() override {
        running_ = inputs[0].state;
        direction_ = inputs[1].state;
        outputs[0].state = running_;
        if (running_) {
            rotationAngle_ += direction_ ? 15.0f : -15.0f;
            if (rotationAngle_ > 360.0f) rotationAngle_ -= 360.0f;
            if (rotationAngle_ < 0.0f) rotationAngle_ += 360.0f;
        }
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Hélice / Rotor
        sf::CircleShape rotor(12);
        rotor.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 12, position.y + 32));
        rotor.setFillColor(running_ ? sf::Color(80, 150, 220) : sf::Color(60, 60, 70));
        rotor.setOutlineThickness(2);
        rotor.setOutlineColor(Theme::GateOutline);
        target.draw(rotor);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<DCMotor>(getPosition(), font);
    }
private:
    bool running_ = false;
    bool direction_ = true;
    float rotationAngle_ = 0.0f;
};

// 26. Driver de Motor L298N (Ponte H Dupla)
// In[0]: ENA, In[1]: IN1, In[2]: IN2 -> Out[0]: OUT1 (Motor A+), Out[1]: OUT2 (Motor A-)
class MotorDriverL298N : public Gate {
public:
    MotorDriverL298N(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::MOTOR_DRIVER_L298N, pos, font) {
        body.setSize({Config::GateWidth, 80.0f});
        label.setString("L298N");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 18), this); // ENA (PWM)
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 40), this); // IN1
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 62), this); // IN2
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this); // OUT1
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 55), this); // OUT2
        setPosition(pos);
    }
    void updateLogic() override {
        bool ena = inputs[0].state;
        bool in1 = inputs[1].state;
        bool in2 = inputs[2].state;
        outputs[0].state = ena && in1 && !in2;
        outputs[1].state = ena && !in1 && in2;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        return std::make_shared<MotorDriverL298N>(getPosition(), font);
    }
};

// 27. Display OLED / Matriz Gráfica (Entradas 4 bits de dados para visualização de padrões)
class OledDisplay : public Gate {
public:
    OledDisplay(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::OLED_DISPLAY, pos, font) {
        body.setSize({Config::GateWidth, 100.0f});
        label.setString("OLED SSD1306");
        for (int i = 0; i < 4; ++i) {
            inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 18 + i * 20), this);
        }
        setPosition(pos);
    }
    void updateLogic() override {
        int val = 0;
        for (int i = 0; i < 4; ++i) if (inputs[i].state) val |= (1 << i);
        pattern_ = val;
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Tela OLED retangular preta com pixels azuis/amarelos
        sf::RectangleShape screen(sf::Vector2f(Config::GateWidth - 24, 45));
        screen.setPosition(sf::Vector2f(position.x + 12, position.y + 45));
        screen.setFillColor(sf::Color(5, 5, 15));
        screen.setOutlineThickness(2);
        screen.setOutlineColor(sf::Color(80, 80, 100));
        target.draw(screen);

        // Grade 4x4 de pixels desenhada conforme o valor de entrada
        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < 4; ++col) {
                int bit = (row * 4 + col) % 4;
                if ((pattern_ >> bit) & 1) {
                    sf::RectangleShape px(sf::Vector2f(10, 8));
                    px.setPosition(sf::Vector2f(position.x + 20 + col * 14, position.y + 52 + row * 14));
                    px.setFillColor(sf::Color(80, 220, 255));
                    target.draw(px);
                }
            }
        }
    }
    float getExtraData() const override {
        return static_cast<float>(pattern_);
    }
    void setExtraData(float val) override {
        pattern_ = static_cast<int>(val) & 0xF;
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<OledDisplay>(getPosition(), font);
        g->setExtraData(static_cast<float>(pattern_));
        return g;
    }
private:
    int pattern_ = 0;
};

// MÓDULOS DE COMUNICAÇÃO SEM FIO
// 28. Módulo Bluetooth HC-05 / HC-06
// In[0]: TX_IN (Sinal enviado para o módulo), Out[0]: RX_OUT (Sinal recebido), Out[1]: STATE (Conectado)
class BluetoothModule : public Gate {
public:
    BluetoothModule(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::BLUETOOTH_MODULE, pos, font) {
        body.setSize({Config::GateWidth, 75.0f});
        label.setString("HC-05 BT: OFF");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 25), this); // TX de entrada
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 25), this); // RX de saída
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 50), this); // STATE (Pareado)
        setPosition(pos);
    }
    void interact() override {
        connected_ = !connected_;
        label.setString(connected_ ? "HC-05: PAIRED" : "HC-05: OFF");
        label.setFillColor(connected_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void updateLogic() override {
        outputs[1].state = connected_;
        // Quando pareado, os dados enviados são ecoados na saída sem fio
        outputs[0].state = connected_ && inputs[0].state;
    }
    float getExtraData() const override {
        return connected_ ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        connected_ = (val > 0.5f);
        label.setString(connected_ ? "HC-05: PAIRED" : "HC-05: OFF");
        label.setFillColor(connected_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Antena serpentina PCB no topo
        sf::RectangleShape antenna(sf::Vector2f(16, 6));
        antenna.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 8, position.y + 48));
        antenna.setFillColor(sf::Color(60, 120, 240));
        target.draw(antenna);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<BluetoothModule>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool connected_ = false;
};

// 29. Módulo WiFi ESP8266 / ESP32
// In[0]: DATA_IN, In[1]: RESET -> Out[0]: DATA_OUT, Out[1]: WIFI_CONNECTED
class WifiModule : public Gate {
public:
    WifiModule(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::WIFI_MODULE, pos, font) {
        body.setSize({Config::GateWidth, 75.0f});
        label.setString("ESP8266: OK");
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 22), this); // TXD
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 50), this); // RST
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 22), this); // RXD
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 50), this); // LINK
        setPosition(pos);
    }
    void interact() override {
        connected_ = !connected_;
        label.setString(connected_ ? "ESP: ONLINE" : "ESP: OFFLINE");
        label.setFillColor(connected_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void updateLogic() override {
        if (inputs[1].state) connected_ = false; // Reset ativo
        outputs[1].state = connected_;
        outputs[0].state = connected_ && inputs[0].state;
    }
    float getExtraData() const override {
        return connected_ ? 1.0f : 0.0f;
    }
    void setExtraData(float val) override {
        connected_ = (val > 0.5f);
        label.setString(connected_ ? "ESP: ONLINE" : "ESP: OFFLINE");
        label.setFillColor(connected_ ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    void draw(sf::RenderTarget& target) override {
        drawBase(target);
        // Ícone de ondas WiFi
        sf::CircleShape w1(10);
        w1.setPosition(sf::Vector2f(position.x + Config::GateWidth/2 - 10, position.y + 46));
        w1.setFillColor(sf::Color::Transparent);
        w1.setOutlineThickness(2);
        w1.setOutlineColor(connected_ ? Theme::SignalOn : sf::Color(100, 100, 100));
        target.draw(w1);
    }
    std::shared_ptr<Gate> clone(const sf::Font& font) const override {
        auto g = std::make_shared<WifiModule>(getPosition(), font);
        g->setExtraData(getExtraData());
        return g;
    }
private:
    bool connected_ = true;
};
