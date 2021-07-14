#pragma once

#include "common/base.h"
#include "common/geometry/d2/point.h"
#include "common/geometry/d2/polygon.h"
#include "common/geometry/d2/segment.h"
#include "common/geometry/d2/distance/distance_l2.h"
#include "common/geometry/d2/utils/inside_segment_polygon.h"
#include "common/graph/graph_ei.h"
#include "common/icfpc2021/solution.h"
#include "common/numeric/utils/abs.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

class Task {
public:
  int eps;
  I2Polygon hole;
  UndirectedGraphEI<std::pair<int64_t, int64_t>> g;
  std::vector<I2Point> bonuses_to_unlock;

  void Load(const std::string& filename) {
    std::ifstream is(filename);
    nlohmann::json raw;
    is >> raw;
    // std::cout << raw << std::endl;
    eps = raw["epsilon"];
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
      auto dd = (d * eps) / 1000000;
      g.AddEdge(u0, u1, {d - dd, d + dd});
    }
    auto bonuses = raw["bonuses"];
    for (auto b : bonuses) {
      auto pos = b["position"];
      bonuses_to_unlock.push_back({pos[0], pos[1]});
      // std::cout << "BTU = " << bonuses_to_unlock.back() << std::endl;
    }
  }

  bool IsValid(const Solution& s) const {
    if (s.points.size() != g.Size()) return false;
    for (unsigned u = 0; u < g.Size(); ++u) {
      auto p0 = s.points[u];
      for (auto e : g.EdgesEI(u)) {
        if (e.to < u) continue;
        auto p1 = s.points[e.to];
        auto d2 = SquaredDistanceL2(p0, p1);
        if ((d2 < e.info.first) || (d2 > e.info.second)) return false;
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

  // Raw points in [0, 1] scale without adjusting to best solution
  // and numer of vertexes / edges.
  double RawPoints(const Solution& s) const {
    return IsValid(s) ? 1.0 / (sqrt(1.0 + Score(s))) : 0.0;
  }
};
