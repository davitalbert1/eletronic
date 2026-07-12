#include "chip/ChipFactory.h"
#include "chip/ChipLibrary.h"
#include "LogicEntities.h"
#include <fstream>

namespace chip {
    std::shared_ptr<CustomChip> createChipInstance(const std::string& chipId,
                                                sf::Vector2f pos,
                                                const sf::Font& font) {
        auto def = ChipLibrary::instance().findById(chipId);
        if (!def) return nullptr;
        return std::make_shared<CustomChip>(pos, font, def);
    }

    std::shared_ptr<CustomChip> createChipInstance(const ChipDefinition::Ptr& def, sf::Vector2f pos, const sf::Font& font) {
        if (!def) return nullptr;
        return std::make_shared<CustomChip>(pos, font, def);
    }

    namespace {
        std::shared_ptr<Gate> makeGate(GateType t, sf::Vector2f pos, const sf::Font& font,
                                    const std::string& chipId, const std::string& pinName) {
            switch (t) {
                case GateType::AND: return std::make_shared<ANDGate>(pos, font);
                case GateType::OR: return std::make_shared<ORGate>(pos, font);
                case GateType::NOT: return std::make_shared<NOTGate>(pos, font);
                case GateType::XOR: return std::make_shared<XORGate>(pos, font);
                case GateType::NAND: return std::make_shared<NANDGate>(pos, font);
                case GateType::NOR: return std::make_shared<NORGate>(pos, font);
                case GateType::XNOR: return std::make_shared<XNORGate>(pos, font);
                case GateType::CLOCK: return std::make_shared<ClockGate>(pos, font);
                case GateType::POWER: return std::make_shared<PowerSource>(pos, font);
                case GateType::SWITCH: return std::make_shared<Switch>(pos, font);
                case GateType::BUTTON: return std::make_shared<Button>(pos, font);
                case GateType::LED: return std::make_shared<LED>(pos, font);
                case GateType::ROM: return std::make_shared<ROM>(pos, font);
                case GateType::SEG7: return std::make_shared<Seg7Display>(pos, font);
                case GateType::LCD: return std::make_shared<LCDDisplay>(pos, font);
                case GateType::COUNTER: return std::make_shared<Counter>(pos, font);
                case GateType::REGISTER: return std::make_shared<Register>(pos, font);
                case GateType::INPUT_PIN: return std::make_shared<InputPin>(pos, font, pinName);
                case GateType::OUTPUT_PIN: return std::make_shared<OutputPin>(pos, font, pinName);
                case GateType::CHIP: return createChipInstance(chipId, pos, font);
                default: return nullptr;
            }
        }
    }

    JsonValue chipDefinitionToJson(const ChipDefinition::Ptr& def) {
        JsonValue root = JsonValue::makeObject();
        root.set("id", def->id);
        root.set("name", def->name);
        root.set("description", def->description);
        root.set("category", def->category);
        root.set("version", def->version);

        JsonValue ins = JsonValue::makeArray();
        for (auto& p : def->inputs) ins.push(JsonValue(p.name));
        root.set("inputs", ins);

        JsonValue outs = JsonValue::makeArray();
        for (auto& p : def->outputs) outs.push(JsonValue(p.name));
        root.set("outputs", outs);

        // Circuito interno
        JsonValue circuit = JsonValue::makeObject();
        JsonValue gates = JsonValue::makeArray();
        for (auto& g : def->circuit->gates) {
            JsonValue jg = JsonValue::makeObject();
            jg.set("type", (int)g->getType());
            jg.set("x", g->getPosition().x);
            jg.set("y", g->getPosition().y);
            jg.set("extra", g->getExtraData());
            if (g->getType() == GateType::CHIP) jg.set("chipId", g->getChipId());
            if (g->getType() == GateType::INPUT_PIN || g->getType() == GateType::OUTPUT_PIN)
                jg.set("pinName", g->getPinName());
            gates.push(jg);
        }
        circuit.set("gates", gates);

        JsonValue wires = JsonValue::makeArray();
        for (auto& w : def->circuit->wires) {
            int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
            for (size_t i = 0; i < def->circuit->gates.size(); ++i) {
                if (def->circuit->gates[i].get() == w.fromPin->parent) gFrom = (int)i;
                if (def->circuit->gates[i].get() == w.toPin->parent)   gTo   = (int)i;
            }
            for (size_t i = 0; i < w.fromPin->parent->outputs.size(); ++i) {
                if (&w.fromPin->parent->outputs[i] == w.fromPin) pFrom = (int)i;
            }
            for (size_t i = 0; i < w.toPin->parent->inputs.size(); ++i) {
                if (&w.toPin->parent->inputs[i] == w.toPin) pTo = (int)i;
            }
            JsonValue jw = JsonValue::makeObject();
            jw.set("fromGate", gFrom);
            jw.set("fromPin", pFrom);
            jw.set("toGate", gTo);
            jw.set("toPin", pTo);
            wires.push(jw);
        }
        circuit.set("wires", wires);
        root.set("circuit", circuit);

        return root;
    }

