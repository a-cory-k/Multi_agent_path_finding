#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <cmath>
#include <set>
#include <map>
#include <tuple>
#include <algorithm>
#include <string>

using namespace std;

const int TILE_SIZE = 40;
const float ROBOT_SPEED = 6.0f;

struct Pos {
    int x, y;
    bool operator==(const Pos& o) const { return x == o.x && y == o.y; }
    bool operator<(const Pos& o) const { return tie(x, y) < tie(o.x, o.y); }
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
    vector<Pos> path;
};

vector<string> grid = {
        "####################",
        "#...........#.....A########",
        "#..O......########.....O..#",
        "#...........#..O...#######",
        "#...........#......#",
        "#.....########.....#",
        "#..O.........OOO....#",
        "#.................#",
        "#..O...........B..#",
        "####################"
};

vector<Object> objects;
vector<Robot> robots;
Pos baseA, baseB;
set<tuple<int, int, int>> reserved;

bool is_free(int x, int y) {
    return y >= 0 && y < (int)grid.size() && x >= 0 && x < (int)grid[0].size() && grid[y][x] != '#';
}

vector<Pos> findPath(Pos start, Pos goal) {
    if (start == goal) return {};
    using State = tuple<int, int, int>;
    queue<State> q;
    q.push({start.x, start.y, 0});
    map<State, State> parent;
    set<State> visited;
    visited.insert({start.x, start.y, 0});

    while (!q.empty()) {
        auto [cx, cy, ct] = q.front(); q.pop();
        if (cx == goal.x && cy == goal.y) {
            vector<Pos> res;
            State curr = {cx, cy, ct};
            while (parent.count(curr)) {
                res.push_back({get<0>(curr), get<1>(curr)});
                curr = parent[curr];
            }
            return res;
        }
        if (ct > 60) continue;
        int dx[] = {0, 1, -1, 0, 0}, dy[] = {0, 0, 0, 1, -1};
        for (int i = 0; i < 5; i++) {
            int nx = cx + dx[i], ny = cy + dy[i], nt = ct + 1;
            if (is_free(nx, ny) && !reserved.count({nx, ny, nt}) && !visited.count({nx, ny, nt})) {
                visited.insert({nx, ny, nt});
                parent[{nx, ny, nt}] = {cx, cy, ct};
                q.push({nx, ny, nt});
            }
        }
    }
    return {};
}

void updateLogic() {
    reserved.clear();
    for(auto &r : robots) reserved.insert({r.gridPos.x, r.gridPos.y, 0});

    for (auto &r : robots) {
        if (!r.hasObject && (r.targetObjIdx == -1 || objects[r.targetObjIdx].carrierId != -1)) {
            int minDist = 1000; r.targetObjIdx = -1;
            for (int i = 0; i < (int)objects.size(); i++) {
                if (objects[i].carrierId == -1 && !objects[i].delivered) {
                    int d = abs(r.gridPos.x - objects[i].pos.x) + abs(r.gridPos.y - objects[i].pos.y);
                    if (d < minDist) { minDist = d; r.targetObjIdx = i; }
                }
            }
        }

        Pos targetPos = r.gridPos;
        bool needMove = false;
        if (r.hasObject) {
            targetPos = (r.team == 0 ? baseA : baseB);
            needMove = true;
        } else if (r.targetObjIdx != -1) {
            targetPos = objects[r.targetObjIdx].pos;
            needMove = true;
        }

        r.path = findPath(r.gridPos, targetPos);

        if (!r.path.empty()) {
            r.gridPos = r.path.back();
            r.path.pop_back();
            reserved.insert({r.gridPos.x, r.gridPos.y, 1});

            if (!r.hasObject && r.targetObjIdx != -1 && r.gridPos == objects[r.targetObjIdx].pos) {
                r.hasObject = true;
                objects[r.targetObjIdx].carrierId = r.id;
            }
            else if (r.hasObject && r.gridPos == (r.team == 0 ? baseA : baseB)) {
                r.hasObject = false;
                for(auto &o : objects) if (o.carrierId == r.id) { o.delivered = true; o.carrierId = -1; break; }
                r.targetObjIdx = -1;
            }
        } else {
            Pos myBase = (r.team == 0 ? baseA : baseB);
            if (r.gridPos == myBase && r.targetObjIdx == -1) {
                int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
                for (int i = 0; i < 4; i++) {
                    int nx = r.gridPos.x + dx[i], ny = r.gridPos.y + dy[i];
                    if (is_free(nx, ny) && !reserved.count({nx, ny, 1})) {
                        r.gridPos = {nx, ny};
                        break;
                    }
                }
            }
            reserved.insert({r.gridPos.x, r.gridPos.y, 1});
        }
    }
}

