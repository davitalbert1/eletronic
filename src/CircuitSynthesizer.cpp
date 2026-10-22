#include "CircuitSynthesizer.h"
#include "LogicEntities.h"
#include "GateRegistry.h"
#include "chip/ChipLibrary.h"
#include "chip/ChipFactory.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <queue>
#include <set>
#include <iostream>

namespace {
    std::string trimStr(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    std::vector<std::string> splitStr(const std::string& text, char delimiter) {
        std::vector<std::string> res;
        std::stringstream ss(text);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            token = trimStr(token);
            if (!token.empty()) res.push_back(token);
        }
        return res;
    }

    std::string toUpperStr(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return std::toupper(c);
        });
        return s;
    }
}

CircuitSynthesizer::CircuitSynthesizer() {}

SynthesisConfig CircuitSynthesizer::parseConfig(const std::string& text) {
    SynthesisConfig cfg;
    std::stringstream ss(text);
    std::string line;

    while (std::getline(ss, line)) {
        line = trimStr(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = toUpperStr(trimStr(line.substr(0, eqPos)));
        std::string val = trimStr(line.substr(eqPos + 1));

        if (key == "INPUTS" || key == "ENTRADAS" || key == "IN") {
            cfg.inputs = splitStr(val, ',');
        } else if (key == "OUTPUTS" || key == "SAIDAS" || key == "OUT") {
            cfg.outputs = splitStr(val, ',');
        } else if (key == "GATES" || key == "PORTAS" || key == "DADOS") {
            // Em analogia direta aos 'dados' de dice.cpp
            cfg.allowedGates = splitStr(val, ',');
            for (auto& g : cfg.allowedGates) g = toUpperStr(g);
        } else if (key == "CHIPS" || key == "CUSTOM_CHIPS") {
            cfg.allowedChips = splitStr(val, ',');
        } else if (key == "NAME" || key == "NOME") {
            cfg.circuitName = val;
        } else if (key == "DESC" || key == "DESCRICAO") {
            cfg.circuitDescription = val;
        } else if (key == "MAX_DEPTH" || key == "PROFUNDIDADE") {
            try {
                cfg.maxDepth = std::stoi(val);
            } catch (...) {}
        } else if (key == "MAX_COMPONENTS" || key == "MAX_PORTAS") {
            try {
                cfg.maxComponents = std::stoi(val);
            } catch (...) {}
        } else if (key.rfind("EXPR_", 0) == 0 || key.rfind("FUNC_", 0) == 0) {
            std::string outName = key.substr(5);
            cfg.targetExpressions[outName] = val;
        } else if (key.rfind("TABLE_", 0) == 0 || key.rfind("TABELA_", 0) == 0) {
            size_t p = (key.rfind("TABLE_", 0) == 0) ? 6 : 7;
            std::string outName = key.substr(p);
            std::vector<int> bits;
            for (char c : val) {
                if (c == '0') bits.push_back(0);
                else if (c == '1') bits.push_back(1);
                else if (c == 'x' || c == 'X' || c == '?') bits.push_back(-1); // don't care
            }
            if (!bits.empty()) {
                cfg.targetTable[outName] = bits;
            }
        } else if (key == "EXPR" || key == "EXPRESSAO") {
            // Sintaxe: S = A ^ B
            auto eq = val.find('=');
            if (eq != std::string::npos) {
                std::string oName = trimStr(val.substr(0, eq));
                std::string expr = trimStr(val.substr(eq + 1));
                cfg.targetExpressions[oName] = expr;
            }
        }
    }

    validate(cfg);
    return cfg;
}

void CircuitSynthesizer::validate(SynthesisConfig& cfg) {
    if (cfg.inputs.empty()) {
        cfg.inputs = {"A", "B"};
    }
    if (cfg.outputs.empty()) {
        if (!cfg.targetExpressions.empty()) {
            for (const auto& kv : cfg.targetExpressions) {
                cfg.outputs.push_back(kv.first);
            }
        } else if (!cfg.targetTable.empty()) {
            for (const auto& kv : cfg.targetTable) cfg.outputs.push_back(kv.first);
        } else {
            cfg.outputs = {"Y"};
        }
    }

    if (cfg.allowedGates.empty()) {
        cfg.allowedGates = {"AND", "OR", "NOT", "XOR", "NAND", "NOR", "XNOR"};
    }

    size_t numComb = (size_t)1 << cfg.inputs.size();

    // Converte targetExpressions para targetTable se targetTable ainda não estiver preenchida
    for (const auto& outName : cfg.outputs) {
        if (cfg.targetTable.find(outName) == cfg.targetTable.end()) {
            auto itExpr = cfg.targetExpressions.find(outName);
            if (itExpr != cfg.targetExpressions.end()) {
                cfg.targetTable[outName] = evaluateExpression(itExpr->second, cfg.inputs);
            } else {
                // Tabela padrão vazia/zeros
                cfg.targetTable[outName] = std::vector<int>(numComb, 0);
            }
        } else {
            // Valida tamanho da tabela
            auto& tbl = cfg.targetTable[outName];
            if (tbl.size() < numComb) tbl.resize(numComb, 0);
        }
    }
}

namespace {
    // Avaliador de expressões booleanas com precedência de operadores
    // NOT (~) > AND (&) > XOR (^) > OR (|)
    struct ExprParser {
        std::string src;
        size_t pos = 0;
        const std::map<std::string, bool>& varValues;

        ExprParser(const std::string& s, const std::map<std::string, bool>& vars)
            : src(s), pos(0), varValues(vars) {}

        void skipWhitespace() {
            while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\r' || src[pos] == '\n')) {
                pos++;
            }
        }

        bool parse() {
            bool val = parseOr();
            return val;
        }

        bool parseOr() {
            bool left = parseXor();
            skipWhitespace();
            while (pos < src.size() && (src[pos] == '|' || (pos + 1 < src.size() && src.substr(pos, 2) == "||"))) {
                if (src[pos] == '|' && pos + 1 < src.size() && src[pos+1] == '|') pos += 2;
                else pos++;
                bool right = parseXor();
                left = left || right;
                skipWhitespace();
            }
            return left;
        }

        bool parseXor() {
            bool left = parseAnd();
            skipWhitespace();
            while (pos < src.size() && (src[pos] == '^')) {
                pos++;
                bool right = parseAnd();
                left = left != right;
                skipWhitespace();
            }
            return left;
        }

        bool parseAnd() {
            bool left = parseNot();
            skipWhitespace();
            while (pos < src.size() && (src[pos] == '&' || (pos + 1 < src.size() && src.substr(pos, 2) == "&&"))) {
                if (src[pos] == '&' && pos + 1 < src.size() && src[pos+1] == '&') pos += 2;
                else pos++;
                bool right = parseNot();
                left = left && right;
                skipWhitespace();
            }
            return left;
        }

        bool parseNot() {
            skipWhitespace();
            if (pos < src.size() && (src[pos] == '!' || src[pos] == '~')) {
                pos++;
                return !parseNot();
            }
            return parsePrimary();
        }

        bool parsePrimary() {
            skipWhitespace();
            if (pos >= src.size()) return false;

            if (src[pos] == '(') {
                pos++;
                bool val = parseOr();
                skipWhitespace();
                if (pos < src.size() && src[pos] == ')') pos++;
                return val;
            }

            if (src[pos] == '0') {
                pos++;
                return false;
            }
            if (src[pos] == '1') {
                pos++;
                return true;
            }

            // Nome de variável
            size_t start = pos;
            while (pos < src.size() && (std::isalnum((unsigned char)src[pos]) || src[pos] == '_')) {
                pos++;
            }
            std::string varName = src.substr(start, pos - start);
            auto it = varValues.find(varName);
            if (it != varValues.end()) return it->second;
            return false;
        }
    };
}

