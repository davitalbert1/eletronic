#include "WorkbenchUI.h"
#include <algorithm>

WorkbenchUI::WorkbenchUI(const sf::Font& font) 
    : font_(font)
    , menuText_(font_)
    , saveText_(font_)
    , loadText_(font_)
    , newText_(font_)
    , saveAsText_(font_)
    , modalTitle_(font_)
    , inputText_(font_)
    , listText_(font_)
    , tooltipText_(font_)
    , importDlsText_(font_)
    , progressText_(font_) {
    setupMenu();
    setupPalette();
    setupModal();
    setupTooltip();
}

void WorkbenchUI::setupMenu() {
    menuBar_.setSize({1200, Config::TopMenuHeight});
    menuBar_.setFillColor(Theme::TopMenu);

    menuButton_.setSize({100, Config::TopMenuHeight - 10});
    menuButton_.setPosition({10, 5});
    menuButton_.setFillColor(Theme::Panel);
    menuButton_.setOutlineThickness(3);
    menuButton_.setOutlineColor(Theme::GateOutline);

    menuText_.setFont(font_);
    menuText_.setString("Menu");
    menuText_.setCharacterSize(16);
    menuText_.setFillColor(Theme::Text);
    menuText_.setPosition({25, 10});

    dropdown_.setSize({140, 150});
    dropdown_.setFillColor(Theme::Panel);
    dropdown_.setOutlineThickness(3);
    dropdown_.setOutlineColor(Theme::GateOutline);

    newText_.setFont(font_);
    newText_.setString("New (Ctrl+N)");
    newText_.setCharacterSize(14);
    saveText_.setFont(font_);
    saveText_.setString("Save (Ctrl+S)");
    saveText_.setCharacterSize(14);
    loadText_.setFont(font_);
    loadText_.setString("Load (Ctrl+L)");
    loadText_.setCharacterSize(14);
    saveAsText_.setFont(font_);
    saveAsText_.setString("Save As...");
    saveAsText_.setCharacterSize(14);

    importDlsText_.setFont(font_);
    importDlsText_.setString("Importar DLS");
    importDlsText_.setCharacterSize(14);
}

void WorkbenchUI::setupPalette() {
    paletteRect_.setSize({Config::PaletteWidth, 800});
    paletteRect_.setFillColor(Theme::Panel);
    paletteRect_.setPosition({0, Config::TopMenuHeight});
}

void WorkbenchUI::setupModal() {
    overlay_.setSize({1200, 800});
    overlay_.setFillColor(Theme::Overlay);

    modalRect_.setSize({ModalW, ModalH});
    modalRect_.setFillColor(Theme::ModalBg);
    modalRect_.setOutlineThickness(2);
    modalRect_.setOutlineColor(Theme::ModalBorder);
    modalRect_.setPosition({ModalX, ModalY});

    modalTitle_.setCharacterSize(20);
    modalTitle_.setFillColor(Theme::Highlight);

    inputText_.setCharacterSize(18);
    inputText_.setFillColor(Theme::Text);

    listText_.setCharacterSize(16);
    listText_.setFillColor(Theme::Text);
}

void WorkbenchUI::setupTooltip() {
    tooltipBg_.setFillColor(Theme::TooltipBg);
    tooltipBg_.setOutlineThickness(1);
    tooltipBg_.setOutlineColor(Theme::TooltipBorder);

    tooltipText_.setCharacterSize(14);
    tooltipText_.setFillColor(Theme::TooltipText);

    progressBar_.setFillColor(Theme::ProgressFill);
    progressBar_.setOutlineThickness(1);
    progressBar_.setOutlineColor(Theme::ProgressBorder);
}

