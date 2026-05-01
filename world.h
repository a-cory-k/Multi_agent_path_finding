#pragma once

#include <vector>
#include <string>
#include <set>
#include <tuple>
#include <SFML/Graphics.hpp>

struct Pos {
    int x, y;
    bool operator==(const Pos& o) const;
    bool operator<(const Pos& o) const; // Нужно для использования Pos в std::set или std::map
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
    int team;
    std::vector<Pos> path;
};

extern std::vector<std::string> grid;
extern std::vector<Object> objects;
extern std::vector<Robot> robots;
extern Pos baseA, baseB;
extern std::set<std::tuple<int, int, int>> reserved;

bool is_free(int x, int y);
void updateLogic();