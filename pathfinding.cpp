#include "pathfinding.h"
#include <queue>
#include <map>

struct Node {
    int x, y, t;
    float f, g;
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

    pq.push({start.x, start.y, 0, getH(start.x, start.y, goal.x, goal.y), 0});
    gScore[{start.x, start.y, 0}] = 0;

    int dx[] = {0, 1, -1, 0, 0};
    int dy[] = {0, 0, 0, 1, -1};

    while (!pq.empty()) {
        Node curr = pq.top(); pq.pop();

        if (curr.x == goal.x && curr.y == goal.y) {
            std::vector<Pos> path;
            State s = {curr.x, curr.y, curr.t};
            while (parent.count(s)) {
                path.push_back({std::get<0>(s), std::get<1>(s)});
                s = parent[s];
            }
            return path;
        }

        if (curr.t > 30) continue;

        for (int i = 0; i < 5; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];
            int nt = curr.t + 1;

            if (is_free(nx, ny)) {
                if (reserved.count({nx, ny, nt})) continue;

                if (i != 0 && reserved.count({nx, ny, curr.t}) && reserved.count({curr.x, curr.y, nt})) continue;

                float moveCost = (i == 0) ? 1.1f : 1.0f;
                float next_g = curr.g + moveCost;
                State ns = {nx, ny, nt};

                if (!gScore.count(ns) || next_g < gScore[ns]) {
                    gScore[ns] = next_g;
                    parent[ns] = {curr.x, curr.y, curr.t};
                    pq.push({nx, ny, nt, next_g + getH(nx, ny, goal.x, goal.y), next_g});
                }
            }
        }
    }
    return {};
}