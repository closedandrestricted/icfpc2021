#pragma once

#include "common/geometry/d2/axis/rectangle.h"
#include "common/geometry/d2/point.h"
#include "common/geometry/d2/segment.h"
#include "common/geometry/d2/stl_hash/segment.h"
#include "common/geometry/d2/utils/box.h"
#include "common/geometry/d2/utils/inside_point_polygon.h"
#include "common/geometry/d2/utils/inside_segment_polygon.h"
#include "common/graph/graph_ei.h"
#include "common/graph/graph_ei/distance_all_pairs_positive_cost.h"
#include "common/icfpc2021/task.h"
#include "common/numeric/utils/usqrt.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "common/geometry/d2/point_io.h"

class TaskCache {
 protected:
  I2Polygon hole;
  I2ARectangle box;
  std::vector<std::vector<unsigned>> valid_points_map;
  std::vector<I2Point> valid_points;
  std::unordered_map<I2ClosedSegment, bool> valid_segments_map;

 public:
  std::vector<std::vector<int64_t>> min_distance;
  std::vector<std::vector<int64_t>> max_distance;

 public:
  const std::vector<I2Point>& GetValidPoints() const {
    return valid_points;
  }

  void Init(const Task& task) {
    hole = task.hole;
    box = Box(hole.v);
    valid_points_map.clear();
    valid_points_map.resize(box.p2.x - box.p1.x + 1);
    // Valid points
    for (unsigned i = 0; i < valid_points_map.size(); ++i) {
      valid_points_map[i].resize(box.p2.y - box.p1.y + 1, 0);
      for (unsigned j = 0; j < valid_points_map[i].size(); ++j) {
        I2Point p0(box.p1.x + i, box.p1.y + j);
        if (geometry::d2::Inside(p0, hole)) {
          valid_points_map[i][j] = 1;
          valid_points.push_back(p0);
        //   std::cout << p0 << std::endl;
        }
      }
    }
    // Init min/max distance between vertexes for figure.
    UndirectedGraphEI<int64_t> gf(task.g.Size());
    for (unsigned u = 0; u < gf.Size(); ++u) {
      for (auto e : task.g.EdgesEI(u)) {
        gf.AddEdge(u, e.to, USqrt(e.info.second - 1) + 1);
      }
    }
    auto vvd = DistanceAllPairsPositiveCost(gf, static_cast<int64_t>(1ll << 30));
    min_distance.clear();
    min_distance.resize(gf.Size());
    max_distance.clear();
    max_distance.resize(gf.Size());
    for (unsigned i = 0; i < gf.Size(); ++i) {
      min_distance[i].resize(gf.Size(), 0);
      max_distance[i].resize(gf.Size());
      for (unsigned j = 0; j < gf.Size(); ++j)
        max_distance[i][j] = vvd[i][j] * vvd[i][j];
    }
    for (unsigned u = 0; u < gf.Size(); ++u) {
      for (auto e : task.g.EdgesEI(u)) {
        min_distance[u][e.to] = e.info.first;
        max_distance[u][e.to] = e.info.second;
      }
    }
  }

  bool CheckPoint(const I2Point& p) const {
    if (!box.Inside(p)) return false;
    return valid_points_map[p.x - box.p1.x][p.y - box.p1.y];
  }

  bool CheckSegmentI(const I2ClosedSegment& s) {
    auto it = valid_segments_map.find(s);
    if (it != valid_segments_map.end()) return it->second;
    bool b = geometry::d2::Inside(s, hole);
    valid_segments_map.insert({s, b});
    return b;
  }

  bool CheckSegment(const I2ClosedSegment& s) {
    return CheckPoint(s.p1) && CheckPoint(s.p2) && CheckSegmentI(s);
  }
};
