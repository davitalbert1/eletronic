#include "tests/TestFramework.h"
#include "DigitalLogicSimImporter.h"
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

TEST_CASE(DLSImporter, ImportSimpleCircuit) {
    auto& font = getTestFont();
    DigitalLogicSimImporter importer;

    std::string dlsJson = R"({
        "name": "DLS_Sample",
        "description": "Test DLS circuit",
        "version": 1,
        "components": [
            { "id": 0, "type": "AND", "label": "AND_1", "x": 100.0, "y": 150.0 },
            { "id": 1, "type": "Input", "label": "IN_1", "x": 20.0, "y": 140.0 },
            { "id": 2, "type": "Output", "label": "OUT_1", "x": 200.0, "y": 150.0 }
        ],
        "connections": [
            { "fromComponent": 1, "fromPin": 0, "toComponent": 0, "toPin": 0 },
            { "fromComponent": 0, "fromPin": 0, "toComponent": 2, "toPin": 0 }
        ]
    })";

    ImportResult result = importer.importFromJson(dlsJson, font);
    TEST_ASSERT_TRUE(result.success);
    TEST_ASSERT_EQ(result.projectName, "DLS_Sample");
    TEST_ASSERT_EQ(result.gates.size(), 3);
    TEST_ASSERT_EQ(result.wires.size(), 2);
    TEST_ASSERT_EQ(result.convertedComponents, 3);
    TEST_ASSERT_EQ(result.convertedConnections, 2);
}

TEST_CASE(DLSImporter, HandleCorruptedData) {
    auto& font = getTestFont();
    DigitalLogicSimImporter importer;

    // JSON corrompido ou sem campos obrigatórios
    std::string malformedJson = "{ invalid json data: [1, 2, ";

    ImportResult result = importer.importFromJson(malformedJson, font);
    TEST_ASSERT_FALSE(result.success);
    TEST_ASSERT_FALSE(result.errorMessage.empty());
}
