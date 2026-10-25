#include "tests/TestFramework.h"
#include "CircuitSynthesizer.h"
#include <SFML/Graphics.hpp>

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

TEST_CASE(Synthesizer, ExpressionEvaluator) {
    CircuitSynthesizer synth;
    std::vector<std::string> inputs = {"A", "B"};

    // Testar expressão AND: "A & B"
    auto truthAnd = synth.evaluateExpression("A & B", inputs);
    TEST_ASSERT_EQ(truthAnd.size(), 4);
    // 00 -> 0, 01 -> 0, 10 -> 0, 11 -> 1
    TEST_ASSERT_EQ(truthAnd[0], 0);
    TEST_ASSERT_EQ(truthAnd[1], 0);
    TEST_ASSERT_EQ(truthAnd[2], 0);
    TEST_ASSERT_EQ(truthAnd[3], 1);

    // Testar expressão XOR: "A ^ B"
    auto truthXor = synth.evaluateExpression("A ^ B", inputs);
    TEST_ASSERT_EQ(truthXor.size(), 4);
    TEST_ASSERT_EQ(truthXor[0], 0);
    TEST_ASSERT_EQ(truthXor[1], 1);
    TEST_ASSERT_EQ(truthXor[2], 1);
    TEST_ASSERT_EQ(truthXor[3], 0);
}

TEST_CASE(Synthesizer, CandidateComparison) {
    CircuitCandidate a;
    a.valido = true;
    a.fora = 0.0;
    a.erro = 0.0;
    a.totalCost = 3;

    CircuitCandidate b;
    b.valido = true;
    b.fora = 0.0;
    b.erro = 0.0;
    b.totalCost = 5;

    // 'a' tem custo menor, então isBetter(a, b) deve ser verdadeiro
    TEST_ASSERT_TRUE(CircuitSynthesizer::isBetter(a, b));
    TEST_ASSERT_FALSE(CircuitSynthesizer::isBetter(b, a));
}

TEST_CASE(Synthesizer, SynthesizePresetHalfAdder) {
    auto& font = getTestFont();
    CircuitSynthesizer synth;

    SynthesisConfig cfg = CircuitSynthesizer::createPreset("Meio Somador (Half Adder)");
    TEST_ASSERT_EQ(cfg.inputs.size(), 2);
    TEST_ASSERT_EQ(cfg.outputs.size(), 2);

    CircuitCandidate result = synth.synthesize(cfg, font);
    TEST_ASSERT_TRUE(result.valido);
    TEST_ASSERT_TRUE(!result.cdlCode.empty());
    TEST_ASSERT_TRUE(!result.gates.empty());
}
