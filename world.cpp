#include "world.h"
#include "pathfinding.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>

std::vector<std::string> grid;
std::vector<std::string> map_filenames;
std::vector<Object> objects;
std::vector<Robot> robots;
std::vector<DeliveryPoint> deliveryPoints;
std::set<std::tuple<int, int, int>> reserved;
Algorithm current_algo = Algorithm::ASTAR;
std::vector<std::string> algo_names = {"Dijkstra", "A-Star"};
int selected_map_idx = 0;
bool simulationStarted = false;

sf::Color getNiceColor(int i) {
    sf::Color colors[] = {sf::Color::Cyan, sf::Color::Magenta, sf::Color::Green, sf::Color::Yellow, sf::Color(255, 165, 0), sf::Color(128, 0, 255)};
    return colors[i % 6];
}

bool is_free(int x, int y) {
    if (grid.empty()) return false;
    if (y < 0 || y >= (int)grid.size() || x < 0 || x >= (int)grid[0].size()) return false;
    return grid[y][x] != '#';
}

void scanMapDirectory() {
    map_filenames.clear();
    if (!std::filesystem::exists("maps")) std::filesystem::create_directory("maps");
    for (const auto& entry : std::filesystem::directory_iterator("maps")) {
        if (entry.path().extension() == ".txt") map_filenames.push_back(entry.path().string());
    }
    std::sort(map_filenames.begin(), map_filenames.end());
}

void resetWorld() {
    if (map_filenames.empty()) return;
    std::ifstream file(map_filenames[selected_map_idx]);
    grid.clear(); objects.clear(); robots.clear(); deliveryPoints.clear();
    simulationStarted = false;
    std::string line;
    while (std::getline(file, line)) if (!line.empty()) grid.push_back(line);

    int rCount = 0;
    for (int y = 0; y < (int)grid.size(); y++) {
        for (int x = 0; x < (int)grid[y].size(); x++) {
            char c = grid[y][x];
            if (c >= 'A' && c <= 'Z' && c != 'R' && c != 'O') deliveryPoints.push_back({{x, y}, rCount++ % 10});
            else if (c == 'O') { objects.push_back({{x, y}, -1, -1, false}); grid[y][x] = '.'; }
            else if (c == 'R') {
                robots.push_back({ (int)robots.size(), {(float)x * 40 + 20, (float)y * 40 + 20}, {x, y}, -1, false, {}, getNiceColor((int)robots.size()) });
                grid[y][x] = '.';
            }
        }
    }
}

void updateLogic() {
    if (!simulationStarted) return;

    reserved.clear();

    for (auto &r : robots) {
        sf::Vector2f targetRealPos((float)r.gridPos.x * 40 + 20, (float)r.gridPos.y * 40 + 20);
        float dist = std::sqrt(std::pow(targetRealPos.x - r.realPos.x, 2) + std::pow(targetRealPos.y - r.realPos.y, 2));

        if (dist > 3.0f) {
            for(int t=0; t<5; t++) reserved.insert({r.gridPos.x, r.gridPos.y, t});
            continue;
        }

        Pos goal = r.gridPos;
        bool foundTarget = false;

        if (r.hasObject) {
            for (auto& b : deliveryPoints) {
                if (b.ownerId == r.id) { goal = b.pos; foundTarget = true; break; }
            }
        } else {
            int bestIdx = -1; float minDist = 1e9;
            for (int i = 0; i < (int)objects.size(); i++) {
                if (!objects[i].delivered && objects[i].carrierId == -1) {
                    float d = std::abs(r.gridPos.x - objects[i].pos.x) + std::abs(r.gridPos.y - objects[i].pos.y);
                    if (d < minDist) { minDist = d; bestIdx = i; }
                }
            }

            if (bestIdx != -1) {
                goal = objects[bestIdx].pos;
                r.targetObjIdx = bestIdx;
                foundTarget = true;
            } else {
                for (auto& b : deliveryPoints) {
                    if (b.ownerId == r.id) { goal = b.pos; foundTarget = true; break; }
                }
            }
        }

        std::vector<Pos> nextPath = findPath(r.gridPos, goal, reserved, current_algo);

        if (!nextPath.empty()) {
            r.path = nextPath;
            r.gridPos = r.path.back();
            r.path.pop_back();
        }

        for (int t = 1; t < 10; t++) {
            reserved.insert({r.gridPos.x, r.gridPos.y, t});
        }

        if (!r.hasObject && r.targetObjIdx != -1 && r.gridPos == objects[r.targetObjIdx].pos) {
            r.hasObject = true; objects[r.targetObjIdx].carrierId = r.id; r.path.clear();
        }
        if (r.hasObject) {
            objects[r.targetObjIdx].pos = r.gridPos;
            for (auto& b : deliveryPoints) if (b.ownerId == r.id && r.gridPos == b.pos) {
                    objects[r.targetObjIdx].delivered = true; r.hasObject = false; r.path.clear(); break;
                }
        }
    }
}