std::vector<int> CircuitSynthesizer::evaluateExpression(const std::string& expression, const std::vector<std::string>& inputNames) {
    size_t n = inputNames.size();
    size_t totalComb = (size_t)1 << n;
    std::vector<int> result(totalComb, 0);

    for (size_t comb = 0; comb < totalComb; ++comb) {
        std::map<std::string, bool> varMap;
        for (size_t i = 0; i < n; ++i) {
            // Bit mais significativo ou padrão index i (comb >> (n - 1 - i)) & 1
            bool val = ((comb >> (n - 1 - i)) & 1) != 0;
            varMap[inputNames[i]] = val;
        }
        ExprParser parser(expression, varMap);
        result[comb] = parser.parse() ? 1 : 0;
    }

    return result;
}

bool CircuitSynthesizer::isBetter(const CircuitCandidate& a, const CircuitCandidate& b) {
    if (!b.valido) return true;
    const long double eps = 1e-12L;

    // 1. Prioriza minimizar bits fora / não-cobertos
    if (a.fora < b.fora - eps) return true;
    if (a.fora > b.fora + eps) return false;

    // 2. Minimiza erro total de tabela verdade
    if (a.erro < b.erro - eps) return true;
    if (a.erro > b.erro + eps) return false;

    // 3. Preferir circuitos com menos componentes (menor complexidade)
    if (a.totalCost != b.totalCost) return a.totalCost < b.totalCost;

    // 4. Preferir menor profundidade lógica (menor atraso)
    if (a.depth != b.depth) return a.depth < b.depth;

    return a.components.size() < b.components.size();
}

std::vector<UserChipProfile> CircuitSynthesizer::catalogUserChips(const sf::Font& font) {
    std::vector<UserChipProfile> profiles;
    auto allChips = chip::ChipLibrary::instance().all();

    for (const auto& def : allChips) {
        if (!def || !def->isValid()) continue;

        UserChipProfile prof;
        prof.definition = def;
        prof.id = def->id;
        prof.name = def->name;
        prof.numInputs = def->inputs.size();
        prof.numOutputs = def->outputs.size();

        for (const auto& inp : def->inputs) prof.inputNames.push_back(inp.name);
        for (const auto& outp : def->outputs) prof.outputNames.push_back(outp.name);

        // Se tiver mais de 8 entradas, pular catalogação exaustiva de tabela verdade para economizar CPU
        if (prof.numInputs > 8) {
            profiles.push_back(prof);
            continue;
        }

        size_t totalComb = (size_t)1 << prof.numInputs;
        prof.truthTables.assign(prof.numOutputs, std::vector<int>(totalComb, 0));

        // Cria instância para simular a tabela verdade do chip
        auto inst = chip::createChipInstance(def, sf::Vector2f(0.f, 0.f), font);
        if (inst) {
            for (size_t comb = 0; comb < totalComb; ++comb) {
                // Alimenta os pinos de entrada
                for (size_t i = 0; i < prof.numInputs; ++i) {
                    bool val = ((comb >> (prof.numInputs - 1 - i)) & 1) != 0;
                    if (i < inst->inputs.size()) inst->inputs[i].state = val;
                }
                // Simula 2 ciclos para propagar por circuitos de múltiplos estágios
                inst->updateLogic();
                inst->updateLogic();

                for (size_t o = 0; o < prof.numOutputs; ++o) {
                    if (o < inst->outputs.size()) {
                        prof.truthTables[o][comb] = inst->outputs[o].state ? 1 : 0;
                    }
                }
            }
        }

        profiles.push_back(prof);
    }

    return profiles;
}