int main() {
    for (int y = 0; y < (int)grid.size(); y++) {
        for (int x = 0; x < (int)grid[y].size(); x++) {
            if (grid[y][x] == 'A') baseA = {x, y};
            if (grid[y][x] == 'B') baseB = {x, y};
            if (grid[y][x] == 'O') { objects.push_back({{x, y}, -1, false}); grid[y][x] = '.'; }
        }
    }

    robots.push_back({0, {40,40}, {1,1}, -1, false, 0});
    robots.push_back({1, {80,40}, {2,1}, -1, false, 0});
    robots.push_back({2, {40,320}, {1,8}, -1, false, 1});
    robots.push_back({3, {80,320}, {2,8}, -1, false, 1});

    sf::RenderWindow window(sf::VideoMode({(unsigned int)grid[0].size()*TILE_SIZE, (unsigned int)grid.size()*TILE_SIZE}), "Robot Warehouse 3.0");
    window.setFramerateLimit(60);
    sf::Clock clock;
    float stepTimer = 0;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        stepTimer += dt;
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        if (stepTimer > 0.4f) { updateLogic(); stepTimer = 0; }

        window.clear(sf::Color(25, 25, 25));

        for (int y = 0; y < (int)grid.size(); y++) {
            for (int x = 0; x < (int)grid[0].size(); x++) {
                sf::RectangleShape tile(sf::Vector2f({TILE_SIZE-1.0f, TILE_SIZE-1.0f}));
                tile.setPosition({(float)x*TILE_SIZE, (float)y*TILE_SIZE});
                tile.setFillColor(grid[y][x] == '#' ? sf::Color(60,60,60) : sf::Color(40,40,40));
                window.draw(tile);
            }
        }

        auto drawBase = [&](Pos p, sf::Color color) {
            sf::RectangleShape b(sf::Vector2f({TILE_SIZE-8.0f, TILE_SIZE-8.0f}));
            b.setPosition({(float)p.x*TILE_SIZE+4, (float)p.y*TILE_SIZE+4});
            b.setFillColor(sf::Color(color.r, color.g, color.b, 40));
            b.setOutlineThickness(2);
            b.setOutlineColor(color);
            window.draw(b);
        };
        drawBase(baseA, sf::Color::Cyan);
        drawBase(baseB, sf::Color::Magenta);

        for (auto &obj : objects) {
            if (obj.delivered || obj.carrierId != -1) continue;
            sf::RectangleShape oShape(sf::Vector2f({18, 18}));
            oShape.setOrigin({9, 9});
            oShape.setPosition({(float)obj.pos.x*TILE_SIZE+20, (float)obj.pos.y*TILE_SIZE+20});
            oShape.setFillColor(sf::Color::Yellow);
            window.draw(oShape);
        }

        for (auto &r : robots) {
            sf::Vector2f target((float)r.gridPos.x*TILE_SIZE+20, (float)r.gridPos.y*TILE_SIZE+20);
            r.realPos += (target - r.realPos) * dt * ROBOT_SPEED;
            sf::CircleShape rShape(14);
            rShape.setOrigin({14, 14});
            rShape.setPosition(r.realPos);
            rShape.setFillColor(r.team == 0 ? sf::Color::Cyan : sf::Color::Magenta);
            window.draw(rShape);
            if (r.hasObject) {
                sf::RectangleShape cargo(sf::Vector2f({12, 12}));
                cargo.setOrigin({6, 6});
                cargo.setPosition(r.realPos);
                cargo.setFillColor(sf::Color::Yellow);
                window.draw(cargo);
            }
        }
        window.display();
    }
    return 0;
}