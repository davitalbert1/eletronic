#include "LogicEntities.h"
#include "chip/ChipDefinition.h"
#include "chip/ChipLibrary.h"

CustomChip::CustomChip(sf::Vector2f pos, const sf::Font& font, const std::shared_ptr<chip::ChipDefinition>& def)
    : Gate(GateType::CHIP, pos, font), definition_(def), chipId_(def ? def->id : "") {
    if (def) {
        label.setString(def->name);
        // Clona o circuito para esta instância ser independente.
        internal_ = def->circuit->clone(font);
        rebuild(font);
    }
    setPosition(pos);
}

void CustomChip::rebuild(const sf::Font& font) {
    inputs.clear();
    outputs.clear();
    inputPins_.clear();
    outputPins_.clear();
    if (!definition_ || !internal_) return;

    // Entradas externas (na ordem de definition_->inputs).
    const float dy = 35.0f;
    for (size_t i = 0; i < definition_->inputs.size(); ++i) {
        inputs.emplace_back(PinType::INPUT, sf::Vector2f(0, 20 + i * dy), this);
    }
    for (size_t i = 0; i < definition_->outputs.size(); ++i) {
        outputs.emplace_back(PinType::OUTPUT, sf::Vector2f(Config::GateWidth, 20 + i * dy), this);
    }

    // Mapeia pinos externos -> InputPin/OutputPin internos
    auto inPins = internal_->findGatesOfType(GateType::INPUT_PIN);
    auto outPins = internal_->findGatesOfType(GateType::OUTPUT_PIN);
    for (auto* g : inPins) inputPins_.push_back(static_cast<InputPin*>(g));
    for (auto* g : outPins) outputPins_.push_back(static_cast<OutputPin*>(g));

    // Garante que o corpo tenha tamanho suficiente para todos os pinos.
    float h = 20.0f + std::max(inputPins_.size(), outputPins_.size()) * dy + 15.0f;
    body.setSize({Config::GateWidth, std::max(h, Config::GateHeight)});
    updateLabelPos();
}

void CustomChip::updateLogic() {
    if (!definition_ || !internal_) return;

    // 1) Alimenta os pinos de entrada internos com os sinais externos.
    for (size_t i = 0; i < inputPins_.size() && i < inputs.size(); ++i) {
        inputPins_[i]->outputs[0].state = inputs[i].state;
    }

    // 2) Simula o circuito interno (reset + fios + updateLogic de cada gate,
    // incluindo chips aninhados de forma recursiva).
    internal_->simulate();

    // 3) Lê as saídas internas e atualiza as saídas externas do chip.
    for (size_t i = 0; i < outputPins_.size() && i < outputs.size(); ++i) {
        outputs[i].state = outputPins_[i]->inputs[0].state;
    }
}

void CustomChip::interact() {
    if (internal_) {
        for (auto& g : internal_->gates) g->interact();
    }
}

std::shared_ptr<Gate> CustomChip::clone(const sf::Font& font) const {
    auto c = std::make_shared<CustomChip>(getPosition(), font, definition_);
    c->setRotation(getRotation());
    return c;
}