#include <windows.h>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <string>
#include "Workbench.h"
#include "Constants.h"
#include "tests/TestFramework.h"

int main(int argc, char* argv[]) {
    // Processamento de flags de linha de comando
    bool runTestsCli = false;
    std::string testFilter = "";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test" || arg == "-t" || arg == "/test") {
            runTestsCli = true;
        } else if (arg.rfind("--filter=", 0) == 0) {
            testFilter = arg.substr(9);
        } else if (!arg.empty() && arg[0] != '-') {
            testFilter = arg;
        }
    }

    if (runTestsCli) {
        int testResult = test::TestRunner::runAll(std::cout, testFilter);
        std::cout.flush();
        std::cerr.flush();
        return testResult;
    }
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
