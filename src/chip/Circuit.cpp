#include "chip/Circuit.h"

namespace chip {
    std::shared_ptr<Circuit> Circuit::clone(const sf::Font& font) const {
        auto copy = std::make_shared<Circuit>();

        // 1) Clonar gates preservando a ordem (o índice é a "identidade" do fio).
        copy->gates.reserve(gates.size());
        for (auto& g : gates) {
            copy->gates.push_back(g->clone(font));
        }

        // 2) Reconectar fios usando (índice do gate, índice do pino).
        copy->wires.reserve(wires.size());
        for (auto& w : wires) {
            int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
            for (size_t i = 0; i < gates.size(); ++i) {
                if (gates[i].get() == w.fromPin->parent) gFrom = (int)i;
                if (gates[i].get() == w.toPin->parent)   gTo   = (int)i;
            }
            for (size_t i = 0; i < w.fromPin->parent->outputs.size(); ++i)
                if (&w.fromPin->parent->outputs[i] == w.fromPin) pFrom = (int)i;
            for (size_t i = 0; i < w.toPin->parent->inputs.size(); ++i)
                if (&w.toPin->parent->inputs[i] == w.toPin) pTo = (int)i;

            if (gFrom >= 0 && gTo >= 0 && pFrom >= 0 && pTo >= 0) {
                Pin* from = &copy->gates[gFrom]->outputs[pFrom];
                Pin* to   = &copy->gates[gTo]->inputs[pTo];
                copy->wires.emplace_back(from, to);
            }
        }
        return copy;
    }

    void Circuit::simulate() {
        // Reset de todas as entradas (evita sinais "fantasma" de fios removidos).
        for (auto& g : gates) {
            for (auto& pin : g->inputs) pin.state = false;
        }

        // Propagar sinais pelos fios (output -> input).
        for (auto& w : wires) w.update();

        // Avaliar a lógica de cada gate.
        for (auto& g : gates) g->updateLogic();
    }

    void Circuit::draw(sf::RenderWindow& window) {
        for (auto& w : wires) w.draw(window);
        for (auto& g : gates) g->draw(window);
    }

    std::vector<Gate*> Circuit::findGatesOfType(GateType t) {
        std::vector<Gate*> result;
        for (auto& g : gates) {
            if (g->getType() == t) result.push_back(g.get());
        }
        return result;
    }
}