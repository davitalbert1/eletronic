#include "WorkbenchProject.h"
#include <algorithm>
#include <cstdint>
#include <windows.h>
#include <ctime>
#include <iomanip>
#include <sstream>

WorkbenchProject::ProjectInfo WorkbenchProject::readProjectInfo(const fs::path& p, const std::string& category) const {
    ProjectInfo info;
    info.name = p.stem().string();
    info.category = category;
    info.fullPath = fs::absolute(p);
    std::error_code ec;
    info.size = fs::file_size(p, ec);
    info.modified = fs::last_write_time(p, ec);

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(p.wstring().c_str(), GetFileExInfoStandard, &fad)) {
        info.createdStr = fileTimeToString(fad.ftCreationTime);
    }

    std::ifstream f(p);
    if (f) {
        size_t gateCount = 0;
        if (f >> gateCount) {
            info.gateCount = gateCount;
            std::string dummy;
            for (size_t i = 0; i < gateCount; ++i) std::getline(f, dummy);
            size_t wireCount = 0;
            if (f >> wireCount) info.wireCount = wireCount;
        }
    }

    loadMeta(info);
    return info;
}

void WorkbenchProject::writeProject(const fs::path& p, 
                                    const std::vector<std::shared_ptr<Gate>>& placedGates,
                                    const std::vector<Wire>& wires,
                                    const std::string& projectName,
                                    const std::string& category,
                                    const std::string& description,
                                    const std::string& tags) {
    std::ofstream f(p);
    if (!f) return;

    f << placedGates.size() << "\n";
    for (auto& g : placedGates) {
        f << (int)g->getType() << " " << g->getPosition().x << " " << g->getPosition().y << " " << g->getExtraData() << "\n";
    }

    auto getGateIdx = [&](Gate* g) -> int {
        for (size_t i = 0; i < placedGates.size(); ++i) {
            if (placedGates[i].get() == g) return (int)i;
        }
        return -1;
    };
    auto getPinIdx = [&](Gate* g, Pin* p, bool input) -> int {
        const auto& pins = input ? g->inputs : g->outputs;
        for (size_t i = 0; i < pins.size(); ++i) {
            if (&pins[i] == p) return (int)i;
        }
        return -1;
    };

    f << wires.size() << "\n";
    for (auto& w : wires) {
        int gFrom = getGateIdx(w.fromPin->parent);
        int pFrom = getPinIdx(w.fromPin->parent, w.fromPin, false);
        int gTo = getGateIdx(w.toPin->parent);
        int pTo = getPinIdx(w.toPin->parent, w.toPin, true);
        f << gFrom << " " << pFrom << " " << gTo << " " << pTo << "\n";
    }
    f.close();

    if (!projectName.empty()) {
        ProjectInfo info;
        info.name = projectName;
        info.category = category;
        info.fullPath = fs::absolute(p);
        info.description = description;
        info.tags = splitTags(tags);
        saveMeta(info);
    }
}

bool WorkbenchProject::loadProject(const fs::path& p,
                                   std::vector<std::shared_ptr<Gate>>& placedGates,
                                   std::vector<Wire>& wires,
                                   std::string& projectName,
                                   std::string& category,
                                   std::string& description,
                                   std::string& tags,
                                   const sf::Font& font) {
    std::ifstream f(p);
    if (!f) return false;

    placedGates.clear();
    wires.clear();

    size_t gateCount;
    if (!(f >> gateCount)) return false;
    if (gateCount > 10000) return false;

    while (gateCount--) {
        int t;
        float x, y, extra;
        if (!(f >> t >> x >> y >> extra)) return false;

        if (t < 0 || t > 4) continue;

        sf::Vector2f pos(x, y);
        std::shared_ptr<Gate> newGate;
        switch ((GateType)t) {
            case GateType::AND:
                newGate = std::make_shared<ANDGate>(pos, font);
                break;
            case GateType::OR:
                newGate = std::make_shared<ORGate>(pos, font);
                break;
            case GateType::NOT:
                newGate = std::make_shared<NOTGate>(pos, font);
                break;
            case GateType::CLOCK:
                newGate = std::make_shared<ClockGate>(pos, font);
                break;
            case GateType::POWER:
                newGate = std::make_shared<PowerSource>(pos, font);
                break;
        }
        if (newGate) {
            newGate->setExtraData(extra);
            placedGates.push_back(newGate);
        }
    }

    size_t wireCount;
    if (f >> wireCount) {
        if (wireCount > 50000) return false;

        while (wireCount--) {
            int gF, pF, gT, pT;
            if (!(f >> gF >> pF >> gT >> pT)) break;

            if (gF >= 0 && gF < (int)placedGates.size() && 
                gT >= 0 && gT < (int)placedGates.size()) {

                if (pF >= 0 && pF < (int)placedGates[gF]->outputs.size() &&
                    pT >= 0 && pT < (int)placedGates[gT]->inputs.size()) {
                    Pin* from = &placedGates[gF]->outputs[pF];
                    Pin* to = &placedGates[gT]->inputs[pT];
                    wires.emplace_back(from, to);
                }
            }
        }
    }

    return true;
}