void WorkbenchUI::drawMenu(sf::RenderWindow& window) {
    window.draw(menuBar_);

    if (isMenuButtonPressed_) {
        menuButton_.setFillColor(Theme::ButtonPressed);
    } else if (isMenuButtonHovered_) {
        menuButton_.setFillColor(Theme::ButtonHover);
    } else {
        menuButton_.setFillColor(Theme::ButtonNormal);
    }
    sf::Color menuBtnColor = menuButton_.getFillColor();
    menuButton_.setFillColor(sf::Color(menuBtnColor.r, menuBtnColor.g, menuBtnColor.b,
        static_cast<std::uint8_t>(menuButtonAlpha_)));
    window.draw(menuButton_);
    window.draw(menuText_);

    if (showDropdown_ || dropdownAlpha_ > 0) {
        dropdown_.setPosition({10, Config::TopMenuHeight});
        sf::Color dropdownColor = dropdown_.getFillColor();
        dropdown_.setFillColor(sf::Color(dropdownColor.r,
            dropdownColor.g,
            dropdownColor.b,
            static_cast<std::uint8_t>(dropdownAlpha_)));
        window.draw(dropdown_);
        newText_.setPosition({20, Config::TopMenuHeight + 10});
        saveText_.setPosition({20, Config::TopMenuHeight + 45});
        loadText_.setPosition({20, Config::TopMenuHeight + 80});
        saveAsText_.setPosition({20, Config::TopMenuHeight + 115});
        importDlsText_.setPosition({20, Config::TopMenuHeight + 150});

        newText_.setFillColor(isNewHovered_ ? Theme::Highlight : Theme::Text);
        saveText_.setFillColor(isSaveHovered_ ? Theme::Highlight : Theme::Text);
        loadText_.setFillColor(isLoadHovered_ ? Theme::Highlight : Theme::Text);
        saveAsText_.setFillColor(isSaveAsHovered_ ? Theme::Highlight : Theme::Text);
        importDlsText_.setFillColor(isImportDlsHovered_ ? Theme::Highlight : Theme::Text);

        window.draw(newText_);
        window.draw(saveText_);
        window.draw(loadText_);
        window.draw(saveAsText_);
        window.draw(importDlsText_);
    }
}

void WorkbenchUI::drawPalette(sf::RenderWindow& window) {
    window.draw(paletteRect_);
}

void WorkbenchUI::drawModal(sf::RenderWindow& window, bool isSaveModal, bool isLoadModal, float modalAlpha) {
    if (isSaveModal || isLoadModal || modalAlpha > 0) {
        sf::Color overlayColor = overlay_.getFillColor();
        std::uint8_t overlayAlpha = static_cast<std::uint8_t>(modalAlpha * 0.8f);
        overlay_.setFillColor(sf::Color(overlayColor.r, overlayColor.g, overlayColor.b, overlayAlpha));
        window.draw(overlay_);

        sf::Color modalColor = modalRect_.getFillColor();
        std::uint8_t modalAlphaUint8 = static_cast<std::uint8_t>(modalAlpha);
        modalRect_.setFillColor(sf::Color(modalColor.r, modalColor.g, modalColor.b, modalAlphaUint8));
        window.draw(modalRect_);

        modalTitle_.setPosition({ModalX + 20, ModalY + 15});
        modalTitle_.setString(isSaveModal ? "Salvar Projeto" : "Carregar Projeto");
        std::uint8_t titleAlpha = static_cast<std::uint8_t>(modalAlpha);
        modalTitle_.setFillColor(sf::Color(Theme::Highlight.r, Theme::Highlight.g, Theme::Highlight.b, titleAlpha));
        window.draw(modalTitle_);
    }
}

void WorkbenchUI::drawTooltip(sf::RenderWindow& window, const sf::Vector2f& mousePos) {
    if (!currentTooltip_.empty() && tooltipTimer_ >= TooltipDelay) {
        tooltipText_.setString(currentTooltip_);
        sf::FloatRect textBounds = tooltipText_.getGlobalBounds();
        float padding = 10.0f;
        tooltipBg_.setSize({textBounds.size.x + padding * 2, textBounds.size.y + padding * 2});
        tooltipBg_.setPosition({mousePos.x + 15, mousePos.y + 15});
        tooltipText_.setPosition({mousePos.x + 15 + padding, mousePos.y + 15 + padding});
        window.draw(tooltipBg_);
        window.draw(tooltipText_);
    }
}

void WorkbenchUI::drawProgressBar(sf::RenderWindow& window, bool isSaving, bool isLoading, float progress) {
    if (isSaving || isLoading) {
        float prog = isSaving ? progress : 0.0f;
        std::string statusText = isSaving ? "Salvando..." : "Carregando...";

        sf::RectangleShape progressBg;
        progressBg.setSize({300, 30});
        progressBg.setPosition({450, 400});
        progressBg.setFillColor(Theme::ProgressBg);
        progressBg.setOutlineThickness(2);
        progressBg.setOutlineColor(Theme::ProgressBorder);
        window.draw(progressBg);

        progressBar_.setSize({296 * prog, 26});
        progressBar_.setPosition({452, 402});
        window.draw(progressBar_);

        progressText_.setString(statusText + " " + std::to_string(int(prog * 100)) + "%");
        progressText_.setPosition({500, 405});
        progressText_.setFillColor(Theme::Text);
        window.draw(progressText_);
    }
}

