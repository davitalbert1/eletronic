// src/chip/Json.cpp
#include "chip/Json.h"
#include <sstream>
#include <cstdio>

namespace chip {

namespace {

void escapeString(std::ostream& os, const std::string& s) {
    os << '"';
    for (char c : s) {
        switch (c) {
            case '"':
                os << "\\\"";
                break;
            case '\\':
                os << "\\\\";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\t':
                os << "\\t";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\f':
                os << "\\f";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    os << buf;
                } else {
                    os << c;
                }
        }
    }
    os << '"';
}

void writeValue(std::ostream& os, const JsonValue& v, int indent) {
    std::string pad(indent, ' ');
    std::string pad2(indent + 2, ' ');
    switch (v.type) {
        case JsonValue::Type::Null:   os << "null"; break;
        case JsonValue::Type::Bool:   os << (v.b ? "true" : "false"); break;
        case JsonValue::Type::Number: {
            // Print integers without trailing .0 when possible
            double n = v.num;
            if (n == (double)(long long)n) os << (long long)n;
            else os << n;
            break;
        }
        case JsonValue::Type::String: escapeString(os, v.str); break;
        case JsonValue::Type::Array: {
            if (v.arr.empty()) { os << "[]"; break; }
            os << "[\n";
            for (size_t i = 0; i < v.arr.size(); ++i) {
                os << pad2;
                writeValue(os, v.arr[i], indent + 2);
                if (i + 1 < v.arr.size()) os << ",";
                os << "\n";
            }
            os << pad << "]";
            break;
        }
        case JsonValue::Type::Object: {
            if (v.obj.empty()) { os << "{}"; break; }
            os << "{\n";
            for (size_t i = 0; i < v.obj.size(); ++i) {
                os << pad2;
                escapeString(os, v.obj[i].first);
                os << ": ";
                writeValue(os, v.obj[i].second, indent + 2);
                if (i + 1 < v.obj.size()) os << ",";
                os << "\n";
            }
            os << pad << "}";
            break;
        }
    }
}

struct Parser {
    const std::string& s;
    size_t i = 0;
    explicit Parser(const std::string& text) : s(text) {}

    void skipWs() {
        while (i < s.size()) {
            char c = s[i];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++i;
            else if (c == '/' && i + 1 < s.size() && s[i+1] == '/') {
                while (i < s.size() && s[i] != '\n') ++i;
            } else if (c == '/' && i + 1 < s.size() && s[i+1] == '*') {
                i += 2;
                while (i + 1 < s.size() && !(s[i] == '*' && s[i+1] == '/')) ++i;
                i += 2;
            } else break;
        }
    }

    char peek() {
        skipWs();
        return i < s.size() ? s[i] : '\0';
    }
    char next() {
        skipWs();
        return i < s.size() ? s[i++] : '\0';
    }

    [[noreturn]] void err(const std::string& msg) {
        throw std::runtime_error("JSON parse error at " + std::to_string(i) + ": " + msg);
    }

    JsonValue parseValue() {
        char c = peek();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return JsonValue(parseString());
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') { expect("null"); return JsonValue(); }
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
        err("unexpected character");
    }

    JsonValue parseObject() {
        JsonValue v = JsonValue::makeObject();
        next(); // {
        skipWs();
        if (peek() == '}') { next(); return v; }
        while (true) {
            skipWs();
            if (peek() != '"') err("expected string key");
            std::string key = parseString();
            if (next() != ':') err("expected ':'");
            JsonValue val = parseValue();
            v.set(key, std::move(val));
            char c = next();
            if (c == ',') continue;
            if (c == '}') break;
            err("expected ',' or '}'");
        }
        return v;
    }

    JsonValue parseArray() {
        JsonValue v = JsonValue::makeArray();
        next(); // [
        skipWs();
        if (peek() == ']') { next(); return v; }
        while (true) {
            v.push(parseValue());
            char c = next();
            if (c == ',') continue;
            if (c == ']') break;
            err("expected ',' or ']'");
        }
        return v;
    }

    std::string parseString() {
        if (next() != '"') err("expected '\"'");
        std::string out;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') return out;
            if (c == '\\') {
                if (i >= s.size()) err("bad escape");
                char e = s[i++];
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'u': {
                        if (i + 4 > s.size()) err("bad unicode escape");
                        unsigned code = 0;
                        for (int k = 0; k < 4; ++k) {
                            char h = s[i++];
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= (h - '0');
                            else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                            else err("bad hex digit");
                        }
                        // Encode as UTF-8 (basic BMP)
                        if (code < 0x80) out += (char)code;
                        else if (code < 0x800) {
                            out += (char)(0xC0 | (code >> 6));
                            out += (char)(0x80 | (code & 0x3F));
                        } else {
                            out += (char)(0xE0 | (code >> 12));
                            out += (char)(0x80 | ((code >> 6) & 0x3F));
                            out += (char)(0x80 | (code & 0x3F));
                        }
                        break;
                    }
                    default: err("bad escape char");
                }
            } else {
                out += c;
            }
        }
        err("unterminated string");
    }

    JsonValue parseNumber() {
        size_t start = i;
        if (peek() == '-') next();
        while (i < s.size() && ((s[i] >= '0' && s[i] <= '9') || s[i] == '.' || s[i] == 'e' || s[i] == 'E' || s[i] == '+' || s[i] == '-'))
            ++i;
        try {
            return JsonValue(std::stod(s.substr(start, i - start)));
        } catch (...) {
            err("bad number");
        }
    }

    JsonValue parseBool() {
        if (s.compare(i, 4, "true") == 0) { i += 4; return JsonValue(true); }
        if (s.compare(i, 5, "false") == 0) { i += 5; return JsonValue(false); }
        err("bad literal");
    }

    void expect(const char* lit) {
        size_t n = std::string(lit).size();
        if (s.compare(i, n, lit) != 0) err(std::string("expected ") + lit);
        i += n;
    }
};

} // namespace

std::string jsonToString(const JsonValue& v, int indent) {
    std::ostringstream os;
    writeValue(os, v, indent);
    return os.str();
}

JsonValue jsonParse(const std::string& text) {
    Parser p(text);
    JsonValue v = p.parseValue();
    p.skipWs();
    if (p.i != text.size()) p.err("trailing characters");
    return v;
}

} // namespace chip