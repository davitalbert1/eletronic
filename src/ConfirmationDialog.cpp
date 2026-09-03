#include "ConfirmationDialog.h"
#include <SFML/Graphics.hpp>

ConfirmationDialog::ConfirmationDialog()
    : visible(false)
    , pendingAction(Action::NONE)
    , projectIndex(-1) {
}

void ConfirmationDialog::show(Action action, const std::string& message,
                              std::function<void()> onConfirm,
                              std::function<void()> onCancel) {
    pendingAction = action;
    this->message = message;
    confirmCallback = onConfirm;
    cancelCallback = onCancel;
    visible = true;
}

void ConfirmationDialog::hide() {
    visible = false;
    pendingAction = Action::NONE;
    projectIndex = -1;
    confirmCallback = nullptr;
    cancelCallback = nullptr;
}

bool ConfirmationDialog::isVisible() const {
    return visible;
}

void ConfirmationDialog::handleMouseClick(float mouseX, float mouseY) {
    if (!visible) return;

    sf::FloatRect yesBtn(sf::Vector2f(YesButtonX, ButtonY), sf::Vector2f(ButtonWidth, ButtonHeight));
    sf::FloatRect noBtn(sf::Vector2f(NoButtonX, ButtonY), sf::Vector2f(ButtonWidth, ButtonHeight));

    if (yesBtn.contains(sf::Vector2f(mouseX, mouseY))) {
        if (confirmCallback) confirmCallback();
        hide();
    } else if (noBtn.contains(sf::Vector2f(mouseX, mouseY))) {
        if (cancelCallback) cancelCallback();
        hide();
    }
}

void ConfirmationDialog::handleKeyPress(int keyCode) {
    if (!visible) return;

    if (keyCode == static_cast<int>(sf::Keyboard::Key::Escape)) {
        if (cancelCallback) cancelCallback();
        hide();
    } else if (keyCode == static_cast<int>(sf::Keyboard::Key::Enter)) {
        if (confirmCallback) confirmCallback();
        hide();
    }
}

void ConfirmationDialog::render(sf::RenderWindow& window, const sf::Font& font) {
    if (!visible) return;

    // Overlay
    sf::RectangleShape overlay(sf::Vector2f(1200, 800));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    // Dialog box
    sf::RectangleShape dialog(sf::Vector2f(DialogWidth, DialogHeight));
    dialog.setPosition({DialogX, DialogY});
    dialog.setFillColor(sf::Color(45, 45, 55));
    dialog.setOutlineThickness(3);
    dialog.setOutlineColor(sf::Color(100, 180, 255));
    window.draw(dialog);

    // Title
    sf::Text title(font);
    title.setFont(font);
    title.setString("Confirmação");
    title.setCharacterSize(20);
    title.setFillColor(sf::Color(100, 180, 255));
    title.setPosition({DialogX + 140, DialogY + 20});
    window.draw(title);

    // Message
    sf::Text msgText(font);
    msgText.setFont(font);
    msgText.setString(message);
    msgText.setCharacterSize(14);
    msgText.setFillColor(sf::Color(220, 220, 220));
    msgText.setPosition({DialogX + 20, DialogY + 60});
    window.draw(msgText);

    // Yes button
    sf::RectangleShape yesBtn(sf::Vector2f(ButtonWidth, ButtonHeight));
    yesBtn.setPosition({YesButtonX, ButtonY});
    yesBtn.setFillColor(sf::Color(200, 80, 80));
    yesBtn.setOutlineThickness(2);
    yesBtn.setOutlineColor(sf::Color(255, 100, 100));
    window.draw(yesBtn);

    sf::Text yesText(font);
    yesText.setFont(font);
    yesText.setString("Sim");
    yesText.setCharacterSize(14);
    yesText.setFillColor(sf::Color::White);
    sf::FloatRect yesBounds = yesText.getLocalBounds();
    yesText.setPosition({YesButtonX + (ButtonWidth - yesBounds.size.x) / 2,
                         ButtonY + 8});
    window.draw(yesText);

    // No button
    sf::RectangleShape noBtn(sf::Vector2f(ButtonWidth, ButtonHeight));
    noBtn.setPosition({NoButtonX, ButtonY});
    noBtn.setFillColor(sf::Color(60, 60, 70));
    noBtn.setOutlineThickness(2);
    noBtn.setOutlineColor(sf::Color(100, 180, 255));
    window.draw(noBtn);

    sf::Text noText(font);
    noText.setFont(font);
    noText.setString("Não");
    noText.setCharacterSize(14);
    noText.setFillColor(sf::Color::White);
    sf::FloatRect noBounds = noText.getLocalBounds();
    noText.setPosition({NoButtonX + (ButtonWidth - noBounds.size.x) / 2,
                        ButtonY + 8});
    window.draw(noText);
}

ConfirmationDialog::Action ConfirmationDialog::getPendingAction() const {
    return pendingAction;
}

int ConfirmationDialog::getProjectIndex() const {
    return projectIndex;
}

void ConfirmationDialog::setProjectIndex(int index) {
    projectIndex = index;
}