void WorkbenchUI::drawStatusMessage(sf::RenderWindow& window, const std::string& message, float timer) {
    if (timer > 0.0f && timer < 5.0f) {
        sf::Text status(font_);
        status.setFont(font_);
        status.setString(message);
        status.setCharacterSize(13);
        status.setFillColor(Theme::Highlight);
        status.setPosition({180, 770});
        window.draw(status);
    }
}

void WorkbenchUI::drawConfirmationDialog(sf::RenderWindow& window, bool show, const std::string& message) {
    if (show) {
        sf::RectangleShape confirmOverlay(sf::Vector2f(1200, 800));
        confirmOverlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(confirmOverlay);

        sf::RectangleShape confirmDialog(sf::Vector2f(400, 200));
        confirmDialog.setPosition({400, 300});
        confirmDialog.setFillColor(Theme::ModalBg);
        confirmDialog.setOutlineThickness(3);
        confirmDialog.setOutlineColor(Theme::Highlight);
        window.draw(confirmDialog);

        sf::Text confirmTitle(font_);
        confirmTitle.setFont(font_);
        confirmTitle.setString("Confirmação");
        confirmTitle.setCharacterSize(20);
        confirmTitle.setFillColor(Theme::Highlight);
        confirmTitle.setPosition({550, 320});
        window.draw(confirmTitle);

        sf::Text confirmText(font_);
        confirmText.setFont(font_);
        confirmText.setString(message);
        confirmText.setCharacterSize(14);
        confirmText.setFillColor(Theme::Text);
        confirmText.setPosition({420, 360});
        window.draw(confirmText);

        sf::FloatRect yesBtn({420, 430}, {120, 35});
        sf::RectangleShape yesBtnShape(yesBtn.size);
        yesBtnShape.setPosition(yesBtn.position);
        yesBtnShape.setFillColor(Theme::ButtonActive);
        window.draw(yesBtnShape);
        sf::Text yesText(font_);
        yesText.setFont(font_);
        yesText.setString("Sim");
        yesText.setCharacterSize(14);
        yesText.setFillColor(sf::Color::White);
        yesText.setPosition({yesBtn.position.x + 40, yesBtn.position.y + 8});
        window.draw(yesText);

        sf::FloatRect noBtn({560, 430}, {120, 35});
        sf::RectangleShape noBtnShape(noBtn.size);
        noBtnShape.setPosition(noBtn.position);
        noBtnShape.setFillColor(Theme::ButtonNormal);
        noBtnShape.setOutlineThickness(1);
        noBtnShape.setOutlineColor(Theme::Accent);
        window.draw(noBtnShape);
        sf::Text noText(font_);
        noText.setFont(font_);
        noText.setString("Não");
        noText.setCharacterSize(14);
        noText.setFillColor(Theme::Text);
        noText.setPosition({noBtn.position.x + 40, noBtn.position.y + 8});
        window.draw(noText);
    }
}

