#include <windows.h>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <string>
#include "Workbench.h"
#include "Constants.h"

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Logic Gate Simulator");
    window.setFramerateLimit(60);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Initialize ImGui backends
    sf::WindowHandle hwnd = window.getNativeHandle();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    sf::Font font;
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Could not load font arial.ttf!" << std::endl;
        return -1;
    }

    Workbench workbench(font);

    while (window.isOpen()) {
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            workbench.handleEvent(*event, window);
        }

        // Update animations and UI
        static sf::Clock clock;
        float deltaTime = clock.restart().asSeconds();
        workbench.update(deltaTime);
        
        // Logic propagation loop
        workbench.updatePropagation();
        workbench.renderImGui();

        window.clear(Theme::Background);
        workbench.draw(window);

        // Render ImGui on top
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
