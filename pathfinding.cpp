#include "pathfinding.h"
#include <queue>
#include <map>

using namespace std;
using State = tuple<int, int, int>;

vector<Pos> findPath(Pos start, Pos goal, const set<State>& reserved) {
    if (start == goal) return {};

    queue<State> q;
    map<State, State> parent;
    set<State> vis;

    q.push({start.x, start.y, 0});
    vis.insert({start.x, start.y, 0});

    while (!q.empty()) {
        auto [cx, cy, ct] = q.front();
        q.pop();

        if (cx == goal.x && cy == goal.y) {
            vector<Pos> res;
            State cur = {cx, cy, ct};
            while (parent.count(cur)) {
                res.push_back({get<0>(cur), get<1>(cur)});
                cur = parent[cur];
            }
            return res;
        }

        if (ct > 50) continue; // Ограничение глубины поиска

        int dx[] = {0, 1, -1, 0, 0}; // 5-е направление — стоять на месте
        int dy[] = {0, 0, 0, 1, -1};

        for (int i = 0; i < 5; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            int nt = ct + 1;
            State nxt = {nx, ny, nt};

            // Исправлено условие: идем, если клетка свободна И НЕ зарезервирована[cite: 2]
            if (is_free(nx, ny) && !vis.count(nxt) && !reserved.count(nxt)) {
                vis.insert(nxt);
                parent[nxt] = {cx, cy, ct};
                q.push(nxt);
            }
        }
    }
    return {};
}