void WorkbenchUI::drawHelpOverlay(sf::RenderWindow& window, bool show, const sf::Font& font) {
    if (!show) return;

    sf::RectangleShape overlay(sf::Vector2f(1200, 800));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(overlay);

    sf::RectangleShape helpPanel(sf::Vector2f(800, 600));
    helpPanel.setPosition({200, 100});
    helpPanel.setFillColor(Theme::ModalBg);
    helpPanel.setOutlineThickness(3);
    helpPanel.setOutlineColor(Theme::Highlight);
    window.draw(helpPanel);

    sf::Text helpTitle(font);
    helpTitle.setFont(font);
    helpTitle.setString("ATALHOS DE TECLADO E AJUDA");
    helpTitle.setCharacterSize(24);
    helpTitle.setFillColor(Theme::Highlight);
    helpTitle.setPosition({420, 120});
    window.draw(helpTitle);

    sf::Text helpText(font);
    helpText.setFont(font);
    helpText.setCharacterSize(16);
    helpText.setFillColor(Theme::Text);

    std::string leftCol = 
        "ARQUIVO:\n"
        "  Ctrl+N     - Novo projeto\n"
        "  Ctrl+S     - Salvar projeto\n"
        "  Ctrl+L     - Carregar projeto\n"
        "  Delete     - Deletar componentes selecionados\n"
        "  Escape     - Fechar modais/cancelar ações\n\n"
        "EDIÇÃO:\n"
        "  Ctrl+Z     - Desfazer última ação\n"
        "  Ctrl+Y     - Refazer ação desfeita\n"
        "  Ctrl+D     - Duplicar componentes selecionados\n"
        "  Ctrl+A     - Selecionar todos os componentes\n\n"
        "VISUALIZAÇÃO:\n"
        "  R          - Rotacionar componente selecionado\n"
        "  G          - Alternar snap-to-grid\n"
        "  F          - Centralizar na seleção\n"
        "  Setas      - Mover componente (pixel por pixel)";

    helpText.setString(leftCol);
    helpText.setPosition({220, 170});
    window.draw(helpText);

    std::string rightCol = 
        "ACESSIBILIDADE:\n"
        "  F1         - Alternar tema (Escuro/Claro/Alto Contraste)\n"
        "  F2         - Alternar tamanho de fonte\n"
        "  F3         - Alternar alto contraste\n"
        "  F4         - Alternar modo performance\n\n"
        "DICAS:\n"
        "  • Shift+Clique para seleção múltipla\n"
        "  • Arraste componentes da paleta para criar\n"
        "  • Clique em pinos para conectar fios\n"
        "  • Botão direito cancela ações\n"
        "  • Backups automáticos a cada 5 minutos\n\n"
        "GRUPOS:\n"
        "  Ctrl+G     - Agrupar componentes selecionados\n"
        "  Ctrl+U     - Desagrupar grupo selecionado\n\n"
        "FILTROS:\n"
        "  Ctrl+T     - Alternar filtro de seleção";

    helpText.setString(rightCol);
    helpText.setPosition({580, 170});
    window.draw(helpText);

    sf::Text closeText(font);
    closeText.setFont(font);
    closeText.setString("Pressione F1 ou / para fechar esta ajuda");
    closeText.setCharacterSize(14);
    closeText.setFillColor(sf::Color(200, 200, 200));
    closeText.setPosition({450, 680});
    window.draw(closeText);
}

void WorkbenchUI::drawFPS(sf::RenderWindow& window, bool show, float fps, bool performanceMode) {
    if (!show) return;

    sf::Text fpsText(font_);
    fpsText.setFont(font_);
    fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
    fpsText.setCharacterSize(14);
    fpsText.setFillColor(performanceMode ? sf::Color(255, 255, 0) : sf::Color(100, 255, 100));
    fpsText.setPosition({1100, 10});
    window.draw(fpsText);

    if (performanceMode) {
        sf::Text perfText(font_);
        perfText.setFont(font_);
        perfText.setString("PERFORMANCE MODE");
        perfText.setCharacterSize(12);
        perfText.setFillColor(sf::Color(255, 200, 0));
        perfText.setPosition({1100, 30});
        window.draw(perfText);
    }
}

void WorkbenchUI::updateMenuAnimation(bool isHovered, bool isPressed, float deltaTime) {
    float transitionSpeed = 10.0f * deltaTime;

    if (isHovered || isPressed) {
        menuButtonAlpha_ = std::min(255.0f, menuButtonAlpha_ + 255.0f * transitionSpeed);
    } else {
        menuButtonAlpha_ = std::max(200.0f, menuButtonAlpha_ - 255.0f * transitionSpeed);
    }
}

void WorkbenchUI::updateDropdownAnimation(bool show, float deltaTime) {
    float transitionSpeed = 10.0f * deltaTime;

    if (show) {
        dropdownAlpha_ = std::min(255.0f, dropdownAlpha_ + 255.0f * transitionSpeed);
    } else {
        dropdownAlpha_ = std::max(0.0f, dropdownAlpha_ - 255.0f * transitionSpeed);
    }
}

void WorkbenchUI::updateModalAnimation(bool show, float deltaTime) {
    float transitionSpeed = 10.0f * deltaTime;

    if (show) {
        modalAlpha_ = std::min(255.0f, modalAlpha_ + 255.0f * transitionSpeed);
    } else {
        modalAlpha_ = std::max(0.0f, modalAlpha_ - 255.0f * transitionSpeed);
    }
}