void WorkbenchProject::exportJSON(const std::string& baseName, 
                                  const std::string& category,
                                  const std::vector<std::shared_ptr<Gate>>& placedGates,
                                  const std::vector<Wire>& wires) {
    fs::path dir = projectDir(category);
    fs::create_directories(dir);
    fs::path p = dir / (baseName + ".json");
    std::ofstream f(p);
    if (!f) return;

    f << "{\n";
    f << "  \"name\": \"" << baseName << "\",\n";
    f << "  \"category\": \"" << category << "\",\n";
    f << "  \"gates\": [\n";
    for (size_t i = 0; i < placedGates.size(); ++i) {
        auto& g = placedGates[i];
        f << "    {\"index\": " << i << ", \"type\": " << (int)g->getType()
          << ", \"x\": " << g->getPosition().x << ", \"y\": " << g->getPosition().y
          << ", \"extra\": " << g->getExtraData() << "}";
        f << (i + 1 < placedGates.size() ? ",\n" : "\n");
    }
    f << "  ],\n";
    f << "  \"wires\": [\n";
    for (size_t i = 0; i < wires.size(); ++i) {
        auto& w = wires[i];
        int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
        for (size_t gi = 0; gi < placedGates.size(); ++gi) {
            if (placedGates[gi].get() == w.fromPin->parent) gFrom = (int)gi;
            if (placedGates[gi].get() == w.toPin->parent) gTo = (int)gi;
        }
        for (size_t pi = 0; pi < w.fromPin->parent->outputs.size(); ++pi)
            if (&w.fromPin->parent->outputs[pi] == w.fromPin) pFrom = (int)pi;
        for (size_t pi = 0; pi < w.toPin->parent->inputs.size(); ++pi)
            if (&w.toPin->parent->inputs[pi] == w.toPin) pTo = (int)pi;
        f << "    {\"fromGate\": " << gFrom << ", \"fromPin\": " << pFrom
          << ", \"toGate\": " << gTo << ", \"toPin\": " << pTo << "}";
        f << (i + 1 < wires.size() ? ",\n" : "\n");
    }
    f << "  ]\n";
    f << "}\n";
    f.close();
}

void WorkbenchProject::exportXML(const std::string& baseName,
                                 const std::string& category,
                                 const std::vector<std::shared_ptr<Gate>>& placedGates,
                                 const std::vector<Wire>& wires) {
    fs::path dir = projectDir(category);
    fs::create_directories(dir);
    fs::path p = dir / (baseName + ".xml");
    std::ofstream f(p);
    if (!f) return;

    f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    f << "<project name=\"" << baseName << "\" category=\"" << category << "\">\n";
    f << "  <gates>\n";
    for (size_t i = 0; i < placedGates.size(); ++i) {
        auto& g = placedGates[i];
        f << "    <gate index=\"" << i << "\" type=\"" << (int)g->getType()
          << "\" x=\"" << g->getPosition().x << "\" y=\"" << g->getPosition().y
          << "\" extra=\"" << g->getExtraData() << "\"/>\n";
    }
    f << "  </gates>\n";
    f << "  <wires>\n";
    for (auto& w : wires) {
        int gFrom = -1, pFrom = -1, gTo = -1, pTo = -1;
        for (size_t gi = 0; gi < placedGates.size(); ++gi) {
            if (placedGates[gi].get() == w.fromPin->parent) { gFrom = (int)gi; }
            if (placedGates[gi].get() == w.toPin->parent) { gTo = (int)gi; }
        }
        for (size_t pi = 0; pi < w.fromPin->parent->outputs.size(); ++pi)
            if (&w.fromPin->parent->outputs[pi] == w.fromPin) pFrom = (int)pi;
        for (size_t pi = 0; pi < w.toPin->parent->inputs.size(); ++pi)
            if (&w.toPin->parent->inputs[pi] == w.toPin) pTo = (int)pi;
        f << "    <wire fromGate=\"" << gFrom << "\" fromPin=\"" << pFrom
          << "\" toGate=\"" << gTo << "\" toPin=\"" << pTo << "\"/>\n";
    }
    f << "  </wires>\n";
    f << "</project>\n";
    f.close();
}

