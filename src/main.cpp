#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "Workbench.hpp"
#include "Constants.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Logic Gate Simulator");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Could not load font arial.ttf!" << std::endl;
        return -1;
    }

    Workbench workbench(font);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Events are now fully handled in workbench
            workbench.handleEvent(*event, window);
        }

        // Logic propagation loop
        workbench.updatePropagation();

        window.clear(Theme::Background);
        
        workbench.draw(window);

        window.display();
    }

    return 0;
}
