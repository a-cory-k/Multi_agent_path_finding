#pragma once
#include "world.h"
#include <vector>
#include <set>
#include <tuple>

std::vector<Pos> findPath(Pos start, Pos goal, const std::set<std::tuple<int, int, int>>& reserved, Algorithm algo);