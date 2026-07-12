#ifndef CHIP_CHIPDEFINITION_H
#define CHIP_CHIPDEFINITION_H
#include <string>
#include <vector>
#include <memory>
#include <set>
#include "chip/Circuit.h"

namespace chip {
    struct PinSpec {
        std::string name;
        // (no futuro: tipo de bit, largura de barramento, etc.)
    };

    class ChipDefinition {
    public:
        using Ptr = std::shared_ptr<ChipDefinition>;

        std::string id;
        std::string name;
        std::string description;
        std::string category;
        int version = 1;

        std::vector<PinSpec> inputs;
        std::vector<PinSpec> outputs;
        std::shared_ptr<Circuit> circuit = std::make_shared<Circuit>();
        std::set<std::string> dependencies;
        void refreshFromCircuit();
        bool isValid() const {
            return !inputs.empty() && !outputs.empty();
        }
    };
}

#endif