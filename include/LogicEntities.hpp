#ifndef LOGIC_ENTITIES_HPP
#define LOGIC_ENTITIES_HPP

#include "Gate.hpp"

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
};

class PowerSource : public Gate {
public:
    PowerSource(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::POWER, pos, font) {
        label.setString("POWER: OFF");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
    }
    void toggle() {
        setState(!outputs[0].state);
    }
    void setState(bool on) {
        outputs[0].state = on;
        label.setString(on ? "POWER: ON" : "POWER: OFF");
        label.setFillColor(on ? Theme::SignalOn : Theme::Text);
        updateLabelPos();
    }
    float getExtraData() const override { return outputs[0].state ? 1.0f : 0.0f; }
    void setExtraData(float data) override { setState(data > 0.5f); }
    void updateLogic() override {} // State is manual
};

class ClockGate : public Gate {
public:
    ClockGate(sf::Vector2f pos, const sf::Font& font) : Gate(GateType::CLOCK, pos, font) {
        label.setString("CLOCK");
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 35), this);
        setPosition(pos);
        clock.restart();
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
    float getExtraData() const override { return interval; }
    void setExtraData(float data) override { 
        interval = data; 
        updateLabel();
    }
private:
    sf::Clock clock;
    float interval = 1.0f;
};

#endif
