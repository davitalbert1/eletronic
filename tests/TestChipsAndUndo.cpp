#include "tests/TestFramework.h"
#include "chip/ChipDefinition.h"
#include "chip/ChipLibrary.h"
#include "UndoRedoManager.h"
#include "LogicEntities.h"
#include "Wire.h"
#include <SFML/Graphics.hpp>
#include <memory>

namespace {
    sf::Font& getTestFont() {
        static sf::Font font;
        static bool loaded = false;
        if (!loaded) {
            font.openFromFile("C:\\Windows\\Fonts\\arial.ttf");
            loaded = true;
        }
        return font;
    }
}

TEST_CASE(ChipLibrary, RegistrationAndLookup) {
    auto& lib = chip::ChipLibrary::instance();
    lib.clear();

    auto def = std::make_shared<chip::ChipDefinition>();
    def->id = "XOR_CHIP";
    def->name = "Custom XOR";
    def->category = "Logic";
    def->inputs.push_back({"A"});
    def->inputs.push_back({"B"});
    def->outputs.push_back({"Q"});

    TEST_ASSERT_TRUE(def->isValid());

    bool regOk = lib.registerChip(def);
    TEST_ASSERT_TRUE(regOk);

    auto found = lib.findById("XOR_CHIP");
    TEST_ASSERT_TRUE(found != nullptr);
    TEST_ASSERT_EQ(found->name, "Custom XOR");

    auto byCat = lib.byCategory("Logic");
    TEST_ASSERT_EQ(byCat.size(), 1);

    lib.unregister("XOR_CHIP");
    TEST_ASSERT_TRUE(lib.findById("XOR_CHIP") == nullptr);
}

TEST_CASE(ChipLibrary, CycleDetection) {
    auto& lib = chip::ChipLibrary::instance();
    lib.clear();

    // Registra Chip A
    auto chipA = std::make_shared<chip::ChipDefinition>();
    chipA->id = "CHIP_A";
    chipA->inputs.push_back({"in"});
    chipA->outputs.push_back({"out"});
    lib.registerChip(chipA);

    // Registra Chip B dependendo de A
    auto chipB = std::make_shared<chip::ChipDefinition>();
    chipB->id = "CHIP_B";
    chipB->inputs.push_back({"in"});
    chipB->outputs.push_back({"out"});
    chipB->dependencies.insert("CHIP_A");
    lib.registerChip(chipB);

    // Agora testar se adicionar uma dependência de B no Chip A criaria ciclo (A -> B -> A)
    bool hasCycle = lib.wouldCreateCycle("CHIP_A", {"CHIP_B"});
    TEST_ASSERT_TRUE(hasCycle);

    // Dependência sem ciclo (C -> A)
    bool noCycle = lib.wouldCreateCycle("CHIP_C", {"CHIP_A"});
    TEST_ASSERT_FALSE(noCycle);
}

TEST_CASE(UndoRedo, StackOperations) {
    auto& font = getTestFont();
    UndoRedoManager manager(10);

    std::vector<std::shared_ptr<Gate>> gates;
    std::vector<Wire> wires;

    TEST_ASSERT_FALSE(manager.canUndo());
    TEST_ASSERT_FALSE(manager.canRedo());

    // Ponto de restauração antes de adicionar o primeiro componente (estado vazio)
    manager.saveState(gates, wires);
    TEST_ASSERT_TRUE(manager.canUndo());

    // Estado 1: adiciona primeiro componente
    gates.push_back(std::make_shared<ANDGate>(sf::Vector2f(0, 0), font));
    // Salva o ponto antes de adicionar o segundo componente
    manager.saveState(gates, wires);

    // Estado 2: adiciona segundo componente
    gates.push_back(std::make_shared<ORGate>(sf::Vector2f(50, 50), font));
    TEST_ASSERT_EQ(gates.size(), 2);

    // Executa Undo: deve restaurar para o estado com 1 componente
    bool undoOk = manager.undo(gates, wires);
    TEST_ASSERT_TRUE(undoOk);
    TEST_ASSERT_EQ(gates.size(), 1);
    TEST_ASSERT_TRUE(manager.canRedo());

    // Executa Redo: deve voltar para 2 componentes
    bool redoOk = manager.redo(gates, wires);
    TEST_ASSERT_TRUE(redoOk);
    TEST_ASSERT_EQ(gates.size(), 2);
}
