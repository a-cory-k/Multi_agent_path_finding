#pragma once
#include <vector>
#include <set>
#include <tuple>
#include "world.h"

std::vector<Pos> findPath(
        Pos start,
        Pos goal,
        const std::set<std::tuple<int, int, int>>& reserved
);