void WorkbenchProject::createBackup(const std::string& projectName,
                                    const std::vector<std::shared_ptr<Gate>>& placedGates,
                                    const std::vector<Wire>& wires) {
    if (projectName.empty()) return;
    fs::path backupDir = "projetos/backups";
    fs::create_directories(backupDir);
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &tt);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &tm);
    fs::path p = backupDir / (projectName + "_" + ts + ".gate");
    writeProject(p, placedGates, wires, "", "", "", "");
}

fs::path WorkbenchProject::metaPath(const fs::path& gatePath) const {
    return gatePath.string() + ".meta";
}

void WorkbenchProject::loadMeta(ProjectInfo& info) const {
    fs::path mp = metaPath(info.fullPath);
    std::ifstream f(mp);
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        if (line.compare(0, 12, "description=") == 0) {
            info.description = line.substr(12);
        } else if (line.compare(0, 5, "tags=") == 0) {
            info.tags = splitTags(line.substr(5));
        } else if (line.compare(0, 8, "starred=") == 0) {
            info.starred = (line.substr(8) == "1" || line.substr(8) == "true");
        }
    }
}

void WorkbenchProject::saveMeta(const ProjectInfo& info) const {
    fs::path mp = metaPath(info.fullPath);
    std::ofstream f(mp);
    if (!f) return;

    f << "description=" << info.description << "\n";
    f << "tags=" << joinTags(info.tags) << "\n";
    f << "starred=" << (info.starred ? "1" : "0") << "\n";
}

std::vector<std::string> WorkbenchProject::splitTags(const std::string& s) {
    std::vector<std::string> result;
    std::istringstream iss(s);
    std::string tag;
    while (std::getline(iss, tag, ',')) {
        tag.erase(0, tag.find_first_not_of(" \t"));
        tag.erase(tag.find_last_not_of(" \t") + 1);
        if (!tag.empty()) result.push_back(tag);
    }
    return result;
}

std::string WorkbenchProject::joinTags(const std::vector<std::string>& tags) {
    std::string result;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) result += ",";
        result += tags[i];
    }
    return result;
}

std::string WorkbenchProject::formatSize(std::uintmax_t bytes) {
    if (bytes < 1024) return std::to_string(bytes) + " B";
    double kb = bytes / 1024.0;
    if (kb < 1024) return std::to_string(static_cast<int>(kb)) + " KB";
    double mb = kb / 1024.0;
    return std::to_string(static_cast<int>(mb)) + " MB";
}

std::string WorkbenchProject::formatFileTime(std::filesystem::file_time_type t) const {
    auto now_sys = std::chrono::system_clock::now();
    auto now_file = std::filesystem::file_time_type::clock::now();
    auto offset = now_sys.time_since_epoch() -
        std::chrono::duration_cast<std::chrono::system_clock::duration>(now_file.time_since_epoch());
    auto sys_dur = std::chrono::duration_cast<std::chrono::system_clock::duration>(t.time_since_epoch());
    auto sys_tp = std::chrono::system_clock::time_point(sys_dur + offset);
    std::time_t tt = std::chrono::system_clock::to_time_t(sys_tp);
    std::tm tm{};
    localtime_s(&tm, &tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y %H:%M");
    return oss.str();
}

std::string WorkbenchProject::fileTimeToString(FILETIME ft) const {
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    auto dur = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::nanoseconds(ul.QuadPart * 100));
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(dur));
    std::tm tm{};
    localtime_s(&tm, &tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y %H:%M");
    return oss.str();
}

fs::path WorkbenchProject::projectDir(const std::string& category) const {
    fs::path base = "projetos";
    if (category.empty()) return base;
    return base / category;
}