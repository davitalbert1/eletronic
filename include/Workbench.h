#pragma once
#include <vector>
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include "LogicEntities.h"
#include "Wire.h"
#include "UndoRedoManager.h"
#include "ConfirmationDialog.h"
#include "WorkbenchProject.h"
#include "WorkbenchUI.h"
#include "WorkbenchInput.h"
#include "WorkbenchAccessibility.h"
#include "WorkbenchPerformance.h"
#include "chip/ChipLibrary.h"
#include "chip/ChipFactory.h"
#include "chip/ChipDefinition.h"
#include "CircuitLanguageParser.h"
#include "ImageExporter.h"
#include "PdfExporter.h"
#include "DigitalLogicSimImporter.h"

class Workbench {
public:
    Workbench(const sf::Font& font);

    // Main operations
    void newProject();
    void save(const std::string& name);
    void load(const std::string& name);

    // Chips personalizados
    // Converte o circuito atual em um chip reutilizavel. Requer que o
    // projeto contenha ao menos um InputPin e um OutputPin.
    bool saveCurrentAsChip(const std::string& id, const std::string& name,
                           const std::string& description, const std::string& category,
                           bool askUpdate = true);
    // Atualiza a definicao de um chip ja registrado a partir do circuito atual.
    void updateChipFromCurrent(const std::string& id);
    // Carrega um chip na bancada de trabalho para edicao (Digital Logic Sim style)
    bool loadChipForEditing(const std::string& id);
    // Deleta um chip personalizado da biblioteca e do disco
    bool deleteChip(const std::string& id);
    // Carrega todos os chips salvos no disco (projetos/chips/)
    void loadAllChipsFromDisk();
    // Adiciona um chip da biblioteca a paleta (chamado apos registrar/deletar).
    void refreshChipPalette();

    // Rendering
    void draw(sf::RenderWindow& window);
    void renderImGui();

    // Update
    void update(float deltaTime);
    void updatePropagation();

    // Event handling
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    // Project management
    void refreshProjectList();
    void refreshCategories();

    // Digital Logic Sim import
    void importDigitalLogicSimProject(const std::string& filePath);
    // Circuit Language Parser
    void importFromCircuitLanguage(const std::string& filePath);
    void exportToCircuitLanguage(const std::string& filePath);
    void showCircuitLanguageDialog();

    // Universal Circuit Window and Component Inspector
    void renderInspector();
    void renderUniversalCircuitWindow();
    std::shared_ptr<Gate> getSelectedGate();

    // Image export
    void exportToImage(const std::string& filePath, const std::string& format); // "png" or "svg"

    // PDF export
    void exportToPDF(const std::string& filePath);

    // Enhanced JSON/XML export
    void exportJSONEnhanced(const std::string& baseName, const std::string& category);
    void exportXMLEnhanced(const std::string& baseName, const std::string& category);
    ImportResult getLastImportResult() const {
        return lastImportResult_;
    }
    void applyImportResult(const ImportResult& importResult);

    // Modal layout geometry
    static constexpr float ModalX = 340.f, ModalY = 140.f, ModalW = 520.f, ModalH = 560.f;
    static constexpr float LoadModalH = 580.f;
    static constexpr float ChipW = 100.f, ChipH = 24.f, ChipGap = 8.f;

    // Project metadata (using WorkbenchProject::ProjectInfo)
    using ProjectInfo = WorkbenchProject::ProjectInfo;

private:
    const sf::Font& font_;

    // Core data
    std::vector<std::shared_ptr<Gate>> placedGates_;
    std::vector<Wire> wires_;
    std::vector<std::shared_ptr<Gate>> paletteItems_;

    // Project state
    std::string currentProjectName_;
    std::string currentCategory_;
    std::string currentDescription_;
    std::string currentTags_;
    std::vector<ProjectInfo> projectInfos_;
    size_t selectedProjectIndex_ = 0;
    std::vector<std::string> categories_;

    // Modal state
    bool isSaveModal_ = false;
    bool isLoadModal_ = false;
    bool showStartupDialog_ = true;
    bool saveAsMode_ = false;
    std::string inputBuffer_;
    std::string categoryBuffer_;
    std::string descriptionBuffer_;
    std::string tagsBuffer_;
    std::string searchBuffer_;
    int listScroll_ = 0;
    int loadCategorySel_ = 0;
    using ModalFocus = WorkbenchInput::InputState::ModalFocus;
    ModalFocus modalFocus_ = ModalFocus::NAME;

