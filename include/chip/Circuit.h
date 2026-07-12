// include/chip/Circuit.h
// Representa o circuito interno de um chip: uma coleção de gates e wires.
// É o "coração" reutilizável. Pode ser clonado para criar instâncias
// independentes (cada CustomChip tem sua própria cópia do Circuit).
#ifndef CHIP_CIRCUIT_H
#define CHIP_CIRCUIT_H

#include <vector>
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"

namespace chip {

// Forward declarations
class ChipDefinition;

// Um circuito é simplesmente gates + wires. Ele sabe como simular a si
// mesmo e como se clonar (deep copy) para instanciamento.
class Circuit {
public:
    Circuit() = default;

    // Gates e wires do circuito (propriedade exclusiva do Circuit).
    std::vector<std::shared_ptr<Gate>> gates;
    std::vector<Wire> wires;

    // Cria uma cópia profunda independente deste circuito.
    // Usa Gate::clone() para duplicar cada componente e reconecta os
    // fios pelas posições (índice de gate + índice de pino), que são
    // estáveis após o clone.
    std::shared_ptr<Circuit> clone(const sf::Font& font) const;

    // Executa um passo de simulação (reset de entradas, propagação de
    // fios, updateLogic de cada gate). Idêntico ao updatePropagation()
    // do Workbench, mas autocontido.
    void simulate();

    // Desenha o circuito (usado no editor interno / preview).
    void draw(sf::RenderWindow& window);

    // Coleta os gates de um tipo específico (ex.: INPUT_PIN/OUTPUT_PIN).
    std::vector<Gate*> findGatesOfType(GateType t);
};

} // namespace chip

#endif // CHIP_CIRCUIT_H