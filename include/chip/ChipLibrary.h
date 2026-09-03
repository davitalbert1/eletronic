#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "chip/ChipDefinition.h"

namespace chip {
    class ChipLibrary {
    public:
        // Singleton simples para acesso global a partir do Workbench e da fÃ¡brica.
        static ChipLibrary& instance();
        bool registerChip(const ChipDefinition::Ptr& def);
        void unregister(const std::string& id);

        ChipDefinition::Ptr findById(const std::string& id) const;
        ChipDefinition::Ptr findByName(const std::string& name) const;

        bool wouldCreateCycle(const std::string& candidateId, const std::set<std::string>& candidateDeps) const;
        std::vector<ChipDefinition::Ptr> all() const;
        std::vector<ChipDefinition::Ptr> byCategory(const std::string& cat) const;
        void clear();

    private:
        ChipLibrary() = default;
        std::unordered_map<std::string, ChipDefinition::Ptr> defs_;

        // DFS para detectar se 'from' alcanÃ§a 'to' seguindo dependencies.
        bool reaches(const std::string& from, const std::string& to,
                    std::set<std::string>& visited) const;
    };
}
