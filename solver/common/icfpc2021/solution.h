#pragma once

#include "common/geometry/d2/point.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

// TODO: Add boosts
class Solution {
 public:
  std::vector<I2Point> points;

 public:
  nlohmann::json ToJson() const {
    nlohmann::json js;
    for (auto p : points) {
      js["vertices"].push_back({p.x, p.y});
    }
    return js;
  }

  bool Load(const std::string& filename) {
    std::ifstream is(filename);
    if (!is.is_open()) return false;
    nlohmann::json raw;
    is >> raw;
    auto jpoints = raw["vertices"];
    points.clear();
    for (unsigned i = 0; i < jpoints.size(); ++i) {
      points.push_back({jpoints[i][0], jpoints[i][1]});
    }
    return true;
  }
};
