#include "UndoRedoManager.h"
#include <algorithm>

UndoRedoManager::UndoRedoManager(size_t maxStates) : maxStates(maxStates) {}

void UndoRedoManager::saveState(const std::vector<std::shared_ptr<Gate>>& gates,
                                 const std::vector<Wire>& wires) {
    StateSnapshot snapshot;
    snapshot.gates = gates;
    snapshot.wires = wires;

    undoStack.push_back(snapshot);
    trimUndoStack();

    // Limpa a pilha de redo quando uma nova ação é executada
    redoStack.clear();
}

bool UndoRedoManager::undo(std::vector<std::shared_ptr<Gate>>& gates,
                           std::vector<Wire>& wires) {
    if (undoStack.empty()) return false;

    // Salva o estado atual na pilha de redo
    StateSnapshot current;
    current.gates = gates;
    current.wires = wires;
    redoStack.push_back(current);
    trimRedoStack();

    // Restaura o estado anterior
    StateSnapshot prev = undoStack.back();
    gates = prev.gates;
    wires = prev.wires;
    undoStack.pop_back();

    return true;
}

bool UndoRedoManager::redo(std::vector<std::shared_ptr<Gate>>& gates,
                           std::vector<Wire>& wires) {
    if (redoStack.empty()) return false;

    // Salva o estado atual na pilha de undo
    StateSnapshot current;
    current.gates = gates;
    current.wires = wires;
    undoStack.push_back(current);
    trimUndoStack();

    // Restaura o estado do redo
    StateSnapshot next = redoStack.back();
    gates = next.gates;
    wires = next.wires;
    redoStack.pop_back();

    return true;
}

bool UndoRedoManager::canUndo() const {
    return !undoStack.empty();
}

bool UndoRedoManager::canRedo() const {
    return !redoStack.empty();
}

void UndoRedoManager::clear() {
    undoStack.clear();
    redoStack.clear();
}

size_t UndoRedoManager::getUndoCount() const {
    return undoStack.size();
}

size_t UndoRedoManager::getRedoCount() const {
    return redoStack.size();
}

void UndoRedoManager::setMaxStates(size_t maxStates) {
    this->maxStates = maxStates;
    trimUndoStack();
    trimRedoStack();
}

void UndoRedoManager::trimUndoStack() {
    while (undoStack.size() > maxStates) undoStack.erase(undoStack.begin());
}

void UndoRedoManager::trimRedoStack() {
    while (redoStack.size() > maxStates) redoStack.erase(redoStack.begin());
}