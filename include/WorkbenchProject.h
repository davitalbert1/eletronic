#pragma once
#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include "LogicEntities.h"
#include "Wire.h"
#ifdef _WIN32
    #include <windows.h>
#endif

namespace fs = std::filesystem;

class WorkbenchProject {
public:
    struct ProjectInfo {
        std::string name;
        std::string category;
        fs::path fullPath;
        std::uintmax_t size = 0;
        std::filesystem::file_time_type modified;
        std::string createdStr;
        size_t gateCount = 0;
        size_t wireCount = 0;
        std::string description;
        std::vector<std::string> tags;
        bool starred = false;
    };

    WorkbenchProject() = default;

    // Project I/O
    void writeProject(const fs::path& p,
                    const std::vector<std::shared_ptr<Gate>>& placedGates,
                    const std::vector<Wire>& wires,
                    const std::string& projectName,
                    const std::string& category,
                    const std::string& description,
                    const std::string& tags);

    bool loadProject(const fs::path& p,
                    std::vector<std::shared_ptr<Gate>>& placedGates,
                    std::vector<Wire>& wires,
                    std::string& projectName,
                    std::string& category,
                    std::string& description,
                    std::string& tags,
                    const sf::Font& font);

    // Export
    void exportJSON(const std::string& baseName,
                    const std::string& category,
                    const std::vector<std::shared_ptr<Gate>>& placedGates,
                    const std::vector<Wire>& wires);

    void exportXML(const std::string& baseName,
                const std::string& category,
                const std::vector<std::shared_ptr<Gate>>& placedGates,
                const std::vector<Wire>& wires);

    // Backup
    void createBackup(const std::string& projectName,
                    const std::vector<std::shared_ptr<Gate>>& placedGates,
                    const std::vector<Wire>& wires);

    // Metadata
    fs::path metaPath(const fs::path& gatePath) const;
    void loadMeta(ProjectInfo& info) const;
    void saveMeta(const ProjectInfo& info) const;
    static std::vector<std::string> splitTags(const std::string& s);
    static std::string joinTags(const std::vector<std::string>& tags);

    // Utilities
    static std::string formatSize(std::uintmax_t bytes);
    std::string formatFileTime(std::filesystem::file_time_type t) const;
    fs::path projectDir(const std::string& category) const;
    ProjectInfo readProjectInfo(const fs::path& p, const std::string& category) const;

private:
    std::string fileTimeToString(FILETIME ft) const;
};
