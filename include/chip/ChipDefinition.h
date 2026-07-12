// include/chip/ChipDefinition.h
// Define o "molde" de um chip personalizado: metadados + circuito interno.
// É compartilhado (shared_ptr) por todas as instâncias (CustomChip).
#ifndef CHIP_CHIPDEFINITION_H
#define CHIP_CHIPDEFINITION_H

#include <string>
#include <vector>
#include <memory>
#include <set>
#include "chip/Circuit.h"

namespace chip {

// Metadados e conteúdo de um chip reutilizável.
struct PinSpec {
    std::string name;
    // (no futuro: tipo de bit, largura de barramento, etc.)
};

class ChipDefinition {
public:
    using Ptr = std::shared_ptr<ChipDefinition>;

    std::string id;            // identificador único (ex.: "uuid" ou nome)
    std::string name;          // nome de exibição (ex.: "HalfAdder")
    std::string description;   // descrição livre
    std::string category;      // categoria na biblioteca
    int version = 1;           // versão (incrementa ao editar)

    // Interface externa (ordem = ordem dos pinos do CustomChip).
    std::vector<PinSpec> inputs;
    std::vector<PinSpec> outputs;

    // Circuito interno (propriedade exclusiva da definição).
    std::shared_ptr<Circuit> circuit = std::make_shared<Circuit>();

    // IDs dos chips dos quais este depende (para detecção de ciclos e
    // para serialização: garante que as definições dependentes existam).
    std::set<std::string> dependencies;

    // Recalcula inputs/outputs a partir dos InputPin/OutputPin do circuito
    // e preenche 'dependencies' com os IDs de CustomChips internos.
    void refreshFromCircuit();

    // Valida se o circuito tem ao menos 1 entrada e 1 saída.
    bool isValid() const { return !inputs.empty() && !outputs.empty(); }
};

} // namespace chip

#endif // CHIP_CHIPDEFINITION_H