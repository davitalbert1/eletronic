#ifndef CHIP_CHIPFACTORY_H
#define CHIP_CHIPFACTORY_H

#include <string>
#include <memory>
#include <vector>
#include "Gate.h"
#include "Wire.h"
#include "chip/ChipDefinition.h"
#include "chip/Json.h"

// Forward declaration (definido em LogicEntities.h, namespace global).
class CustomChip;

namespace chip {

// Cria um CustomChip a partir de uma definição registrada (por id ou ptr).
std::shared_ptr<CustomChip> createChipInstance(const std::string& chipId,
                                               sf::Vector2f pos,
                                               const sf::Font& font);

std::shared_ptr<CustomChip> createChipInstance(const ChipDefinition::Ptr& def,
                                               sf::Vector2f pos,
                                               const sf::Font& font);

// ---- Serialização de CHIPS (JSON) ----
// Converte uma definição (com circuito interno) em JSON.
JsonValue chipDefinitionToJson(const ChipDefinition::Ptr& def);
// Reconstrói uma definição a partir de JSON. Não a registra.
ChipDefinition::Ptr chipDefinitionFromJson(const JsonValue& v, const sf::Font& font);

// Salva/carrega uma definição em arquivo .chip (JSON).
bool saveChipDefinition(const ChipDefinition::Ptr& def, const std::string& path);
ChipDefinition::Ptr loadChipDefinition(const std::string& path, const sf::Font& font);

// ---- Serialização de PROJETOS (JSON) ----
// Um projeto é: gates (com tipo, posição, extra, chipId/pinName) + wires.
// Chips referenciados são embutidos via "chipLibrary" para autossuficiência.
JsonValue projectToJson(const std::vector<std::shared_ptr<Gate>>& gates,
                        const std::vector<Wire>& wires,
                        const std::string& name,
                        const std::string& category,
                        const std::string& description,
                        const std::string& tags);

bool projectFromJson(const JsonValue& v,
                     std::vector<std::shared_ptr<Gate>>& gates,
                     std::vector<Wire>& wires,
                     std::string& name,
                     std::string& category,
                     std::string& description,
                     std::string& tags,
                     const sf::Font& font);

} // namespace chip

#endif // CHIP_CHIPFACTORY_H