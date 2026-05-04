#include "world.h"
#include "pathfinding.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
bool simulationStarted = false;
std::vector<std::string> grid;
std::vector<std::string> map_filenames;
std::vector<Object> objects;
std::vector<Robot> robots;
std::vector<Pos> deliveryPoints;
std::set<std::tuple<int, int, int>> reserved;
Algorithm current_algo = Algorithm::ASTAR;
std::vector<std::string> algo_names = {"BFS", "A-Star"};
int selected_map_idx = 0;

bool is_free(int x, int y) {
    return y >= 0 && y < (int)grid.size() && x >= 0 && x < (int)grid[0].size() && grid[y][x] != '#';
}

void scanMapDirectory() {
    map_filenames.clear();
    for (const auto& entry : std::filesystem::directory_iterator("maps")) {
        if (entry.path().extension() == ".txt") map_filenames.push_back(entry.path().string());
    }
    std::sort(map_filenames.begin(), map_filenames.end());
}

bool loadMapFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    grid.clear();
    std::string line;
    while (std::getline(file, line)) { if (!line.empty()) grid.push_back(line); }
    return !grid.empty();
}

void resetWorld() {
    simulationStarted = false;
    if (map_filenames.empty()) return;
    loadMapFromFile(map_filenames[selected_map_idx]);
    objects.clear(); robots.clear(); deliveryPoints.clear();
    int rCount = 0;
    for (int y = 0; y < (int)grid.size(); y++) {
        for (int x = 0; x < (int)grid[y].size(); x++) {
            if (grid[y][x] >= 'A' && grid[y][x] <= 'Z' && grid[y][x] != 'R' && grid[y][x] != 'O') deliveryPoints.push_back({x, y});
            else if (grid[y][x] == 'O') { objects.push_back({{x, y}, -1, false}); grid[y][x] = '.'; }
            else if (grid[y][x] == 'R') { robots.push_back({rCount++, {(float)x*40+20, (float)y*40+20}, {x, y}, -1, false, {}}); grid[y][x] = '.'; }
        }
    }
}

void updateLogic() {
    if (!simulationStarted) return; // Проверка флага запуска

    reserved.clear();
    // Сначала бронируем текущие позиции всех роботов
    for (auto &r : robots) reserved.insert({r.gridPos.x, r.gridPos.y, 0});

    for (auto &r : robots) {
        // 1. Поиск новой цели, если робот свободен
        if (!r.hasObject && (r.targetObjIdx == -1 || objects[r.targetObjIdx].carrierId != -1)) {
            int best = -1, minDist = 1e9;
            for (int i = 0; i < (int)objects.size(); i++) {
                if (objects[i].carrierId == -1 && !objects[i].delivered) {
                    int d = std::abs(r.gridPos.x - objects[i].pos.x) + std::abs(r.gridPos.y - objects[i].pos.y);
                    if (d < minDist) { minDist = d; best = i; }
                }
            }
            r.targetObjIdx = best;
        }

        // 2. Определяем цель
        Pos target = r.gridPos;
        bool isAtBase = false;

        if (r.hasObject) {
            int minDist = 1e9;
            for (auto& b : deliveryPoints) {
                int d = std::abs(r.gridPos.x - b.x) + std::abs(r.gridPos.y - b.y);
                if (d < minDist) { minDist = d; target = b; }
            }
        } else if (r.targetObjIdx != -1) {
            target = objects[r.targetObjIdx].pos;
        }

        // 3. ПРОВЕРКА: Если робот на базе и без дела, ищем куда отойти
        for (auto& b : deliveryPoints) {
            if (r.gridPos == b && !r.hasObject) {
                isAtBase = true; break;
            }
        }

        // Если робот мешает на базе, меняем цель на любую соседнюю свободную клетку
        if (isAtBase && r.targetObjIdx == -1) {
            int dx[] = {1, -1, 0, 0};
            int dy[] = {0, 0, 1, -1};
            for (int i = 0; i < 4; i++) {
                int nx = r.gridPos.x + dx[i], ny = r.gridPos.y + dy[i];
                // Если соседняя клетка не стена и не база, идем туда, чтобы уступить
                if (is_free(nx, ny)) {
                    bool isAnotherBase = false;
                    for (auto& b : deliveryPoints) if (nx == b.x && ny == b.y) isAnotherBase = true;
                    if (!isAnotherBase) { target = {nx, ny}; break; }
                }
            }
        }

        // 4. Поиск пути и движение
        r.path = findPath(r.gridPos, target, reserved, current_algo);

        if (!r.path.empty()) {
            r.gridPos = r.path.back();
            r.path.pop_back();
            reserved.insert({r.gridPos.x, r.gridPos.y, 1});

            // Обновляем позицию груза
            if (r.hasObject) {
                for (auto &o : objects) if (o.carrierId == r.id) o.pos = r.gridPos;
            }

            // Подбор
            if (!r.hasObject && r.targetObjIdx != -1 && r.gridPos == objects[r.targetObjIdx].pos) {
                r.hasObject = true;
                objects[r.targetObjIdx].carrierId = r.id;
            }

            // Разгрузка
            if (r.hasObject) {
                for (auto& b : deliveryPoints) {
                    if (r.gridPos == b) {
                        r.hasObject = false;
                        for (auto &o : objects) if (o.carrierId == r.id) { o.delivered = true; o.carrierId = -1; }
                        r.targetObjIdx = -1;
                        break;
                    }
                }
            }
        } else {
            // Если стоим, всё равно бронируем место, чтобы в нас не въехали
            reserved.insert({r.gridPos.x, r.gridPos.y, 1});
        }
    }
}