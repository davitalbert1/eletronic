#include "tests/TestFramework.h"
#include "chip/Json.h"
#include <string>

using namespace chip;

TEST_CASE(JsonParser, ParsePrimitives) {
    JsonValue numVal = jsonParse("123.45");
    TEST_ASSERT_EQ(numVal.type, JsonValue::Type::Number);
    TEST_ASSERT_NEAR(numVal.asNumber(), 123.45, 0.0001);
    TEST_ASSERT_EQ(numVal.asInt(), 123);

    JsonValue strVal = jsonParse("\"hello world\"");
    TEST_ASSERT_EQ(strVal.type, JsonValue::Type::String);
    TEST_ASSERT_EQ(strVal.asString(), "hello world");

    JsonValue boolTrue = jsonParse("true");
    TEST_ASSERT_EQ(boolTrue.type, JsonValue::Type::Bool);
    TEST_ASSERT_TRUE(boolTrue.asBool());

    JsonValue boolFalse = jsonParse("false");
    TEST_ASSERT_EQ(boolFalse.type, JsonValue::Type::Bool);
    TEST_ASSERT_FALSE(boolFalse.asBool());

    JsonValue nullVal = jsonParse("null");
    TEST_ASSERT_TRUE(nullVal.isNull());
}

TEST_CASE(JsonParser, ParseObjectAndArray) {
    std::string jsonStr = R"({
        "name": "AdderCircuit",
        "gates": 4,
        "active": true,
        "pins": ["A", "B", "Cin"]
    })";

    JsonValue root = jsonParse(jsonStr);
    TEST_ASSERT_TRUE(root.isObject());
    TEST_ASSERT_TRUE(root.has("name"));
    TEST_ASSERT_TRUE(root.has("gates"));
    TEST_ASSERT_TRUE(root.has("active"));
    TEST_ASSERT_TRUE(root.has("pins"));

    TEST_ASSERT_EQ(root.get("name").asString(), "AdderCircuit");
    TEST_ASSERT_EQ(root.get("gates").asInt(), 4);
    TEST_ASSERT_TRUE(root.get("active").asBool());

    const auto& pins = root.get("pins");
    TEST_ASSERT_TRUE(pins.isArray());
    TEST_ASSERT_EQ(pins.arr.size(), 3);
    TEST_ASSERT_EQ(pins.arr[0].asString(), "A");
    TEST_ASSERT_EQ(pins.arr[1].asString(), "B");
    TEST_ASSERT_EQ(pins.arr[2].asString(), "Cin");
}

TEST_CASE(JsonParser, SerializeAndDeserialize) {
    JsonValue obj = JsonValue::makeObject();
    obj.set("title", JsonValue("TestProject"));
    obj.set("version", JsonValue(2));

    JsonValue arr = JsonValue::makeArray();
    arr.push(JsonValue("gate1"));
    arr.push(JsonValue("gate2"));
    obj.set("list", arr);

    std::string text = jsonToString(obj, 2);
    TEST_ASSERT_TRUE(!text.empty());

    JsonValue parsed = jsonParse(text);
    TEST_ASSERT_TRUE(parsed.isObject());
    TEST_ASSERT_EQ(parsed.get("title").asString(), "TestProject");
    TEST_ASSERT_EQ(parsed.get("version").asInt(), 2);
    TEST_ASSERT_EQ(parsed.get("list").arr.size(), 2);
}
