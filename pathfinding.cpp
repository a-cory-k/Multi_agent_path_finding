#include "pathfinding.h"
#include <queue>
#include <map>
#include <cmath>

struct Node {
    int x, y, t;
    float f, g;
    bool operator>(const Node& o) const { return f > o.f; }
};

float heuristic(int x1, int y1, int x2, int y2) {
    return (float)(std::abs(x1 - x2) + std::abs(y1 - y2));
}

std::vector<Pos> findPath(Pos start, Pos goal, const std::set<std::tuple<int, int, int>>& reserved, Algorithm algo) {
    if (start == goal) return {};

    using State = std::tuple<int, int, int>;
    std::map<State, State> parent;
    std::map<State, float> gScore;

    int dx[] = {0, 1, -1, 0, 0, 0};
    int dy[] = {0, 0, 0, 1, -1, 0};

    if (algo == Algorithm::ASTAR) {
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        pq.push({start.x, start.y, 0, heuristic(start.x, start.y, goal.x, goal.y), 0});
        gScore[{start.x, start.y, 0}] = 0;

        while (!pq.empty()) {
            Node curr = pq.top(); pq.pop();
            if (curr.x == goal.x && curr.y == goal.y) {
                std::vector<Pos> res;
                State s = {curr.x, curr.y, curr.t};
                while (parent.count(s)) { res.push_back({std::get<0>(s), std::get<1>(s)}); s = parent[s]; }
                return res;
            }
            if (curr.t > 60) continue;

            for (int i = 0; i < 6; i++) {
                int nx = curr.x + dx[i], ny = curr.y + dy[i], nt = curr.t + 1;
                if (is_free(nx, ny) && !reserved.count({nx, ny, nt})) {
                    float next_g = curr.g + (i == 0 ? 0.5f : 1.0f);
                    State nextS = {nx, ny, nt};
                    if (!gScore.count(nextS) || next_g < gScore[nextS]) {
                        gScore[nextS] = next_g;
                        parent[nextS] = {curr.x, curr.y, curr.t};
                        pq.push({nx, ny, nt, next_g + heuristic(nx, ny, goal.x, goal.y), next_g});
                    }
                }
            }
        }
    } else { // BFS
        std::queue<Node> q;
        q.push({start.x, start.y, 0, 0, 0});
        std::set<State> visited;
        visited.insert({start.x, start.y, 0});

        while (!q.empty()) {
            Node curr = q.front(); q.pop();
            if (curr.x == goal.x && curr.y == goal.y) {
                std::vector<Pos> res;
                State s = {curr.x, curr.y, curr.t};
                while (parent.count(s)) { res.push_back({std::get<0>(s), std::get<1>(s)}); s = parent[s]; }
                return res;
            }
            if (curr.t > 60) continue;

            for (int i = 0; i < 6; i++) {
                int nx = curr.x + dx[i], ny = curr.y + dy[i], nt = curr.t + 1;
                if (is_free(nx, ny) && !reserved.count({nx, ny, nt}) && !visited.count({nx, ny, nt})) {
                    visited.insert({nx, ny, nt});
                    parent[{nx, ny, nt}] = {curr.x, curr.y, curr.t};
                    q.push({nx, ny, nt, 0, 0});
                }
            }
        }
    }
    return {};
}