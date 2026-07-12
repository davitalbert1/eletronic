#ifndef UNDO_REDO_MANAGER_HPP
#define UNDO_REDO_MANAGER_HPP

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

    // Salva o estado atual nas pilhas
    void saveState(const std::vector<std::shared_ptr<Gate>>& gates, 
                   const std::vector<Wire>& wires);

    // Desfaz a última ação
    bool undo(std::vector<std::shared_ptr<Gate>>& gates, 
              std::vector<Wire>& wires);

    // Refaz a última ação desfeita
    bool redo(std::vector<std::shared_ptr<Gate>>& gates, 
              std::vector<Wire>& wires);

    // Verifica se há ações para desfazer
    bool canUndo() const;

    // Verifica se há ações para refazer
    bool canRedo() const;

    // Limpa todas as pilhas
    void clear();

    // Retorna o número de estados na pilha de undo
    size_t getUndoCount() const;

    // Retorna o número de estados na pilha de redo
    size_t getRedoCount() const;

    // Define o número máximo de estados
    void setMaxStates(size_t maxStates);

private:
    std::vector<StateSnapshot> undoStack;
    std::vector<StateSnapshot> redoStack;
    size_t maxStates;

    // Remove estados antigos se exceder o limite
    void trimUndoStack();
    void trimRedoStack();
};

#endif // UNDO_REDO_MANAGER_HPP