// Algoritmo Quine-McCluskey para simplificação SOP exata
std::vector<std::string> CircuitSynthesizer::minimizeQuineMcCluskey(int numInputs, const std::vector<int>& minterms, const std::vector<int>& dontCares) {
    if (minterms.empty()) return {};

    std::set<int> allTerms(minterms.begin(), minterms.end());
    allTerms.insert(dontCares.begin(), dontCares.end());

    struct Term {
        std::string bits; // ex: "01-1"
        std::set<int> coveredMinterms;
        bool combined = false;
    };

    auto toBinaryString = [](int val, int len) {
        std::string s(len, '0');
        for (int i = 0; i < len; ++i) {
            if ((val >> (len - 1 - i)) & 1) s[i] = '1';
        }
        return s;
    };

    std::vector<Term> currentGroups;
    for (int m : allTerms) {
        Term t;
        t.bits = toBinaryString(m, numInputs);
        if (std::find(minterms.begin(), minterms.end(), m) != minterms.end()) {
            t.coveredMinterms.insert(m);
        }
        currentGroups.push_back(t);
    }

    std::vector<Term> primeImplicants;

    while (!currentGroups.empty()) {
        std::vector<Term> nextGroups;
        std::set<std::string> seen;

        for (size_t i = 0; i < currentGroups.size(); ++i) {
            for (size_t j = i + 1; j < currentGroups.size(); ++j) {
                int diffCount = 0;
                int diffPos = -1;
                for (int k = 0; k < numInputs; ++k) {
                    if (currentGroups[i].bits[k] != currentGroups[j].bits[k]) {
                        diffCount++;
                        diffPos = k;
                    }
                }
                if (diffCount == 1) {
                    currentGroups[i].combined = true;
                    currentGroups[j].combined = true;

                    Term newT;
                    newT.bits = currentGroups[i].bits;
                    newT.bits[diffPos] = '-';
                    newT.coveredMinterms = currentGroups[i].coveredMinterms;
                    newT.coveredMinterms.insert(currentGroups[j].coveredMinterms.begin(), currentGroups[j].coveredMinterms.end());

                    if (seen.find(newT.bits) == seen.end()) {
                        seen.insert(newT.bits);
                        nextGroups.push_back(newT);
                    }
                }
            }
        }

        for (const auto& t : currentGroups) {
            if (!t.combined && !t.coveredMinterms.empty()) primeImplicants.push_back(t);
        }

        currentGroups = nextGroups;
    }

    // Cobertura essencial dos mintermos
    std::set<int> uncovered(minterms.begin(), minterms.end());
    std::vector<std::string> selectedTerms;

    while (!uncovered.empty()) {
        int bestIdx = -1;
        size_t maxCover = 0;
        for (size_t i = 0; i < primeImplicants.size(); ++i) {
            size_t count = 0;
            for (int m : primeImplicants[i].coveredMinterms) {
                if (uncovered.count(m)) count++;
            }
            if (count > maxCover) {
                maxCover = count;
                bestIdx = (int)i;
            }
        }

        if (bestIdx == -1 || maxCover == 0) break;

        selectedTerms.push_back(primeImplicants[bestIdx].bits);
        for (int m : primeImplicants[bestIdx].coveredMinterms) uncovered.erase(m);
    }

    return selectedTerms;
}

