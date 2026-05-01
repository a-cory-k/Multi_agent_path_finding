#include "world.h"
#include "pathfinding.h"
#include <cmath>

std::vector<std::string> grid = {
        "####################",
        "#...........#.....A#",
        "#..O......####.....#",
        "#...........#..O...#",
        "#...........#......#",
        "#.....########.....#",
        "#..O.........OOO...#",
        "#.................#",
        "#..O...........B...#",
        "####################"
};

std::vector<Object> objects;
std::vector<Robot> robots;
Pos baseA, baseB;
std::set<std::tuple<int, int, int>> reserved;

// Исправление ошибки линковки: реализация операторов[cite: 7]
bool Pos::operator==(const Pos& o) const { return x == o.x && y == o.y; }
bool Pos::operator<(const Pos& o) const { return std::tie(x, y) < std::tie(o.x, o.y); }

bool is_free(int x, int y) {
    return y >= 0 && y < (int)grid.size() &&
           x >= 0 && x < (int)grid[0].size() &&
           grid[y][x] != '#';
}

void updateLogic() {
    reserved.clear();
    // Резервируем текущие позиции всех роботов (время 0)[cite: 6]
    for (auto &r : robots)
        reserved.insert({r.gridPos.x, r.gridPos.y, 0});

    for (auto &r : robots) {
        // Поиск ближайшего объекта, если робот свободен[cite: 6]
        if (!r.hasObject && (r.targetObjIdx == -1 || objects[r.targetObjIdx].carrierId != -1)) {
            int best = -1, dist = 1e9;
            for (int i = 0; i < (int)objects.size(); i++) {
                if (objects[i].carrierId == -1 && !objects[i].delivered) {
                    int d = abs(r.gridPos.x - objects[i].pos.x) + abs(r.gridPos.y - objects[i].pos.y);
                    if (d < dist) { dist = d; best = i; }
                }
            }
            r.targetObjIdx = best;
        }

        Pos target = r.gridPos;
        if (r.hasObject) target = (r.team == 0 ? baseA : baseB);
        else if (r.targetObjIdx != -1) target = objects[r.targetObjIdx].pos;

        // Поиск пути с учетом зарезервированных клеток[cite: 6]
        r.path = findPath(r.gridPos, target, reserved);

        if (!r.path.empty()) {
            r.gridPos = r.path.back();
            r.path.pop_back();

            // Резервируем следующую клетку для других роботов (время 1)[cite: 6]
            reserved.insert({r.gridPos.x, r.gridPos.y, 1});

            // Если несем объект — обновляем его позицию
            if (r.hasObject) {
                for (auto &o : objects) {
                    if (o.carrierId == r.id) { o.pos = r.gridPos; break; }
                }
            }

            // Логика подбора объекта[cite: 6]
            if (!r.hasObject && r.targetObjIdx != -1 && r.gridPos == objects[r.targetObjIdx].pos) {
                r.hasObject = true;
                objects[r.targetObjIdx].carrierId = r.id;
            }

            // Логика доставки на базу[cite: 6]
            if (r.hasObject && r.gridPos == (r.team == 0 ? baseA : baseB)) {
                r.hasObject = false;
                for (auto &o : objects) {
                    if (o.carrierId == r.id) { o.delivered = true; o.carrierId = -1; break; }
                }
                r.targetObjIdx = -1;
            }
        }
    }
}