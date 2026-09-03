#pragma once
#include <vector>
#include <memory>
#include "LogicEntities.h"
#include "Wire.h"

class UndoRedoManager {
    public:
        struct StateSnapshot {
            std::vector<std::shared_ptr<Gate>> gates;
            std::vector<Wire> wires;
        };

        UndoRedoManager(size_t maxStates = 50);
        ~UndoRedoManager() = default;

        void saveState(const std::vector<std::shared_ptr<Gate>>& gates, const std::vector<Wire>& wires);
        bool undo(std::vector<std::shared_ptr<Gate>>& gates, std::vector<Wire>& wires);

        bool redo(std::vector<std::shared_ptr<Gate>>& gates, std::vector<Wire>& wires);
        bool canUndo() const;
        bool canRedo() const;
        void clear();
        size_t getUndoCount() const;
        size_t getRedoCount() const;
        void setMaxStates(size_t maxStates);
    private:
        std::vector<StateSnapshot> undoStack;
        std::vector<StateSnapshot> redoStack;
        size_t maxStates;
        void trimUndoStack();
        void trimRedoStack();
};

