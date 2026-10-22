#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Gate.h"
#include "Wire.h"
#include "chip/ChipDefinition.h"

// Estrutura de configuração da síntese lógica.
// Inspirada no struct Config de dice.cpp (ranges, chances, dados).
struct SynthesisConfig {
    std::vector<std::string> inputs; // Nomes das entradas (ex: "A", "B")
    std::vector<std::string> outputs; // Nomes das saídas (ex: "S", "Cout")

    // Processamento esperado:
    // Modo 1: Tabela verdade esperada para cada saída (tamanho 2^N para N entradas)
    // -1 = Don't care ('X'), 0 = Falso, 1 = Verdadeiro
    std::map<std::string, std::vector<int>> targetTable;

    // Modo 2: Expressões booleanas esperadas para cada saída (ex: "S = A ^ B", "Cout = A & B")
    std::map<std::string, std::string> targetExpressions;

    // Primitivas disponíveis (equivalente aos 'dados' em dice.cpp):
    // Portas padrão permitidas (ex: "AND", "OR", "NOT", "XOR", "NAND", "NOR", "XNOR")
    std::vector<std::string> allowedGates;

    // Chips de usuário permitidos (IDs de ChipDefinition registrados na ChipLibrary)
    std::vector<std::string> allowedChips;

    // Restrições de busca
    int maxDepth = 4;
    int maxComponents = 16;
    bool enableQuineMcCluskey = true;
    bool enableHeuristicSearch = true;
    std::string circuitName = "CircuitoSintetizado";
    std::string circuitDescription = "Gerado automaticamente por sintese logica";
};

// Estrutura de candidato a circuito.
// Diretamente análoga ao struct Candidato de dice.cpp:
// guarda os componentes escolhidos, métricas de erro, cobertura e validade.
struct CircuitCandidate {
    struct ComponentInstance {
        std::string gateType; // "AND", "OR", "XOR", etc. ou ID de chip de usuário
        std::string instanceId; // Identificador único (ex: "and_1", "chip_ha_1")
        bool isCustomChip = false;
        std::shared_ptr<chip::ChipDefinition> chipDef = nullptr;
        std::vector<std::pair<std::string, int>> inputsConnected; // {sourceNodeId, sourcePinIndex}
        sf::Vector2f position{0.f, 0.f};
    };

    std::vector<ComponentInstance> components;
    std::vector<std::pair<std::string, int>> outputConnections; // Para cada saída externa {sourceNodeId, sourcePinIndex}

    // Métricas de erro e adequação (exatamente como em dice.cpp)
    long double fora = 1.0L; // Penalidade por saídas incompletas ou não mapeadas (0 = cobertura total)
    long double erro = 1.0L; // Taxa de bits incorretos da tabela verdade (0.0 = 100% correto)
    long double erroTotal = 1.0L; // Combinação de fora + erro
    int totalCost = 0; // Custo total (número de portas + chips)
    int depth = 0; // Níveis de propagação lógica
    bool valido = false; // Se o candidato é executável e bem formado

    // Tabela verdade alcançada por este candidato para cada saída
    std::map<std::string, std::vector<int>> achievedTable;

    // Representação em CDL e objetos SFML gerados
    std::string cdlCode;
    std::vector<std::shared_ptr<Gate>> gates;
    std::vector<Wire> wires;
    std::string summary;
};

// Informações pré-calculadas de um chip de usuário para síntese rápida
struct UserChipProfile {
    std::shared_ptr<chip::ChipDefinition> definition;
    std::string id;
    std::string name;
    size_t numInputs = 0;
    size_t numOutputs = 0;
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    // Tabela verdade de cada saída do chip: [outputIndex][inputPattern (0..2^numInputs-1)]
    std::vector<std::vector<int>> truthTables;
    bool isPureCombinational = true;
};

class CircuitSynthesizer {
public:
    CircuitSynthesizer();

    // Lê configuração no estilo dice.cpp a partir de uma string ou arquivo .ini
    static SynthesisConfig parseConfig(const std::string& text);
    static void validate(SynthesisConfig& cfg);

    // Avaliador de expressões booleanas: converte expressões (ex: "A ^ B", "A & (B | C)")
    // na tabela verdade esperada de 2^N combinações
    static std::vector<int> evaluateExpression(const std::string& expression, const std::vector<std::string>& inputNames);

    // Função de comparação para busca do melhor candidato (idêntica a melhorQue em dice.cpp):
    // 1. Prioriza validade e cobertura total (fora == 0)
    // 2. Minimiza erro de tabela verdade (erro == 0)
    // 3. Prefere circuitos mais simples (menor totalCost / número de componentes)
    // 4. Prefere menor profundidade lógica (menor atraso de propagação)
    static bool isBetter(const CircuitCandidate& a, const CircuitCandidate& b);

    // Caracteriza todos os chips criados por usuários na ChipLibrary
    std::vector<UserChipProfile> catalogUserChips(const sf::Font& font);

    // Síntese principal: recebe entradas, processamento esperado e saídas,
    // analisa o catálogo de portas padrão e chips de usuários,
    // e encontra o melhor circuito candidato gerando objetos Gate, Wire e CDL.
    CircuitCandidate synthesize(SynthesisConfig cfg, const sf::Font& font);

    // Avalia um candidato simulando sua tabela verdade e computando erro e cobertura
    // (idêntica à função avaliar() de dice.cpp)
    CircuitCandidate evaluateCandidate(CircuitCandidate candidate, const SynthesisConfig& cfg, const std::vector<UserChipProfile>& chipProfiles);

    // Formata o circuito em CDL e constrói instâncias SFML Gate/Wire com layout automático
    void buildConcreteCircuit(CircuitCandidate& candidate, const SynthesisConfig& cfg, const sf::Font& font);

    // Presets úteis para facilitar ao usuário configurar circuitos comuns
    static SynthesisConfig createPreset(const std::string& presetName);
    static std::vector<std::string> getAvailablePresets();

private:
    // Algoritmo 1: Busca recursiva combinatória (idêntico a buscarRecursivo em dice.cpp)
    void searchRecursive(
        const SynthesisConfig& cfg,
        const std::vector<UserChipProfile>& allowedChips,
        CircuitCandidate& current,
        CircuitCandidate& best,
        int currentDepth,
        int maxDepth
    );

    // Algoritmo 2: Síntese exata Quine-McCluskey com mapeamento de portas e chips de usuário
    CircuitCandidate synthesizeQuineMcCluskeyWithChips(const SynthesisConfig& cfg, const std::vector<UserChipProfile>& chipProfiles);

    // Quine-McCluskey: minimização de mintermos para Soma de Produtos (SOP)
    std::vector<std::string> minimizeQuineMcCluskey(int numInputs, const std::vector<int>& minterms, const std::vector<int>& dontCares);

    // Identifica se um subconjunto de termos ou função pode ser substituído por um chip de usuário
    bool matchSubfunctionToUserChip(
        const std::vector<int>& targetOutputTable,
        int numInputs,
        const std::vector<UserChipProfile>& chipProfiles,
        std::shared_ptr<chip::ChipDefinition>& matchedChip,
        std::vector<int>& matchedInputMapping,
        int& matchedChipOutputIndex
    );
};
