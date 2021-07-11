#pragma once

#include "common/geometry/d2/point.h"

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
};
