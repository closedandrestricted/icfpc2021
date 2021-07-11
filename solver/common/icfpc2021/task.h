#pragma once

#include "common/base.h"
#include "common/geometry/d2/polygon.h"
#include "common/geometry/d2/segment.h"
#include "common/geometry/d2/distance/distance_l2.h"
#include "common/geometry/d2/utils/inside_segment_polygon.h"
#include "common/graph/graph_ei.h"
#include "common/icfpc2021/solution.h"
#include "common/numeric/utils/abs.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

class Task {
public:
  I2Polygon hole;
  UndirectedGraphEI<int64_t> g;
  unsigned eps;

  void Load(const std::string& filename) {
    std::ifstream is(filename);
    nlohmann::json raw;
    is >> raw;
    // std::cout << raw << std::endl;
    eps = int(raw["epsilon"]);
    auto figure = raw["figure"];
    unsigned hsize = raw["hole"].size(), fsize = figure["vertices"].size();
    std::vector<I2Point> vh, vf;
    for (unsigned i = 0; i < hsize; ++i) vh.push_back({raw["hole"][i][0], raw["hole"][i][1]});
    hole = I2Polygon(vh);
    for (unsigned i = 0; i < fsize; ++i) vf.push_back({figure["vertices"][i][0], figure["vertices"][i][1]});
    g.Resize(fsize);
    for (auto e : figure["edges"]) {
      unsigned u0 = e[0], u1 = e[1];
      auto d = SquaredDistanceL2(vf[u0], vf[u1]);
      g.AddEdge(u0, u1, d);
    }
  }

  bool CheckDistance(int64_t required, int64_t current) const {
    return Abs(current - required) * 1000000ll <= required * eps;
  }

  bool IsValid(const Solution& s) const {
    if (s.points.size() != g.Size()) return false;
    for (unsigned u = 0; u < g.Size(); ++u) {
      auto p0 = s.points[u];
      for (auto e : g.EdgesEI(u)) {
        if (e.to < u) continue;
        auto p1 = s.points[e.to];
        auto d2 = SquaredDistanceL2(p0, p1);
        if (!CheckDistance(e.info, d2)) return false;
        if (!geometry::d2::Inside(I2ClosedSegment(p0, p1), hole)) return false;
      }
    }
    return true;
  }

  int64_t Score(const Solution& s) const {
    int64_t score = 0;
    for (unsigned i = 0; i < hole.Size(); ++i) {
      auto p0 = hole[i];
      int64_t min_d = SquaredDistanceL2(p0, s.points[0]);
      for (unsigned j = 1; j < s.points.size(); ++j) {
        int64_t d = SquaredDistanceL2(p0, s.points[j]);
        min_d = std::min(min_d, d);
      }
      score += min_d;
    }
    return score;
  }
};