    ChipDefinition::Ptr chipDefinitionFromJson(const JsonValue& v, const sf::Font& font) {
        auto def = std::make_shared<ChipDefinition>();
        def->id = v.get("id").asString();
        def->name = v.get("name").asString("Chip");
        def->description = v.get("description").asString();
        def->category = v.get("category").asString();
        def->version = v.get("version").asInt(1);

        auto ins = v.get("inputs");
        if (ins.isArray()) for (auto& e : ins.arr) def->inputs.push_back({e.asString()});
        auto outs = v.get("outputs");
        if (outs.isArray()) for (auto& e : outs.arr) def->outputs.push_back({e.asString()});

        auto circuit = v.get("circuit");
        auto gates = circuit.get("gates");
        if (gates.isArray()) {
            for (auto& jg : gates.arr) {
                GateType t = (GateType)jg.get("type").asInt(0);
                sf::Vector2f pos(jg.get("x").asNumber(0), jg.get("y").asNumber(0));
                auto g = makeGate(t, pos, font, jg.get("chipId").asString(),
                                jg.get("pinName").asString("PIN"));
                if (g) {
                    g->setExtraData((float)jg.get("extra").asNumber(0));
                    def->circuit->gates.push_back(g);
                }
            }
        }
        auto wires = circuit.get("wires");
        if (wires.isArray()) {
            for (auto& jw : wires.arr) {
                int gF = jw.get("fromGate").asInt(-1);
                int pF = jw.get("fromPin").asInt(-1);
                int gT = jw.get("toGate").asInt(-1);
                int pT = jw.get("toPin").asInt(-1);
                if (gF >= 0 && gT >= 0 && gF < (int)def->circuit->gates.size() &&
                    gT < (int)def->circuit->gates.size()) {
                    auto& a = def->circuit->gates[gF];
                    auto& b = def->circuit->gates[gT];
                    if (pF >= 0 && pF < (int)a->outputs.size() &&
                        pT >= 0 && pT < (int)b->inputs.size()) {
                        def->circuit->wires.emplace_back(&a->outputs[pF], &b->inputs[pT]);
                    }
                }
            }
        }
        return def;
    }

    bool saveChipDefinition(const ChipDefinition::Ptr& def, const std::string& path) {
        if (!def) return false;
        std::ofstream f(path);
        if (!f) return false;
        f << jsonToString(chipDefinitionToJson(def), 2);
        return true;
    }

    ChipDefinition::Ptr loadChipDefinition(const std::string& path, const sf::Font& font) {
        std::ifstream f(path);
        if (!f) return nullptr;
        std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        try {
            return chipDefinitionFromJson(jsonParse(text), font);
        } catch (...) {
            return nullptr;
        }
    }

