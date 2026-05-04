#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <optional>
#include "world.h"

void worker();

int main() {
    // Проверка сканирования карт
    scanMapDirectory();
    if (map_filenames.empty()) {
        std::cerr << "CRITICAL: No maps found in 'maps/' folder!" << std::endl;
    } else {
        std::cout << "Loaded " << map_filenames.size() << " maps." << std::endl;
    }

    resetWorld();

    sf::RenderWindow window(sf::VideoMode({1050, 600}), "Warehouse MAPF SFML 3");
    window.setFramerateLimit(60);

    sf::Font font;
    // SFML 3: Метод называется openFromFile
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "CRITICAL: Could not load arial.ttf! Place it next to the executable." << std::endl;
    }

    std::thread t{worker};
    t.detach();

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* mB = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mPos = {(float)mB->position.x, (float)mB->position.y};

                // Клики по списку карт
                for (int i = 0; i < (int)map_filenames.size(); i++) {
                    if (sf::FloatRect({810.f, 60.f + i * 40.f}, {230.f, 30.f}).contains(mPos)) {
                        selected_map_idx = i;
                        resetWorld();
                        std::cout << "Switched to map: " << map_filenames[i] << std::endl;
                    }
                }

                // Клики по алгоритмам
                for (int i = 0; i < (int)algo_names.size(); i++) {
                    if (sf::FloatRect({810.f, 400.f + i * 50.f}, {230.f, 40.f}).contains(mPos)) {
                        current_algo = (Algorithm)i;
                        simulationStarted = true;
                        std::cout << "Started simulation with: " << algo_names[i] << std::endl;
                    }
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));

        // 1. Отрисовка сетки
        for (int y = 0; y < (int)grid.size(); y++) {
            for (int x = 0; x < (int)grid[y].size(); x++) {
                sf::RectangleShape tile({38.f, 38.f});
                tile.setPosition({x * 40.f, y * 40.f});
                tile.setFillColor(grid[y][x] == '#' ? sf::Color(60, 60, 60) : sf::Color(40, 40, 40));
                window.draw(tile);
            }
        }

        // 2. Отрисовка баз (Delivery Points)
        for (auto& b : deliveryPoints) {
            sf::RectangleShape r({34.f, 34.f});
            r.setPosition({b.pos.x * 40.f + 3.f, b.pos.y * 40.f + 3.f});
            if (b.ownerId < (int)robots.size()) {
                sf::Color c = robots[b.ownerId].color;
                r.setOutlineColor(c);
                r.setOutlineThickness(2);
                r.setFillColor(sf::Color(c.r, c.g, c.b, 60));
            }
            window.draw(r);
        }

        // 3. Отрисовка роботов
        for (auto &r : robots) {
            sf::Vector2f targetPos({(float)r.gridPos.x * 40.f + 20.f, (float)r.gridPos.y * 40.f + 20.f});
            // Увеличиваем множитель до 10.0f для резвого перемещения[cite: 5]
            r.realPos += (targetPos - r.realPos) * dt * 10.0f;

            sf::CircleShape rs(14.f);
            rs.setOrigin({14.f, 14.f});
            rs.setPosition(r.realPos);
            rs.setFillColor(r.color);
            window.draw(rs);
        }

        // 4. Отрисовка объектов (коробок)
        for (auto &o : objects) {
            if (o.delivered) continue;
            sf::RectangleShape box({18.f, 18.f});
            box.setOrigin({9.f, 9.f});
            if (o.carrierId != -1) box.setPosition(robots[o.carrierId].realPos);
            else box.setPosition({o.pos.x * 40.f + 20.f, o.pos.y * 40.f + 20.f});
            box.setFillColor(sf::Color::Yellow);
            window.draw(box);
        }

        // 5. Правая панель
        sf::RectangleShape mBg({250.f, 600.f});
        mBg.setPosition({800.f, 0.f});
        mBg.setFillColor(sf::Color(50, 50, 50));
        window.draw(mBg);

        // 6. Текст (Интерфейс)
        sf::Text uiText(font);
        uiText.setCharacterSize(18);

        // Список карт
        uiText.setString("SELECT MAP:");
        uiText.setPosition({810.f, 20.f});
        uiText.setFillColor(sf::Color::Yellow);
        window.draw(uiText);

        for (int i = 0; i < (int)map_filenames.size(); i++) {
            std::string name = map_filenames[i];
            uiText.setString(name.substr(name.find_last_of("/\\") + 1));
            uiText.setPosition({810.f, 60.f + i * 40.f});
            uiText.setFillColor(i == selected_map_idx ? sf::Color::Green : sf::Color::White);
            window.draw(uiText);
        }

        // Список алгоритмов
        uiText.setString("ALGORITHMS:");
        uiText.setPosition({810.f, 360.f});
        uiText.setFillColor(sf::Color::Yellow);
        window.draw(uiText);

        for (int i = 0; i < (int)algo_names.size(); i++) {
            uiText.setString(algo_names[i]);
            uiText.setPosition({810.f, 400.f + i * 50.f});
            uiText.setFillColor((int)current_algo == i && simulationStarted ? sf::Color::Green : sf::Color::White);
            window.draw(uiText);
        }

        window.display();
    }
    return 0;
}