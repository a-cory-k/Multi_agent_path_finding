#pragma once
#include <vector>
#include <string>
#include <set>
#include <tuple>
#include <SFML/Graphics.hpp>
extern bool simulationStarted;
struct Pos {
    int x, y;
    bool operator==(const Pos& o) const { return x == o.x && y == o.y; }
    bool operator<(const Pos& o) const { return std::tie(x, y) < std::tie(o.x, o.y); }
};

struct Object {
    Pos pos;
    int carrierId = -1;
    bool delivered = false;
};

struct Robot {
    int id;
    sf::Vector2f realPos;
    Pos gridPos;
    int targetObjIdx = -1;
    bool hasObject = false;
    std::vector<Pos> path;
};

enum class Algorithm { BFS, ASTAR };

extern Algorithm current_algo;
extern std::vector<std::string> algo_names;
extern std::vector<std::string> grid;
extern std::vector<std::string> map_filenames;
extern std::vector<Object> objects;
extern std::vector<Robot> robots;
extern std::vector<Pos> deliveryPoints;
extern std::set<std::tuple<int, int, int>> reserved;
extern int selected_map_idx;

bool is_free(int x, int y);
void updateLogic();
void scanMapDirectory();
bool loadMapFromFile(const std::string& filename);
void resetWorld();