    // Status
    std::string statusMessage_;
    float statusMessageTimer_ = 0.0f;

    // Automatic backup
    float backupTimer_ = 0.0f;
    bool autoBackupEnabled_ = true;
    static constexpr float BackupInterval = 300.0f;

    // Progress
    bool isSaving_ = false;
    bool isLoading_ = false;
    float saveProgress_ = 0.0f;
    // Circuit Language Parser state
    CircuitLanguageParser circuitLanguageParser_;
    bool showCircuitLanguageDialog_ = false;
    bool showUniversalWindow_ = true;
    bool showInspector_ = true;
    std::string inspectorTextBuffer_;
    std::shared_ptr<Gate> lastInspectedGate_;
    std::string universalEditorBuffer_;
    std::string cdlErrorMessage_;
    std::vector<std::string> cdlWarnings_;
    bool showExportImageDialog_ = false;
    bool showExportPdfDialog_ = false;
    std::string circuitLanguageBuffer_;
    std::string exportFilePathBuffer_;
    std::string exportFormatBuffer_ = "png";

    // Image and PDF exporters
    ImageExporter imageExporter_;
    PdfExporter pdfExporter_;
    PdfExportOptions pdfOptions_;
    float loadProgress_ = 0.0f;

    // Help system
    bool showHelp_ = false;

    // Digital Logic Sim import
    ImportResult lastImportResult_;
    bool showImportResultDialog_ = false;
    DigitalLogicSimImporter dlsImporter_;

    // Component grouping (using WorkbenchInput::ComponentGroup)
    using ComponentGroup = WorkbenchInput::ComponentGroup;
    std::vector<ComponentGroup> groups_;
    size_t selectedGroup_ = SIZE_MAX;

    // Selection filter
    WorkbenchInput::SelectionFilter currentFilter_ = WorkbenchInput::SelectionFilter::ALL;

    // Snap-to-grid
    bool enableSnapToGrid_ = true;
    sf::Vector2f snapToGrid(sf::Vector2f pos);

    // Palette scrolling
    float paletteScroll_ = 0.0f;
    static constexpr float PaletteItemStep = 90.0f;
    static constexpr float PaletteTop = Config::TopMenuHeight + 20.0f;
    static constexpr float PaletteViewBottom = 800.0f;
    void updatePalettePositions();

    // Itens de paleta que representam chips (id da definiÃ§Ã£o).
    std::vector<std::string> chipPaletteIds_;

    // Undo/Redo
    std::unique_ptr<UndoRedoManager> undoRedoManager_;

    // Confirmation dialog
    std::unique_ptr<ConfirmationDialog> confirmationDialog_;
    bool showConfirmDialog_ = false;
    std::string confirmMessage_;

    // Specialized modules
    WorkbenchProject projectManager_;
    WorkbenchUI ui_;
    WorkbenchInput input_;
    WorkbenchAccessibility accessibility_;
    WorkbenchPerformance performance_;

    // Helper methods
    void saveState();
    void undo();
    void redo();
    void showConfirmation(ConfirmationDialog::Action action, const std::string& message, int projectIndex = -1);
    void handleConfirmDialog();
    void toggleHelp();
    void drawHelpOverlay(sf::RenderWindow& window);
    void drawImportResultDialog(sf::RenderWindow& window);
    void setupPalette();
    void applySort();
    void applySearch();
    std::string sortOrderLabel() const;
    static std::vector<std::string> splitTags(const std::string& s);
    static std::string joinTags(const std::vector<std::string>& tags);
    void toggleStarred(size_t index);
    void deleteProject(size_t index);
    void writeProject(const fs::path& p);
    void exportJSON(const std::string& baseName, const std::string& category);
    void exportXML(const std::string& baseName, const std::string& category);
    void createBackup();
    void cycleTheme();
    void cycleFontScale();
    void toggleHighContrast();
    void toggleLargeText();
    void applyTheme();
    void applyFontScale();
    void togglePerformanceMode();
    void updatePerformanceSettings();
    void handleError(const std::string& error);
    void clearError();
    bool validateProjectData() const;
    bool attemptRecovery();
    void saveEmergencyBackup();
};

