#include "chip/ChipLibrary.h"

namespace chip {
    ChipLibrary& ChipLibrary::instance() {
        static ChipLibrary lib;
        return lib;
    }

    bool ChipLibrary::registerChip(const ChipDefinition::Ptr& def) {
        if (!def) return false;
        if (wouldCreateCycle(def->id, def->dependencies)) return false;
        defs_[def->id] = def;
        return true;
    }

    void ChipLibrary::unregister(const std::string& id) {
        defs_.erase(id);
    }

    ChipDefinition::Ptr ChipLibrary::findById(const std::string& id) const {
        auto it = defs_.find(id);
        return it == defs_.end() ? nullptr : it->second;
    }

    ChipDefinition::Ptr ChipLibrary::findByName(const std::string& name) const {
        for (auto& kv : defs_) {
            if (kv.second->name == name) return kv.second;
        }
        return nullptr;
    }

    bool ChipLibrary::wouldCreateCycle(const std::string& candidateId, const std::set<std::string>& candidateDeps) const {
        if (candidateDeps.count(candidateId)) return true;

        std::set<std::string> visited;
        for (const auto& dep : candidateDeps) {
            if (reaches(dep, candidateId, visited)) return true;
        }
        return false;
    }

    std::vector<ChipDefinition::Ptr> ChipLibrary::all() const {
        std::vector<ChipDefinition::Ptr> out;
        out.reserve(defs_.size());
        for (auto& kv : defs_) out.push_back(kv.second);
        return out;
    }

    std::vector<ChipDefinition::Ptr> ChipLibrary::byCategory(const std::string& cat) const {
        std::vector<ChipDefinition::Ptr> out;
        for (auto& kv : defs_) {
            if (kv.second->category == cat) out.push_back(kv.second);
        }
        return out;
    }

    void ChipLibrary::clear() {
        defs_.clear();
    }

    bool ChipLibrary::reaches(const std::string& from, const std::string& to, std::set<std::string>& visited) const {
        if (from == to) return true;
        if (visited.count(from)) return false;
        visited.insert(from);

        auto it = defs_.find(from);
        if (it == defs_.end()) return false;
        for (const auto& dep : it->second->dependencies) {
            if (reaches(dep, to, visited)) return true;
        }
        return false;
    }
}