#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"

// Define CircuitTokenType before SFML includes to avoid potential macro conflicts
// with Windows headers (windows.h defines many macros)
enum class CircuitTokenType {
    IDENTIFIER,
    NUMBER,
    STRING,
    CIRCUIT,
    GATE,
    WIRE,
    INPUT,
    OUTPUT,
    CHIP,
    PARAM,
    GROUP,
    COMMENT,
    AT,
    ARROW,
    DOT,
    EQUALS,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    COMMA,
    COLON,
    SEMICOLON,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    CircuitTokenType type;
    std::string value;
    int line;
    int column;

    Token() : type(CircuitTokenType::UNKNOWN), line(0), column(0) {}
    Token(CircuitTokenType t, const std::string& v, int l, int c) : type(t), value(v), line(l), column(c) {}
};

struct ASTNode {
    virtual ~ASTNode() = default;
    int line = 0;
};

struct GateNode : ASTNode {
    std::string gateType;
    std::string id;
    float x = 0, y = 0;
    std::map<std::string, std::string> params;
};

struct WireNode : ASTNode {
    std::string fromGate;
    int fromPin = 0;
    std::string toGate;
    int toPin = 0;
};

struct InputNode : ASTNode {
    std::string id;
    float x = 0, y = 0;
};

struct OutputNode : ASTNode {
    std::string id;
    float x = 0, y = 0;
};

struct ChipNode : ASTNode {
    std::string chipId;
    std::string instanceId;
    float x = 0, y = 0;
};

struct GroupNode : ASTNode {
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> children;
};

struct CircuitNode : ASTNode {
    std::string name;
    std::string description;
    std::vector<std::shared_ptr<ASTNode>> children;
};

struct CircuitParseResult {
    bool success = false;
    std::string errorMessage;
    std::string circuitName;
    std::string circuitDescription;
    std::vector<std::shared_ptr<Gate>> gates;
    std::vector<Wire> wires;
    std::vector<std::string> warnings;
};

class CircuitLanguageParser {
public:
    CircuitLanguageParser() = default;

    CircuitParseResult parse(const std::string& source, const sf::Font& font);
    CircuitParseResult parseFromFile(const std::string& filePath, const sf::Font& font);

    std::string generateDescription(
        const std::vector<std::shared_ptr<Gate>>& gates,
        const std::vector<Wire>& wires,
        const std::string& circuitName,
        const std::string& description = ""
    );

    static std::vector<std::string> supportedGateTypes();
    static std::string getLanguageHelp();

private:
    std::vector<Token> tokenize(const std::string& source);

    std::shared_ptr<CircuitNode> parseCircuit();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<GateNode> parseGate();
    std::shared_ptr<WireNode> parseWire();
    std::shared_ptr<InputNode> parseInput();
    std::shared_ptr<OutputNode> parseOutput();
    std::shared_ptr<ChipNode> parseChip();
    std::shared_ptr<GroupNode> parseGroup();
    void parseParams(std::map<std::string, std::string>& params);
    sf::Vector2f parsePosition();

    bool generateCircuit(
        const std::shared_ptr<CircuitNode>& ast,
        CircuitParseResult& result,
        const sf::Font& font
    );
    GateType mapGateType(const std::string& typeName, std::string& outLabel);

    bool match(CircuitTokenType type);
    bool check(CircuitTokenType type) const;
    const Token& current() const;
    const Token& previous() const;
    bool isAtEnd() const;
    void advance();
    void error(const std::string& message);
    void warning(const std::string& message);

    std::vector<Token> tokens_;
    size_t currentToken_ = 0;
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
    std::map<std::string, std::shared_ptr<Gate>> gateMap_;
    std::string source_;
};
