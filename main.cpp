#include <SFML/Graphics.hpp>
#include <thread>
#include "world.h"

void worker();

const int TILE_SIZE = 40;
const float ROBOT_SPEED = 6.0f;

int main() {
    // Инициализация сетки и объектов[cite: 1]
    for (int y = 0; y < (int)grid.size(); y++) {
        for (int x = 0; x < (int)grid[y].size(); x++) {
            if (grid[y][x] == 'A') baseA = {x, y};
            if (grid[y][x] == 'B') baseB = {x, y};
            if (grid[y][x] == 'O') {
                objects.push_back({{x, y}, -1, false});
                grid[y][x] = '.'; // Очищаем клетку для прохода роботов
            }
        }
    }

    // Инициализация роботов[cite: 1]
    robots.push_back({0, {40, 40}, {1, 1}, -1, false, 0});
    robots.push_back({1, {80, 40}, {2, 1}, -1, false, 0});
    robots.push_back({2, {40, 320}, {1, 8}, -1, false, 1});
    robots.push_back({3, {80, 320}, {2, 8}, -1, false, 1});

    std::thread t(worker);
    t.detach();

    sf::RenderWindow window(
            sf::VideoMode({(unsigned)grid[0].size() * TILE_SIZE, (unsigned)grid.size() * TILE_SIZE}),
            "Robot Warehouse"
    );
    window.setFramerateLimit(60);
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>()) window.close();
        }

        window.clear(sf::Color(25, 25, 25));

        // Отрисовка пола и стен[cite: 1]
        for (int y = 0; y < (int)grid.size(); y++) {
            for (int x = 0; x < (int)grid[0].size(); x++) {
                sf::RectangleShape tile({TILE_SIZE - 1.f, TILE_SIZE - 1.f});
                tile.setPosition({x * TILE_SIZE * 1.f, y * TILE_SIZE * 1.f});
                tile.setFillColor(grid[y][x] == '#' ? sf::Color(60, 60, 60) : sf::Color(40, 40, 40));
                window.draw(tile);
            }
        }

        // Отрисовка баз
        auto drawBase = [&](Pos p, sf::Color color) {
            sf::RectangleShape b({TILE_SIZE - 8.f, TILE_SIZE - 8.f});
            b.setPosition({(float)p.x * TILE_SIZE + 4.f, (float)p.y * TILE_SIZE + 4.f});
            b.setFillColor(sf::Color(color.r, color.g, color.b, 40));
            b.setOutlineThickness(2);
            b.setOutlineColor(color);
            window.draw(b);
        };
        drawBase(baseA, sf::Color::Cyan);
        drawBase(baseB, sf::Color::Magenta);

        // Отрисовка свободных коробок
        for (auto &obj : objects) {
            if (obj.delivered || obj.carrierId != -1) continue;
            sf::RectangleShape oShape({18, 18});
            oShape.setOrigin({9, 9});
            oShape.setPosition({(float)obj.pos.x * TILE_SIZE + 20, (float)obj.pos.y * TILE_SIZE + 20});
            oShape.setFillColor(sf::Color::Yellow);
            window.draw(oShape);
        }

        // Отрисовка роботов и груза[cite: 1]
        for (auto &r : robots) {
            sf::Vector2f target(r.gridPos.x * TILE_SIZE + 20.f, r.gridPos.y * TILE_SIZE + 20.f);
            r.realPos += (target - r.realPos) * dt * ROBOT_SPEED;

            sf::CircleShape rShape(14);
            rShape.setOrigin({14, 14});
            rShape.setPosition(r.realPos);
            rShape.setFillColor(r.team ? sf::Color::Magenta : sf::Color::Cyan);
            window.draw(rShape);

            if (r.hasObject) { // Отрисовка коробки на роботе
                sf::RectangleShape cargo({12, 12});
                cargo.setOrigin({6, 6});
                cargo.setPosition(r.realPos);
                cargo.setFillColor(sf::Color::Yellow);
                window.draw(cargo);
            }
        }
        window.display();
    }
}