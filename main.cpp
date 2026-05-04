#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include "world.h"

void worker();

int main() {
    scanMapDirectory();
    resetWorld();

    sf::RenderWindow window(sf::VideoMode({1050, 600}), "BFS vs A-Star");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Error: Could not load arial.ttf" << std::endl;
    }

    std::thread t(worker);
    t.detach();
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* mB = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mPos = {(float)mB->position.x, (float)mB->position.y};

                // Клик по картам
                for (int i = 0; i < (int)map_filenames.size(); i++) {
                    if (sf::FloatRect({810.f, 50.f + i*50.f}, {230.f, 40.f}).contains(mPos)) {
                        selected_map_idx = i;
                        resetWorld();
                    }
                }
                // Клик по алгоритмам
                for (int i = 0; i < (int)algo_names.size(); i++) {
                    if (sf::FloatRect({810.f, 400.f + i*50.f}, {230.f, 40.f}).contains(mPos)) {
                        current_algo = (Algorithm)i;
                        simulationStarted = true;
                    }
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));

        // Отрисовка сетки
        for (int y = 0; y < (int)grid.size(); y++) {
            for (int x = 0; x < (int)grid[y].size(); x++) {
                sf::RectangleShape tile({39.f, 39.f});
                tile.setPosition({(float)x*40.f, (float)y*40.f});
                tile.setFillColor(grid[y][x] == '#' ? sf::Color(70, 70, 70) : sf::Color(45, 45, 45));
                window.draw(tile);
            }
        }

        // Отрисовка баз
        for (auto& b : deliveryPoints) {
            sf::RectangleShape s({32.f, 32.f});
            s.setPosition({(float)b.x*40.f + 4.f, (float)b.y*40.f + 4.f});
            s.setOutlineColor(sf::Color::Cyan);
            s.setOutlineThickness(2);
            s.setFillColor(sf::Color(0, 255, 255, 50));
            window.draw(s);
        }

        // Отрисовка роботов
        for (auto &r : robots) {
            sf::Vector2f target((float)r.gridPos.x*40.f + 20.f, (float)r.gridPos.y*40.f + 20.f);
            r.realPos += (target - r.realPos) * dt * 8.0f;
            sf::CircleShape rs(14.f);
            rs.setOrigin({14.f, 14.f});
            rs.setPosition(r.realPos);
            rs.setFillColor(sf::Color::Cyan);
            window.draw(rs);
        }

        // Отрисовка коробок
        for (auto &obj : objects) {
            if (obj.delivered) continue;
            sf::RectangleShape o({18.f, 18.f});
            o.setOrigin({9.f, 9.f});
            if (obj.carrierId != -1) {
                for (auto& r : robots) if (r.id == obj.carrierId) o.setPosition(r.realPos);
            }
            else {
                o.setPosition({(float)obj.pos.x*40.f + 20.f, (float)obj.pos.y*40.f + 20.f});
            }
            o.setFillColor(sf::Color::Yellow);
            window.draw(o);
        }

        // Правое меню
        sf::RectangleShape mBg({250.f, 600.f});
        mBg.setPosition({800.f, 0.f});
        mBg.setFillColor(sf::Color(50, 50, 50));
        window.draw(mBg);

        // Список карт (названия)
        for (int i = 0; i < (int)map_filenames.size(); i++) {
            sf::Text mt(font, map_filenames[i].substr(map_filenames[i].find_last_of("/\\") + 1), 14);
            mt.setPosition({820.f, 60.f + i*50.f});
            window.draw(mt);
        }

        // Кнопки алгоритмов
        for (int i = 0; i < (int)algo_names.size(); i++) {
            sf::RectangleShape btn({230.f, 40.f});
            btn.setPosition({810.f, 400.f + i*50.f});
            btn.setFillColor((int)current_algo == i ? sf::Color(255, 140, 0) : sf::Color(70, 70, 70));
            window.draw(btn);

            sf::Text t(font, algo_names[i], 16);
            t.setPosition({820.f, 410.f + i*50.f});
            window.draw(t);
        }

        window.display();
    }
    return 0;
}