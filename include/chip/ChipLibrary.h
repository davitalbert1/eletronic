// include/chip/ChipLibrary.h
// Registro global de definições de chips. Responsável por:
//  - registrar/remover chips;
//  - detectar dependências circulares (proteção anti-loop);
//  - buscar definições por id ou nome;
//  - listar chips de uma categoria (para a paleta).
#ifndef CHIP_CHIPLIBRARY_H
#define CHIP_CHIPLIBRARY_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "chip/ChipDefinition.h"

namespace chip {

class ChipLibrary {
public:
    // Singleton simples para acesso global a partir do Workbench e da fábrica.
    static ChipLibrary& instance();

    // Registra (ou substitui) uma definição. Retorna false se isso criaria
    // uma dependência circular (o próprio chip ou um ancestral nele).
    bool registerChip(const ChipDefinition::Ptr& def);

    // Remove uma definição pelo id. Não remove se houver instâncias ativas
    // (opcional: pode-se checar antes na UI).
    void unregister(const std::string& id);

    ChipDefinition::Ptr findById(const std::string& id) const;
    ChipDefinition::Ptr findByName(const std::string& name) const;

    // Verifica se 'candidate' (com suas dependências) pode depender de
    // 'targetId' sem formar ciclo. Usado antes de registrar/atualizar.
    bool wouldCreateCycle(const std::string& candidateId,
                          const std::set<std::string>& candidateDeps) const;

    // Lista todos os chips (para serialização/exportação).
    std::vector<ChipDefinition::Ptr> all() const;

    // Lista chips de uma categoria (para a paleta de componentes).
    std::vector<ChipDefinition::Ptr> byCategory(const std::string& cat) const;

    void clear();

private:
    ChipLibrary() = default;
    std::unordered_map<std::string, ChipDefinition::Ptr> defs_;

    // DFS para detectar se 'from' alcança 'to' seguindo dependencies.
    bool reaches(const std::string& from, const std::string& to,
                 std::set<std::string>& visited) const;
};

} // namespace chip

#endif // CHIP_CHIPLIBRARY_H