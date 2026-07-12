#ifndef CONFIRMATION_DIALOG_HPP
#define CONFIRMATION_DIALOG_HPP

#include <string>
#include <functional>
#include <SFML/Graphics.hpp>

class ConfirmationDialog {
public:
    enum class Action {
        NONE,
        DELETE_PROJECT,
        NEW_PROJECT,
        DELETE_COMPONENTS,
        CLOSE_WITHOUT_SAVE,
        CUSTOM
    };

    ConfirmationDialog();
    ~ConfirmationDialog() = default;

    void show(Action action, const std::string& message, 
              std::function<void()> onConfirm = nullptr,
              std::function<void()> onCancel = nullptr);

    void hide();
    bool isVisible() const;
    void handleMouseClick(float mouseX, float mouseY);
    void handleKeyPress(int keyCode);
    void render(sf::RenderWindow& window, const sf::Font& font);
    Action getPendingAction() const;
    int getProjectIndex() const;
    void setProjectIndex(int index);

private:
    bool visible;
    Action pendingAction;
    std::string message;
    int projectIndex;
    std::function<void()> confirmCallback;
    std::function<void()> cancelCallback;

    // Layout geometry
    static constexpr float DialogWidth = 400.0f;
    static constexpr float DialogHeight = 200.0f;
    static constexpr float DialogX = 400.0f;
    static constexpr float DialogY = 300.0f;
    static constexpr float ButtonWidth = 120.0f;
    static constexpr float ButtonHeight = 35.0f;
    static constexpr float YesButtonX = 420.0f;
    static constexpr float NoButtonX = 560.0f;
    static constexpr float ButtonY = 430.0f;
};

#endif // CONFIRMATION_DIALOG_HPP