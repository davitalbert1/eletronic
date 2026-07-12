#ifndef CHIP_JSON_H
#define CHIP_JSON_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

namespace chip {
    struct JsonValue {
        enum class Type { Null, Bool, Number, String, Array, Object };
        Type type = Type::Null;

        bool b = false;
        double num = 0.0;
        std::string str;
        std::vector<JsonValue> arr;
        std::vector<std::pair<std::string, JsonValue>> obj; // ordered map

        JsonValue() = default;
        JsonValue(bool v) : type(Type::Bool), b(v) {}
        JsonValue(double v) : type(Type::Number), num(v) {}
        JsonValue(int v) : type(Type::Number), num(static_cast<double>(v)) {}
        JsonValue(const std::string& v) : type(Type::String), str(v) {}
        JsonValue(const char* v) : type(Type::String), str(v) {}

        static JsonValue makeArray() {
            JsonValue v;
            v.type = Type::Array;
            return v;
            }
        static JsonValue makeObject() {
            JsonValue v;
            v.type = Type::Object;
            return v;
        }

        bool isNull() const {
            return type == Type::Null;
        }
        bool isObject() const {
            return type == Type::Object;
        }
        bool isArray() const {
            return type == Type::Array;
        }

        // Object helpers
        void set(const std::string& key, JsonValue v) {
            for (auto& kv : obj) {
                if (kv.first == key) {
                    kv.second = std::move(v);
                    return;
                }
            }
            obj.emplace_back(key, std::move(v));
        }
        bool has(const std::string& key) const {
            for (auto& kv : obj) if (kv.first == key) return true;
            return false;
        }
        const JsonValue& get(const std::string& key) const {
            static JsonValue nullVal;
            for (auto& kv : obj) {
                if (kv.first == key) return kv.second;
            }
            return nullVal;
        }
        void push(JsonValue v) {
            arr.push_back(std::move(v));
        }

        std::string asString(const std::string& def = "") const {
            return type == Type::String ? str : def;
        }
        double asNumber(double def = 0.0) const {
            return type == Type::Number ? num : def;
        }
        int asInt(int def = 0) const {
            return type == Type::Number ? static_cast<int>(num) : def;
        }
        bool asBool(bool def = false) const {
            return type == Type::Bool ? b : def;
        }
    };

    std::string jsonToString(const JsonValue& v, int indent = 0);
    JsonValue jsonParse(const std::string& text);

}

#endif