#include "chip/ChipDefinition.h"
#include "LogicEntities.h"

namespace chip {
    void ChipDefinition::refreshFromCircuit() {
        inputs.clear();
        outputs.clear();
        dependencies.clear();

        if (!circuit) return;

        for (auto& g : circuit->gates) {
            if (g->getType() == GateType::INPUT_PIN) {
                inputs.push_back({g->getPinName()});
            } else if (g->getType() == GateType::OUTPUT_PIN) {
                outputs.push_back({g->getPinName()});
            } else if (g->getType() == GateType::CHIP) {
                std::string cid = g->getChipId();
                if (!cid.empty()) dependencies.insert(cid);
            }
        }
    }
}