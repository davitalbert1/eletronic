#include "tests/TestFramework.h"
#include "LogicEntities.h"
#include "GateRegistry.h"
#include "Wire.h"
#include <SFML/Graphics.hpp>
#include <memory>

namespace {
    // Fonte dummy reutilizável nos testes de lógica
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

TEST_CASE(LogicGates, ANDGateTruthTable) {
    auto& font = getTestFont();
    ANDGate andGate({0, 0}, font);
    TEST_ASSERT_EQ(andGate.inputs.size(), 2);
    TEST_ASSERT_EQ(andGate.outputs.size(), 1);

    // 0 & 0 -> 0
    andGate.inputs[0].state = false;
    andGate.inputs[1].state = false;
    andGate.updateLogic();
    TEST_ASSERT_FALSE(andGate.outputs[0].state);

    // 0 & 1 -> 0
    andGate.inputs[0].state = false;
    andGate.inputs[1].state = true;
    andGate.updateLogic();
    TEST_ASSERT_FALSE(andGate.outputs[0].state);

    // 1 & 0 -> 0
    andGate.inputs[0].state = true;
    andGate.inputs[1].state = false;
    andGate.updateLogic();
    TEST_ASSERT_FALSE(andGate.outputs[0].state);

    // 1 & 1 -> 1
    andGate.inputs[0].state = true;
    andGate.inputs[1].state = true;
    andGate.updateLogic();
    TEST_ASSERT_TRUE(andGate.outputs[0].state);
}

TEST_CASE(LogicGates, ORGateTruthTable) {
    auto& font = getTestFont();
    ORGate orGate({0, 0}, font);
    TEST_ASSERT_EQ(orGate.inputs.size(), 2);
    TEST_ASSERT_EQ(orGate.outputs.size(), 1);

    // 0 | 0 -> 0
    orGate.inputs[0].state = false;
    orGate.inputs[1].state = false;
    orGate.updateLogic();
    TEST_ASSERT_FALSE(orGate.outputs[0].state);

    // 0 | 1 -> 1
    orGate.inputs[0].state = false;
    orGate.inputs[1].state = true;
    orGate.updateLogic();
    TEST_ASSERT_TRUE(orGate.outputs[0].state);

    // 1 | 0 -> 1
    orGate.inputs[0].state = true;
    orGate.inputs[1].state = false;
    orGate.updateLogic();
    TEST_ASSERT_TRUE(orGate.outputs[0].state);

    // 1 | 1 -> 1
    orGate.inputs[0].state = true;
    orGate.inputs[1].state = true;
    orGate.updateLogic();
    TEST_ASSERT_TRUE(orGate.outputs[0].state);
}

TEST_CASE(LogicGates, NOTGateTruthTable) {
    auto& font = getTestFont();
    NOTGate notGate({0, 0}, font);
    TEST_ASSERT_EQ(notGate.inputs.size(), 1);
    TEST_ASSERT_EQ(notGate.outputs.size(), 1);

    // !0 -> 1
    notGate.inputs[0].state = false;
    notGate.updateLogic();
    TEST_ASSERT_TRUE(notGate.outputs[0].state);

    // !1 -> 0
    notGate.inputs[0].state = true;
    notGate.updateLogic();
    TEST_ASSERT_FALSE(notGate.outputs[0].state);
}

TEST_CASE(LogicGates, XORGateTruthTable) {
    auto& font = getTestFont();
    XORGate xorGate({0, 0}, font);
    TEST_ASSERT_EQ(xorGate.inputs.size(), 2);
    TEST_ASSERT_EQ(xorGate.outputs.size(), 1);

    xorGate.inputs[0].state = false;
    xorGate.inputs[1].state = false;
    xorGate.updateLogic();
    TEST_ASSERT_FALSE(xorGate.outputs[0].state);

    xorGate.inputs[0].state = false;
    xorGate.inputs[1].state = true;
    xorGate.updateLogic();
    TEST_ASSERT_TRUE(xorGate.outputs[0].state);

    xorGate.inputs[0].state = true;
    xorGate.inputs[1].state = false;
    xorGate.updateLogic();
    TEST_ASSERT_TRUE(xorGate.outputs[0].state);

    xorGate.inputs[0].state = true;
    xorGate.inputs[1].state = true;
    xorGate.updateLogic();
    TEST_ASSERT_FALSE(xorGate.outputs[0].state);
}

TEST_CASE(LogicGates, NANDandNORTruthTable) {
    auto& font = getTestFont();
    NANDGate nandGate({0, 0}, font);
    NORGate norGate({0, 0}, font);

    // NAND
    nandGate.inputs[0].state = true;
    nandGate.inputs[1].state = true;
    nandGate.updateLogic();
    TEST_ASSERT_FALSE(nandGate.outputs[0].state);

    nandGate.inputs[0].state = true;
    nandGate.inputs[1].state = false;
    nandGate.updateLogic();
    TEST_ASSERT_TRUE(nandGate.outputs[0].state);

    // NOR
    norGate.inputs[0].state = false;
    norGate.inputs[1].state = false;
    norGate.updateLogic();
    TEST_ASSERT_TRUE(norGate.outputs[0].state);

    norGate.inputs[0].state = true;
    norGate.inputs[1].state = false;
    norGate.updateLogic();
    TEST_ASSERT_FALSE(norGate.outputs[0].state);
}

TEST_CASE(LogicGates, WireSignalPropagation) {
    auto& font = getTestFont();
    PowerSource power({0, 0}, font);
    NOTGate notGate({100, 0}, font);

    Wire wire(&power.outputs[0], &notGate.inputs[0]);

    // Power OFF -> notGate deve receber false -> saída NOT vira true
    power.setState(false);
    wire.update();
    TEST_ASSERT_FALSE(notGate.inputs[0].state);
    notGate.updateLogic();
    TEST_ASSERT_TRUE(notGate.outputs[0].state);

    // Power ON -> notGate deve receber true -> saída NOT vira false
    power.setState(true);
    wire.update();
    TEST_ASSERT_TRUE(notGate.inputs[0].state);
    notGate.updateLogic();
    TEST_ASSERT_FALSE(notGate.outputs[0].state);
}

TEST_CASE(LogicGates, GateRegistryLookup) {
    auto& reg = GateRegistry::instance();

    TEST_ASSERT_EQ(reg.getLabel(GateType::AND), "AND");
    TEST_ASSERT_EQ(reg.getCodeName(GateType::AND), "AND");
    TEST_ASSERT_EQ(reg.stringToType("AND"), GateType::AND);
    TEST_ASSERT_EQ(reg.stringToType("OR"), GateType::OR);
    TEST_ASSERT_EQ(reg.stringToType("NOT"), GateType::NOT);
    TEST_ASSERT_EQ(reg.stringToType("XOR"), GateType::XOR);

    auto& font = getTestFont();
    auto gate = reg.createGate(GateType::AND, {10, 20}, font);
    TEST_ASSERT_TRUE(gate != nullptr);
    TEST_ASSERT_EQ(gate->getType(), GateType::AND);
    TEST_ASSERT_EQ(gate->getPosition().x, 10.0f);
    TEST_ASSERT_EQ(gate->getPosition().y, 20.0f);
}
