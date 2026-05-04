#include "pathfinding.h"
#include <queue>
#include <map>
#include <cmath>
#include <tuple>


struct Node {
    int x, y, t;
    float f, g; // f = g + h
    bool operator>(const Node& o) const { return f > o.f; }
};

float getH(int x1, int y1, int x2, int y2) {
    return (float)(std::abs(x1 - x2) + std::abs(y1 - y2));
}

std::vector<Pos> findPath(Pos start, Pos goal, const std::set<std::tuple<int, int, int>>& reserved, Algorithm algo) {

    if (start == goal) return {};

    using State = std::tuple<int, int, int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    std::map<State, State> parent;
    std::map<State, float> gScore;

    float start_h = (algo == Algorithm::ASTAR) ? getH(start.x, start.y, goal.x, goal.y) : 0.0f;
    pq.push({start.x, start.y, 0, start_h, 0.0f});
    gScore[{start.x, start.y, 0}] = 0.0f;

    int dx[] = {0, 1, -1, 0, 0};
    int dy[] = {0, 0, 0, 1, -1};

    while (!pq.empty()) {
        Node curr = pq.top();
        pq.pop();

        if (curr.x == goal.x && curr.y == goal.y) {
            std::vector<Pos> path;
            State s = {curr.x, curr.y, curr.t};
            while (parent.count(s)) {
                path.push_back({std::get<0>(s), std::get<1>(s)});
                s = parent[s];
            }
            return path;
        }

        if (curr.t > 40) continue;

        for (int i = 0; i < 5; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];
            int nt = curr.t + 1;

            if (is_free(nx, ny)) {
                bool vConflict = reserved.count({nx, ny, nt});

                bool eConflict = false;
                if (i != 0 && reserved.count({nx, ny, curr.t}) && reserved.count({curr.x, curr.y, nt})) {
                    eConflict = true;
                }

                if (!vConflict && !eConflict) {
                    float moveCost = (i == 0) ? 1.2f : 1.0f;
                    float next_g = curr.g + moveCost;
                    State ns = {nx, ny, nt};

                    if (!gScore.count(ns) || next_g < gScore[ns]) {
                        gScore[ns] = next_g;
                        parent[ns] = {curr.x, curr.y, curr.t};

                        float hValue = 0.0f;
                        if (algo == Algorithm::ASTAR) {
                            hValue = getH(nx, ny, goal.x, goal.y);
                        }

                        pq.push({nx, ny, nt, next_g + hValue, next_g});
                    }
                }
            }
        }
    }

    return {};
}