    JsonValue projectToJson(const std::vector<std::shared_ptr<Gate>>& gates,
                            const std::vector<Wire>& wires,
                            const std::string& name,
                            const std::string& category,
                            const std::string& description,
                            const std::string& tags) {
        JsonValue root = JsonValue::makeObject();
        root.set("format", "eletronic-project");
        root.set("version", 1);
        root.set("name", name);
        root.set("category", category);
        root.set("description", description);
        root.set("tags", tags);

        // Embute as definições de chips referenciadas (autossuficiência).
        std::set<std::string> usedChips;
        for (auto& g : gates) {
            if (g->getType() == GateType::CHIP) usedChips.insert(g->getChipId());
        }

        JsonValue lib = JsonValue::makeArray();
        for (auto& id : usedChips) {
            auto def = ChipLibrary::instance().findById(id);
            if (def) lib.push(chipDefinitionToJson(def));
        }
        root.set("chipLibrary", lib);

        JsonValue jgates = JsonValue::makeArray();
        for (auto& g : gates) {
            JsonValue jg = JsonValue::makeObject();
            jg.set("type", (int)g->getType());
            jg.set("x", g->getPosition().x);
            jg.set("y", g->getPosition().y);
            jg.set("extra", g->getExtraData());
            if (g->getType() == GateType::CHIP) jg.set("chipId", g->getChipId());
            if (g->getType() == GateType::INPUT_PIN || g->getType() == GateType::OUTPUT_PIN)
                jg.set("pinName", g->getPinName());
            jgates.push(jg);
        }
        root.set("gates", jgates);

        JsonValue jwires = JsonValue::makeArray();
        for (auto& w : wires) {
            int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
            for (size_t i = 0; i < gates.size(); ++i) {
                if (gates[i].get() == w.fromPin->parent) gFrom = (int)i;
                if (gates[i].get() == w.toPin->parent)   gTo   = (int)i;
            }
            for (size_t i = 0; i < w.fromPin->parent->outputs.size(); ++i) {
                if (&w.fromPin->parent->outputs[i] == w.fromPin) pFrom = (int)i;
            }
            for (size_t i = 0; i < w.toPin->parent->inputs.size(); ++i) {
                if (&w.toPin->parent->inputs[i] == w.toPin) pTo = (int)i;
            }
            JsonValue jw = JsonValue::makeObject();
            jw.set("fromGate", gFrom);
            jw.set("fromPin", pFrom);
            jw.set("toGate", gTo);
            jw.set("toPin", pTo);
            jwires.push(jw);
        }
        root.set("wires", jwires);

        return root;
    }

    bool projectFromJson(const JsonValue& v,
                        std::vector<std::shared_ptr<Gate>>& gates,
                        std::vector<Wire>& wires,
                        std::string& name,
                        std::string& category,
                        std::string& description,
                        std::string& tags,
                        const sf::Font& font) {
        name = v.get("name").asString();
        category = v.get("category").asString();
        description = v.get("description").asString();
        tags = v.get("tags").asString();

        // 1) Registra as definições embutidas (chipLibrary) antes de instanciar.
        auto lib = v.get("chipLibrary");
        if (lib.isArray()) {
            for (auto& jdef : lib.arr) {
                auto def = chipDefinitionFromJson(jdef, font);
                if (def) ChipLibrary::instance().registerChip(def);
            }
        }

        // 2) Recria os gates.
        auto jgates = v.get("gates");
        if (!jgates.isArray()) return false;
        for (auto& jg : jgates.arr) {
            GateType t = (GateType)jg.get("type").asInt(0);
            sf::Vector2f pos(jg.get("x").asNumber(0), jg.get("y").asNumber(0));
            auto g = makeGate(t, pos, font, jg.get("chipId").asString(), jg.get("pinName").asString("PIN"));
            if (g) {
                g->setExtraData((float)jg.get("extra").asNumber(0));
                gates.push_back(g);
            }
        }

        // 3) Reconecta os fios.
        auto jwires = v.get("wires");
        if (jwires.isArray()) {
            for (auto& jw : jwires.arr) {
                int gF = jw.get("fromGate").asInt(-1);
                int pF = jw.get("fromPin").asInt(-1);
                int gT = jw.get("toGate").asInt(-1);
                int pT = jw.get("toPin").asInt(-1);
                if (gF >= 0 && gT >= 0 && gF < (int)gates.size() && gT < (int)gates.size()) {
                    auto& a = gates[gF];
                    auto& b = gates[gT];
                    if (pF >= 0 && pF < (int)a->outputs.size() &&
                        pT >= 0 && pT < (int)b->inputs.size()) {
                        wires.emplace_back(&a->outputs[pF], &b->inputs[pT]);
                    }
                }
            }
        }
        return true;
    }
}