void WorkbenchUI::updateTooltip(const std::string& text, float deltaTime) {
    if (!text.empty()) {
        tooltipTimer_ += deltaTime;
    } else {
        tooltipTimer_ = 0.0f;
    }
}

void WorkbenchUI::updateStatusMessage(float deltaTime, float& timer) {
    if (timer > 0.0f) {
        timer -= deltaTime;
        if (timer < 0.0f) timer = 0.0f;
    }
}

sf::FloatRect WorkbenchUI::chipRect(size_t i, float startY) {
    size_t col = i % 4;
    size_t row = i / 4;
    float x = ModalX + 20 + static_cast<float>(col) * (ChipW + ChipGap);
    float y = startY + static_cast<float>(row) * (ChipH + 6);
    return sf::FloatRect({x, y}, {ChipW, ChipH});
}

sf::FloatRect WorkbenchUI::nameInputRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 68}, {ModalW - 40, 32});
}

sf::FloatRect WorkbenchUI::categoryInputRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 130}, {ModalW - 40, 32});
}

sf::FloatRect WorkbenchUI::descriptionInputRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 200}, {ModalW - 40, 32});
}

sf::FloatRect WorkbenchUI::tagsInputRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 260}, {ModalW - 40, 32});
}

sf::FloatRect WorkbenchUI::saveButtonRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 320}, {200, 30});
}

sf::FloatRect WorkbenchUI::exportJsonRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 358}, {200, 28});
}

sf::FloatRect WorkbenchUI::exportXmlRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 392}, {200, 28});
}

sf::FloatRect WorkbenchUI::loadRowRect(size_t i) const {
    static constexpr float LoadListBaseY = ModalY + 140.f;
    static constexpr float LoadRowH = 40.f;
    static constexpr int LoadVisibleRows = 6;
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i)) * LoadRowH;
    return sf::FloatRect({ModalX + 15, y}, {ModalW - 30, LoadRowH - 4});
}

sf::FloatRect WorkbenchUI::loadRowLoadBtnRect(size_t i) const {
    static constexpr float LoadListBaseY = ModalY + 140.f;
    static constexpr float LoadRowH = 40.f;
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i)) * LoadRowH;
    return sf::FloatRect({ModalX + ModalW - 160, y + 7}, {65, 26});
}

sf::FloatRect WorkbenchUI::loadRowDeleteBtnRect(size_t i) const {
    static constexpr float LoadListBaseY = ModalY + 140.f;
    static constexpr float LoadRowH = 40.f;
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i)) * LoadRowH;
    return sf::FloatRect({ModalX + ModalW - 85, y + 7}, {65, 26});
}

sf::FloatRect WorkbenchUI::loadSearchRect() const {
    return sf::FloatRect({ModalX + 20, ModalY + 100}, {ModalW - 160, 28});
}

sf::FloatRect WorkbenchUI::loadSortBtnRect() const {
    return sf::FloatRect({ModalX + ModalW - 140, ModalY + 100}, {120, 28});
}

sf::FloatRect WorkbenchUI::loadStarRect(size_t i) const {
    static constexpr float LoadListBaseY = ModalY + 140.f;
    static constexpr float LoadRowH = 40.f;
    float y = LoadListBaseY + static_cast<float>(static_cast<int>(i)) * LoadRowH;
    return sf::FloatRect({ModalX + ModalW - 230, y + 7}, {24, 26});
}

sf::FloatRect WorkbenchUI::loadDetailPanelRect() const {
    return sf::FloatRect({ModalX + 15, ModalY + ModalH - 120}, {ModalW - 30, 90});
}

sf::FloatRect WorkbenchUI::loadSaveMetaBtnRect() const {
    return sf::FloatRect({ModalX + ModalW - 140, ModalY + ModalH - 35}, {120, 26});
}

sf::FloatRect WorkbenchUI::importResultRect() {
    return sf::FloatRect({ImportResultX, ImportResultY}, {ImportResultW, ImportResultH});
}

sf::FloatRect WorkbenchUI::importResultCloseBtn() {
    return sf::FloatRect({ImportResultX + ImportResultW - 120, ImportResultY + ImportResultH - 45}, {100, 30});
}
