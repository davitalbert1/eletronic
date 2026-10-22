#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <SFML/Graphics.hpp>
#include "Gate.h"

// GateRegistry centraliza mapeamentos de GateType, labels de exibição,
// nomes literais em arquivos/linguagem CDL e fábricas de instanciação.
// Isso evita ter que atualizar GateLabels e múltiplos switch-cases espalhados
// pelo código sempre que novos tipos ou componentes forem adicionados ou editados.
struct GateTypeInfo {
    GateType type;
    std::string codeName; // Nome usado em CDL / JSON / XML (ex: "AND", "CLOCK", "7SEG")
    std::string displayName; // Rótulo na UI / Canvas (ex: "AND", "CLOCK", "7SEG")
    std::function<std::shared_ptr<Gate>(sf::Vector2f pos, const sf::Font& font, const std::string& extraParam)> factory;
};

class GateRegistry {
public:
    static GateRegistry& instance();

    // Obter o label para um GateType (substitui o antigo array GateLabels)
    std::string getLabel(GateType type) const;

    // Obter o código textual (string) para exportações/CDL
    std::string getCodeName(GateType type) const;

    // Converter string para GateType
    GateType stringToType(const std::string& name) const;

    // Criar instância de componente dinamicamente
    std::shared_ptr<Gate> createGate(GateType type, sf::Vector2f pos, const sf::Font& font, const std::string& extraParam = "") const;
    std::shared_ptr<Gate> createGate(const std::string& name, sf::Vector2f pos, const sf::Font& font, const std::string& extraParam = "") const;

    // Todos os tipos suportados para CDL / exportação
    std::vector<std::string> getAllCodeNames() const;

private:
    GateRegistry();
    void registerType(GateType type, const std::string& codeName, const std::string& displayName,
                      std::function<std::shared_ptr<Gate>(sf::Vector2f, const sf::Font&, const std::string&)> factory);

    std::map<GateType, GateTypeInfo> typeMap_;
    std::map<std::string, GateType> nameToTypeMap_;
};