bool CircuitSynthesizer::matchSubfunctionToUserChip(
    const std::vector<int>& targetOutputTable,
    int numInputs,
    const std::vector<UserChipProfile>& chipProfiles,
    std::shared_ptr<chip::ChipDefinition>& matchedChip,
    std::vector<int>& matchedInputMapping,
    int& matchedChipOutputIndex
) {
    // Procura se algum chip de usuário gera exatamente a tabela verdade de saída com permutação de pinos
    for (const auto& prof : chipProfiles) {
        if (!prof.isPureCombinational || prof.truthTables.empty()) continue;
        if (prof.numInputs > (size_t)numInputs) continue;

        size_t targetComb = targetOutputTable.size();

        for (size_t outIdx = 0; outIdx < prof.truthTables.size(); ++outIdx) {
            const auto& chipTable = prof.truthTables[outIdx];

            // Tenta mapear as entradas do chip para subconjuntos das entradas alvo
            // Se prof.numInputs == 2 e numInputs == 2: permutações (0,1) e (1,0)
            if (prof.numInputs == 2 && numInputs >= 2) {
                for (int inA = 0; inA < numInputs; ++inA) {
                    for (int inB = 0; inB < numInputs; ++inB) {
                        if (inA == inB) continue;

                        bool match = true;
                        for (size_t c = 0; c < targetComb; ++c) {
                            if (targetOutputTable[c] == -1) continue; // don't care

                            int bitA = (c >> (numInputs - 1 - inA)) & 1;
                            int bitB = (c >> (numInputs - 1 - inB)) & 1;
                            int chipComb = (bitA << 1) | bitB;

                            if (chipTable[chipComb] != targetOutputTable[c]) {
                                match = false;
                                break;
                            }
                        }

                        if (match) {
                            matchedChip = prof.definition;
                            matchedInputMapping = {inA, inB};
                            matchedChipOutputIndex = (int)outIdx;
                            return true;
                        }
                    }
                }
            } else if (prof.numInputs == 1 && numInputs >= 1) {
                for (int inA = 0; inA < numInputs; ++inA) {
                    bool match = true;
                    for (size_t c = 0; c < targetComb; ++c) {
                        if (targetOutputTable[c] == -1) continue;
                        int bitA = (c >> (numInputs - 1 - inA)) & 1;
                        if (chipTable[bitA] != targetOutputTable[c]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        matchedChip = prof.definition;
                        matchedInputMapping = {inA};
                        matchedChipOutputIndex = (int)outIdx;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

CircuitCandidate CircuitSynthesizer::synthesizeQuineMcCluskeyWithChips(
    const SynthesisConfig& cfg,
    const std::vector<UserChipProfile>& chipProfiles
) {
    CircuitCandidate cand;
    cand.valido = true;
    int numIn = (int)cfg.inputs.size();
    int componentCounter = 1;

    // Para cada saída esperada:
    for (size_t outIdx = 0; outIdx < cfg.outputs.size(); ++outIdx) {
        std::string outName = cfg.outputs[outIdx];
        const auto& table = cfg.targetTable.at(outName);

        // 1) Verifica se algum chip de usuário resolve diretamente esta saída
        std::shared_ptr<chip::ChipDefinition> matchedChip;
        std::vector<int> mappedInputs;
        int chipOutIdx = 0;

        if (matchSubfunctionToUserChip(table, numIn, chipProfiles, matchedChip, mappedInputs, chipOutIdx)) {
            CircuitCandidate::ComponentInstance chipInst;
            chipInst.isCustomChip = true;
            chipInst.gateType = matchedChip->id;
            chipInst.instanceId = "chip_" + matchedChip->id + "_" + std::to_string(componentCounter++);
            chipInst.chipDef = matchedChip;

            for (int inPinIdx : mappedInputs) {
                chipInst.inputsConnected.push_back({"in_" + cfg.inputs[inPinIdx], 0});
            }

            cand.components.push_back(chipInst);
            cand.outputConnections.push_back({chipInst.instanceId, chipOutIdx});
            continue;
        }

        // 2) Caso não haja chip direto, aplica Quine-McCluskey para gerar AND / OR / NOT
        std::vector<int> minterms;
        std::vector<int> dontCares;
        for (size_t i = 0; i < table.size(); ++i) {
            if (table[i] == 1) minterms.push_back((int)i);
            else if (table[i] == -1) dontCares.push_back((int)i);
        }

        // Saída constante 0
        if (minterms.empty()) continue;
        if (minterms.size() + dontCares.size() == table.size()) {
            // Saída constante 1
            CircuitCandidate::ComponentInstance notGate;
            notGate.gateType = "NOT";
            notGate.instanceId = "not_const_" + std::to_string(componentCounter++);
            notGate.inputsConnected.push_back({"in_" + cfg.inputs[0], 0});
            cand.components.push_back(notGate);

            CircuitCandidate::ComponentInstance orConst;
            orConst.gateType = "OR";
            orConst.instanceId = "or_const_" + std::to_string(componentCounter++);
            orConst.inputsConnected.push_back({"in_" + cfg.inputs[0], 0});
            orConst.inputsConnected.push_back({notGate.instanceId, 0});
            cand.components.push_back(orConst);
            cand.outputConnections.push_back({orConst.instanceId, 0});
            continue;
        }

        std::vector<std::string> terms = minimizeQuineMcCluskey(numIn, minterms, dontCares);

        std::vector<std::string> termOutputGates;

        // Cria gates NOT compartilhados se necessário
        std::map<int, std::string> notGatesForInput;
        auto getNotGate = [&](int inputIndex) {
            if (notGatesForInput.count(inputIndex)) return notGatesForInput[inputIndex];
            CircuitCandidate::ComponentInstance notInst;
            notInst.gateType = "NOT";
            notInst.instanceId = "not_" + cfg.inputs[inputIndex] + "_" + std::to_string(componentCounter++);
            notInst.inputsConnected.push_back({"in_" + cfg.inputs[inputIndex], 0});
            cand.components.push_back(notInst);
            notGatesForInput[inputIndex] = notInst.instanceId;
            return notInst.instanceId;
        };

        for (const std::string& term : terms) {
            std::vector<std::pair<std::string, int>> literals;
            for (int k = 0; k < numIn; ++k) {
                if (term[k] == '1') {
                    literals.push_back({"in_" + cfg.inputs[k], 0});
                } else if (term[k] == '0') {
                    std::string notId = getNotGate(k);
                    literals.push_back({notId, 0});
                }
            }

            if (literals.empty()) continue;

            if (literals.size() == 1) {
                termOutputGates.push_back(literals[0].first);
            } else {
                // Árvore de ANDs de 2 entradas
                std::string prev = literals[0].first;
                int prevPin = literals[0].second;
                for (size_t l = 1; l < literals.size(); ++l) {
                    CircuitCandidate::ComponentInstance andInst;
                    andInst.gateType = "AND";
                    andInst.instanceId = "and_" + std::to_string(componentCounter++);
                    andInst.inputsConnected.push_back({prev, prevPin});
                    andInst.inputsConnected.push_back(literals[l]);
                    cand.components.push_back(andInst);
                    prev = andInst.instanceId;
                    prevPin = 0;
                }
                termOutputGates.push_back(prev);
            }
        }

        // Combina termos com árvore de ORs
        if (termOutputGates.empty()) {
            // Nenhuma saída ativada
        } else if (termOutputGates.size() == 1) {
            cand.outputConnections.push_back({termOutputGates[0], 0});
        } else {
            std::string prevOr = termOutputGates[0];
            for (size_t t = 1; t < termOutputGates.size(); ++t) {
                CircuitCandidate::ComponentInstance orInst;
                orInst.gateType = "OR";
                orInst.instanceId = "or_" + std::to_string(componentCounter++);
                orInst.inputsConnected.push_back({prevOr, 0});
                orInst.inputsConnected.push_back({termOutputGates[t], 0});
                cand.components.push_back(orInst);
                prevOr = orInst.instanceId;
            }
            cand.outputConnections.push_back({prevOr, 0});
        }
    }

    cand.totalCost = (int)cand.components.size();
    cand.fora = 0.0L;
    cand.erro = 0.0L;
    cand.erroTotal = 0.0L;
    return cand;
}

CircuitCandidate CircuitSynthesizer::evaluateCandidate(
    CircuitCandidate candidate,
    const SynthesisConfig& cfg,
    const std::vector<UserChipProfile>& chipProfiles
) {
    int numIn = (int)cfg.inputs.size();
    size_t totalComb = (size_t)1 << numIn;

    if (candidate.outputConnections.size() < cfg.outputs.size()) {
        candidate.fora = 1.0L;
        candidate.erro = 1.0L;
        candidate.erroTotal = 2.0L;
        candidate.valido = false;
        return candidate;
    }

    long double bitErrors = 0.0L;
    long double totalTargetBits = 0.0L;

    for (size_t o = 0; o < cfg.outputs.size(); ++o) {
        std::string outName = cfg.outputs[o];
        const auto& target = cfg.targetTable.at(outName);
        std::vector<int> achieved(totalComb, 0);

        for (size_t comb = 0; comb < totalComb; ++comb) {
            std::map<std::string, int> nodeSignals;
            for (int i = 0; i < numIn; ++i) {
                nodeSignals["in_" + cfg.inputs[i]] = (comb >> (numIn - 1 - i)) & 1;
            }

            // Simula componentes topologicamente
            for (const auto& comp : candidate.components) {
                std::vector<int> inVals;
                for (const auto& src : comp.inputsConnected) {
                    auto it = nodeSignals.find(src.first);
                    inVals.push_back(it != nodeSignals.end() ? it->second : 0);
                }

                int outVal = 0;
                if (!comp.isCustomChip) {
                    if (comp.gateType == "AND") outVal = (inVals.size() >= 2 && inVals[0] && inVals[1]) ? 1 : 0;
                    else if (comp.gateType == "OR") outVal = (inVals.size() >= 2 && (inVals[0] || inVals[1])) ? 1 : 0;
                    else if (comp.gateType == "NOT") outVal = (inVals.size() >= 1 && !inVals[0]) ? 1 : 0;
                    else if (comp.gateType == "XOR") outVal = (inVals.size() >= 2 && (inVals[0] != inVals[1])) ? 1 : 0;
                    else if (comp.gateType == "NAND") outVal = (inVals.size() >= 2 && !(inVals[0] && inVals[1])) ? 1 : 0;
                    else if (comp.gateType == "NOR") outVal = (inVals.size() >= 2 && !(inVals[0] || inVals[1])) ? 1 : 0;
                    else if (comp.gateType == "XNOR") outVal = (inVals.size() >= 2 && (inVals[0] == inVals[1])) ? 1 : 0;
                } else {
                    // Busca na tabela pré-calculada do chip
                    for (const auto& prof : chipProfiles) {
                        if (prof.id == comp.gateType && !prof.truthTables.empty()) {
                            int chipComb = 0;
                            for (size_t p = 0; p < inVals.size() && p < prof.numInputs; ++p) {
                                chipComb = (chipComb << 1) | (inVals[p] ? 1 : 0);
                            }
                            if (chipComb < (int)prof.truthTables[0].size()) {
                                outVal = prof.truthTables[0][chipComb];
                            }
                            break;
                        }
                    }
                }
                nodeSignals[comp.instanceId] = outVal;
            }

            int finalOut = 0;
            if (o < candidate.outputConnections.size()) {
                auto it = nodeSignals.find(candidate.outputConnections[o].first);
                if (it != nodeSignals.end()) finalOut = it->second;
            }
            achieved[comb] = finalOut;

            if (target[comb] != -1) {
                totalTargetBits += 1.0L;
                if (finalOut != target[comb]) {
                    bitErrors += 1.0L;
                }
            }
        }

        candidate.achievedTable[outName] = achieved;
    }

    candidate.fora = 0.0L;
    candidate.erro = (totalTargetBits > 0) ? (bitErrors / totalTargetBits) : 0.0L;
    candidate.erroTotal = candidate.fora + candidate.erro;
    candidate.totalCost = (int)candidate.components.size();
    candidate.valido = true;
    return candidate;
}

void CircuitSynthesizer::searchRecursive(
    const SynthesisConfig& cfg,
    const std::vector<UserChipProfile>& allowedChips,
    CircuitCandidate& current,
    CircuitCandidate& best,
    int currentDepth,
    int maxDepth
) {
    if (currentDepth >= maxDepth || (int)current.components.size() >= cfg.maxComponents) {
        return;
    }

    // Avalia candidato atual se tiver conexões com as saídas
    if (!current.components.empty()) {
        current.outputConnections.clear();
        // Conecta as saídas com os últimos componentes
        for (size_t o = 0; o < cfg.outputs.size(); ++o) {
            size_t idx = current.components.size() > o ? (current.components.size() - 1 - o) : 0;
            current.outputConnections.push_back({current.components[idx].instanceId, 0});
        }

        CircuitCandidate evaluated = evaluateCandidate(current, cfg, allowedChips);
        if (isBetter(evaluated, best)) {
            best = evaluated;
            if (best.erroTotal == 0.0L && best.fora == 0.0L) {
                return; // Encontrou circuito perfeito!
            }
        }
    }

    // Pool de nós disponíveis para conectar como entradas do próximo componente
    std::vector<std::string> availableSources;
    for (const auto& inName : cfg.inputs) {
        availableSources.push_back("in_" + inName);
    }
    for (const auto& comp : current.components) {
        availableSources.push_back(comp.instanceId);
    }

    // Tenta portas primitivas
    for (const auto& gateName : cfg.allowedGates) {
        int arity = (gateName == "NOT") ? 1 : 2;

        if (arity == 1) {
            for (const auto& src : availableSources) {
                CircuitCandidate::ComponentInstance inst;
                inst.gateType = gateName;
                inst.instanceId = gateName + "_" + std::to_string(current.components.size() + 1);
                inst.inputsConnected.push_back({src, 0});

                current.components.push_back(inst);
                searchRecursive(cfg, allowedChips, current, best, currentDepth + 1, maxDepth);
                current.components.pop_back();

                if (best.erroTotal == 0.0L) return;
            }
        } else if (arity == 2) {
            for (size_t s1 = 0; s1 < availableSources.size(); ++s1) {
                for (size_t s2 = s1; s2 < availableSources.size(); ++s2) {
                    CircuitCandidate::ComponentInstance inst;
                    inst.gateType = gateName;
                    inst.instanceId = gateName + "_" + std::to_string(current.components.size() + 1);
                    inst.inputsConnected.push_back({availableSources[s1], 0});
                    inst.inputsConnected.push_back({availableSources[s2], 0});

                    current.components.push_back(inst);
                    searchRecursive(cfg, allowedChips, current, best, currentDepth + 1, maxDepth);
                    current.components.pop_back();

                    if (best.erroTotal == 0.0L) return;
                }
            }
        }
    }

    // Tenta chips de usuário permitidos
    for (const auto& chipProf : allowedChips) {
        if (chipProf.numInputs == 2) {
            for (size_t s1 = 0; s1 < availableSources.size(); ++s1) {
                for (size_t s2 = 0; s2 < availableSources.size(); ++s2) {
                    if (s1 == s2) continue;
                    CircuitCandidate::ComponentInstance inst;
                    inst.isCustomChip = true;
                    inst.gateType = chipProf.id;
                    inst.chipDef = chipProf.definition;
                    inst.instanceId = "chip_" + chipProf.id + "_" + std::to_string(current.components.size() + 1);
                    inst.inputsConnected.push_back({availableSources[s1], 0});
                    inst.inputsConnected.push_back({availableSources[s2], 0});

                    current.components.push_back(inst);
                    searchRecursive(cfg, allowedChips, current, best, currentDepth + 1, maxDepth);
                    current.components.pop_back();

                    if (best.erroTotal == 0.0L) return;
                }
            }
        }
    }
}

CircuitCandidate CircuitSynthesizer::synthesize(SynthesisConfig cfg, const sf::Font& font) {
    validate(cfg);
    auto allUserChips = catalogUserChips(font);

    // Filtra apenas os chips permitidos na configuração se especificado
    std::vector<UserChipProfile> allowedChips;
    if (!cfg.allowedChips.empty()) {
        for (const auto& prof : allUserChips) {
            if (std::find(cfg.allowedChips.begin(), cfg.allowedChips.end(), prof.id) != cfg.allowedChips.end() ||
                std::find(cfg.allowedChips.begin(), cfg.allowedChips.end(), prof.name) != cfg.allowedChips.end()) {
                allowedChips.push_back(prof);
            }
        }
    } else {
        allowedChips = allUserChips;
    }

    CircuitCandidate bestCandidate;

    // 1) Algoritmo de Busca Heurística Combinatória (inspirado diretamente em dice.cpp)
    // Tenta encontrar a menor topologia possível para funções pequenas
    if (cfg.enableHeuristicSearch && cfg.inputs.size() <= 4) {
        CircuitCandidate current;
        searchRecursive(cfg, allowedChips, current, bestCandidate, 0, cfg.maxDepth);
    }

    // 2) Se a busca combinatória não encontrou solução perfeita (erro > 0)
    // ou se Quine-McCluskey estiver ativo, utiliza síntese exata Quine-McCluskey com mapeamento de chips
    if ((!bestCandidate.valido || bestCandidate.erro > 1e-9L) && cfg.enableQuineMcCluskey) {
        CircuitCandidate qmCandidate = synthesizeQuineMcCluskeyWithChips(cfg, allowedChips);
        qmCandidate = evaluateCandidate(qmCandidate, cfg, allowedChips);
        if (isBetter(qmCandidate, bestCandidate)) {
            bestCandidate = qmCandidate;
        }
    }

    // 3) Constrói os objetos visuais Gate, Wire e gera código CDL
    buildConcreteCircuit(bestCandidate, cfg, font);

    return bestCandidate;
}

void CircuitSynthesizer::buildConcreteCircuit(
    CircuitCandidate& candidate,
    const SynthesisConfig& cfg,
    const sf::Font& font
) {
    candidate.gates.clear();
    candidate.wires.clear();

    std::ostringstream cdl;
    cdl << "circuit " << (cfg.circuitName.empty() ? "CircuitoSintetizado" : cfg.circuitName) << " {\n";
    cdl << "  # Circuito gerado automaticamente pelo CircuitSynthesizer\n";
    cdl << "  # Precisao: " << (100.0L - 100.0L * candidate.erro) << "%\n";
    cdl << "  # Componentes: " << candidate.components.size() << "\n\n";

    std::map<std::string, std::shared_ptr<Gate>> gateMap;

    // 1. Pinos de entrada
    float startX = 100.0f;
    float startY = 120.0f;
    float rowHeight = 90.0f;

    for (size_t i = 0; i < cfg.inputs.size(); ++i) {
        std::string inName = cfg.inputs[i];
        std::string nodeId = "in_" + inName;
        sf::Vector2f pos(startX, startY + i * rowHeight);

        auto inGate = GateRegistry::instance().createGate(GateType::INPUT_PIN, pos, font, inName);
        gateMap[nodeId] = inGate;
        candidate.gates.push_back(inGate);

        cdl << "  input " << inName << " at (" << pos.x << ", " << pos.y << ")\n";
    }

    // 2. Componentes internos
    // Layout em colunas baseado na dependência
    std::map<std::string, int> columnMap;
    for (const auto& inName : cfg.inputs) {
        columnMap["in_" + inName] = 0;
    }

    for (const auto& comp : candidate.components) {
        int maxCol = 0;
        for (const auto& src : comp.inputsConnected) {
            maxCol = std::max(maxCol, columnMap[src.first]);
        }
        columnMap[comp.instanceId] = maxCol + 1;
    }

    std::map<int, std::vector<std::string>> componentsByCol;
    for (const auto& comp : candidate.components) {
        int col = columnMap[comp.instanceId];
        componentsByCol[col].push_back(comp.instanceId);
    }

    float colWidth = 180.0f;
    for (auto& comp : candidate.components) {
        int col = columnMap[comp.instanceId];
        const auto& colList = componentsByCol[col];
        auto it = std::find(colList.begin(), colList.end(), comp.instanceId);
        int row = (it != colList.end()) ? (int)std::distance(colList.begin(), it) : 0;

        sf::Vector2f pos(startX + col * colWidth, startY + row * rowHeight);
        comp.position = pos;

        std::shared_ptr<Gate> gate = nullptr;
        if (comp.isCustomChip) {
            gate = chip::createChipInstance(comp.chipDef, pos, font);
            if (gate) {
                cdl << "  chip " << comp.chipDef->id << " " << comp.instanceId << " at (" << pos.x << ", " << pos.y << ")\n";
            }
        } else {
            gate = GateRegistry::instance().createGate(comp.gateType, pos, font);
            if (gate) {
                cdl << "  gate " << comp.gateType << " " << comp.instanceId << " at (" << pos.x << ", " << pos.y << ")\n";
            }
        }

        if (gate) {
            gateMap[comp.instanceId] = gate;
            candidate.gates.push_back(gate);
        }
    }

    // 3. Pinos de saída
    int maxCol = 1;
    for (const auto& kv : columnMap) {
        maxCol = std::max(maxCol, kv.second);
    }

    float outX = startX + (maxCol + 1) * colWidth;
    for (size_t o = 0; o < cfg.outputs.size(); ++o) {
        std::string outName = cfg.outputs[o];
        sf::Vector2f pos(outX, startY + o * rowHeight);

        auto outGate = GateRegistry::instance().createGate(GateType::OUTPUT_PIN, pos, font, outName);
        std::string outId = "out_" + outName;
        gateMap[outId] = outGate;
        candidate.gates.push_back(outGate);

        cdl << "  output " << outName << " at (" << pos.x << ", " << pos.y << ")\n";
    }

    cdl << "\n";

    // 4. Criação dos fios e instruções CDL
    for (const auto& comp : candidate.components) {
        auto destGateIt = gateMap.find(comp.instanceId);
        if (destGateIt == gateMap.end()) continue;
        auto destGate = destGateIt->second;

        for (size_t pin = 0; pin < comp.inputsConnected.size(); ++pin) {
            std::string srcId = comp.inputsConnected[pin].first;
            int srcPinIdx = comp.inputsConnected[pin].second;

            auto srcGateIt = gateMap.find(srcId);
            if (srcGateIt == gateMap.end()) continue;
            auto srcGate = srcGateIt->second;

            if (srcPinIdx < (int)srcGate->outputs.size() && pin < destGate->inputs.size()) {
                candidate.wires.emplace_back(&srcGate->outputs[srcPinIdx], &destGate->inputs[pin]);
                cdl << "  wire " << srcId << ".out[" << srcPinIdx << "] -> "
                    << comp.instanceId << ".in[" << pin << "]\n";
            }
        }
    }

    for (size_t o = 0; o < candidate.outputConnections.size() && o < cfg.outputs.size(); ++o) {
        std::string srcId = candidate.outputConnections[o].first;
        int srcPinIdx = candidate.outputConnections[o].second;
        std::string outId = "out_" + cfg.outputs[o];

        auto srcGateIt = gateMap.find(srcId);
        auto destGateIt = gateMap.find(outId);
        if (srcGateIt != gateMap.end() && destGateIt != gateMap.end()) {
            auto srcGate = srcGateIt->second;
            auto destGate = destGateIt->second;
            if (srcPinIdx < (int)srcGate->outputs.size() && !destGate->inputs.empty()) {
                candidate.wires.emplace_back(&srcGate->outputs[srcPinIdx], &destGate->inputs[0]);
                cdl << "  wire " << srcId << ".out[" << srcPinIdx << "] -> "
                    << outId << ".in[0]\n";
            }
        }
    }

    cdl << "}\n";
    candidate.cdlCode = cdl.str();

    // Sumário detalhado (inspirado na saída de dice.cpp)
    std::ostringstream sum;
    sum << "=== Sintese de Circuito Concluida ===\n";
    sum << "Circuito: " << cfg.circuitName << "\n";
    sum << "Componentes gerados: " << candidate.components.size() << "\n";
    sum << "Conexoes (fios): " << candidate.wires.size() << "\n";
    sum << "Erro de tabela verdade: " << (double)(candidate.erro * 100.0L) << "%\n";
    sum << "Fora do intervalo/especificacao: " << (double)(candidate.fora * 100.0L) << "%\n";
    if (candidate.erro == 0.0L && candidate.fora == 0.0L) {
        sum << "Cobertura: Perfeita (100% compativel com entradas e saidas solicitadas)\n";
    } else {
        sum << "Cobertura: Aproximada (com erro minimo encontrado)\n";
    }
    candidate.summary = sum.str();
}

std::vector<std::string> CircuitSynthesizer::getAvailablePresets() {
    return {
        "Meio Somador (Half Adder)",
        "Somador Completo (Full Adder)",
        "Multiplexador 2:1 (MUX)",
        "Demultiplexador 1:2 (DEMUX)",
        "Comparador de 1 bit (A == B, A > B, A < B)",
        "Porta XOR usando apenas NAND"
    };
}

SynthesisConfig CircuitSynthesizer::createPreset(const std::string& presetName) {
    SynthesisConfig cfg;

    if (presetName == "Meio Somador (Half Adder)") {
        cfg.circuitName = "HalfAdder";
        cfg.circuitDescription = "Soma 2 bits: S = A ^ B, Cout = A & B";
        cfg.inputs = {"A", "B"};
        cfg.outputs = {"S", "Cout"};
        cfg.targetExpressions["S"] = "A ^ B";
        cfg.targetExpressions["Cout"] = "A & B";
        cfg.allowedGates = {"AND", "XOR", "OR", "NOT"};
    } else if (presetName == "Somador Completo (Full Adder)") {
        cfg.circuitName = "FullAdder";
        cfg.circuitDescription = "Soma 3 bits: A, B, Cin -> S, Cout";
        cfg.inputs = {"A", "B", "Cin"};
        cfg.outputs = {"S", "Cout"};
        cfg.targetExpressions["S"] = "A ^ B ^ Cin";
        cfg.targetExpressions["Cout"] = "(A & B) | (B & Cin) | (A & Cin)";
        cfg.allowedGates = {"AND", "OR", "XOR", "NOT"};
    } else if (presetName == "Multiplexador 2:1 (MUX)") {
        cfg.circuitName = "MUX_2to1";
        cfg.circuitDescription = "Seleciona entre D0 e D1 baseado em Sel: Y = (~Sel & D0) | (Sel & D1)";
        cfg.inputs = {"Sel", "D0", "D1"};
        cfg.outputs = {"Y"};
        cfg.targetExpressions["Y"] = "(!Sel & D0) | (Sel & D1)";
        cfg.allowedGates = {"AND", "OR", "NOT"};
    } else if (presetName == "Demultiplexador 1:2 (DEMUX)") {
        cfg.circuitName = "DEMUX_1to2";
        cfg.circuitDescription = "Direciona entrada In para Y0 ou Y1 conforme Sel";
        cfg.inputs = {"Sel", "In"};
        cfg.outputs = {"Y0", "Y1"};
        cfg.targetExpressions["Y0"] = "!Sel & In";
        cfg.targetExpressions["Y1"] = "Sel & In";
        cfg.allowedGates = {"AND", "NOT"};
    } else if (presetName == "Comparador de 1 bit (A == B, A > B, A < B)") {
        cfg.circuitName = "Comparator1Bit";
        cfg.circuitDescription = "Compara A e B: Eq, Gt, Lt";
        cfg.inputs = {"A", "B"};
        cfg.outputs = {"Eq", "Gt", "Lt"};
        cfg.targetExpressions["Eq"] = "!(A ^ B)";
        cfg.targetExpressions["Gt"] = "A & !B";
        cfg.targetExpressions["Lt"] = "!A & B";
        cfg.allowedGates = {"AND", "NOT", "XNOR", "OR"};
    } else if (presetName == "Porta XOR usando apenas NAND") {
        cfg.circuitName = "XOR_from_NAND";
        cfg.circuitDescription = "XOR sintetizado exclusivamente a partir de portas NAND";
        cfg.inputs = {"A", "B"};
        cfg.outputs = {"Y"};
        cfg.targetExpressions["Y"] = "A ^ B";
        cfg.allowedGates = {"NAND"};
        cfg.maxDepth = 4;
        cfg.maxComponents = 5;
    }

    validate(cfg);
    return cfg;
}
