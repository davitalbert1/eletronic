#include "tests/TestFramework.h"
#include "CircuitLanguageParser.h"
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

TEST_CASE(CircuitParser, ParseValidCDL) {
    auto& font = getTestFont();
    CircuitLanguageParser parser;

    std::string validCode = 
        "circuit TestCircuit {\n"
        "  input in1 at (10, 20)\n"
        "  input in2 at (10, 60)\n"
        "  gate AND and1 at (100, 40)\n"
        "  output out1 at (200, 40)\n"
        "  wire in1.0 -> and1.0\n"
        "  wire in2.0 -> and1.1\n"
        "  wire and1.0 -> out1.0\n"
        "}\n";

    CircuitParseResult res = parser.parse(validCode, font);
    TEST_ASSERT_TRUE(res.success);
    TEST_ASSERT_EQ(res.circuitName, "TestCircuit");
    TEST_ASSERT_EQ(res.gates.size(), 4);
    TEST_ASSERT_EQ(res.wires.size(), 3);
}

TEST_CASE(CircuitParser, DetectSyntaxErrors) {
    auto& font = getTestFont();
    CircuitLanguageParser parser;

    // Falta fechar chave e sintaxe inválida
    std::string invalidCode = 
        "circuit BrokenCircuit {\n"
        "  gate INVALID_TYPE g1 at (10, 20)\n"
        "  wire g1 ->\n"
        "}\n";

    CircuitParseResult res = parser.parse(invalidCode, font);
    // Deve reportar falha com mensagem de erro apontando o problema
    TEST_ASSERT_FALSE(res.success);
    TEST_ASSERT_FALSE(res.errorMessage.empty());
}

TEST_CASE(CircuitParser, RoundTripExportAndParse) {
    auto& font = getTestFont();
    CircuitLanguageParser parser;

    std::string initialCode = 
        "circuit RoundTrip {\n"
        "  input a at (50, 50)\n"
        "  gate NOT not1 at (150, 50)\n"
        "  output y at (250, 50)\n"
        "  wire a.0 -> not1.0\n"
        "  wire not1.0 -> y.0\n"
        "}\n";

    CircuitParseResult parse1 = parser.parse(initialCode, font);
    TEST_ASSERT_TRUE(parse1.success);

    // Exportar para CDL novamente
    std::string generatedCDL = parser.generateDescription(parse1.gates, parse1.wires, "RoundTrip", "Exported");
    TEST_ASSERT_TRUE(!generatedCDL.empty());

    // Fazer parse do CDL gerado para garantir coerência
    CircuitParseResult parse2 = parser.parse(generatedCDL, font);
    TEST_ASSERT_TRUE(parse2.success);
    TEST_ASSERT_EQ(parse2.circuitName, "RoundTrip");
    TEST_ASSERT_EQ(parse2.gates.size(), 3);
    TEST_ASSERT_EQ(parse2.wires.size(), 2);
}
