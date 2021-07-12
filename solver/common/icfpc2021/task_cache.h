#pragma once

#include "common/geometry/d2/axis/rectangle.h"
#include "common/geometry/d2/distance/distance_l2.h"
#include "common/geometry/d2/point.h"
#include "common/geometry/d2/segment.h"
#include "common/geometry/d2/stl_hash/segment.h"
#include "common/geometry/d2/utils/box.h"
#include "common/geometry/d2/utils/inside_point_polygon.h"
#include "common/geometry/d2/utils/inside_segment_polygon.h"
#include "common/graph/graph_ei.h"
#include "common/graph/graph_ei/distance_positive_cost.h"
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
  std::unordered_map<I2ClosedSegment, int64_t> segments_hole_distance;

 public:
  std::vector<std::vector<int64_t>> min_distance;
  std::vector<std::vector<int64_t>> max_distance;
  std::vector<std::vector<int64_t>> min_hole_distance;

 public:
  const std::vector<I2Point>& GetValidPoints() const {
    return valid_points;
  }

  void Init(const Task& task) {
    hole = task.hole;
    unsigned hsize = hole.Size();
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
    // Init 'hole-distance' from vertexes in the hole
    UndirectedGraphEI<int64_t> gh(hsize + 1);
    for (unsigned u = 0; u < hsize; ++u) {
      for (unsigned v = 0; v < hsize; ++v) {
        I2ClosedSegment s(hole[u], hole[v]);
        if (CheckSegmentI(s)) {
          gh.AddEdge(u, v, USqrt(SquaredDistanceL2(s.p1, s.p2)));
        }
      }
    }
    auto vvd2 = DistanceAllPairsPositiveCost(gh, static_cast<int64_t>(1ll << 30));
    min_hole_distance.resize(hsize);
    for (unsigned u = 0; u < hsize; ++u) {
      min_hole_distance[u].resize(hsize);
      for (unsigned v = 0; v < hsize; ++v) {
        I2ClosedSegment s(hole[u], hole[v]);
        if (CheckSegmentI(s)) {
          min_hole_distance[u][v] = SquaredDistanceL2(s.p1, s.p2);
        } else {
          min_hole_distance[u][v] = vvd2[u][v] * vvd2[u][v];
        }
        segments_hole_distance.insert({s, min_hole_distance[u][v]});
      }
    }
    // Init 'hole-distance' from vertexes in the the hole to other vertexes
    for (auto p : valid_points) {
      I2ClosedSegment stest(p, p);
      if (segments_hole_distance.find(stest) != segments_hole_distance.end()) continue;
      gh.ClearEdges(hsize);
      for (unsigned u = 0; u < hsize; ++u) {
        I2ClosedSegment s(hole[u], p);
        if (CheckSegmentI(s)) gh.AddBaseEdge(hsize, u, USqrt(SquaredDistanceL2(p, hole[u])));
      }
      auto vd = DistanceFromSourcePositiveCost(gh, hsize, int64_t(1ll << 60));
      for (unsigned u = 0; u < hsize; ++u) {
        int64_t d = vd[u];
        int64_t d2 = std::max(d * d, SquaredDistanceL2(p, hole[u]));
        I2ClosedSegment s1(p, hole[u]), s2(hole[u], p);
        segments_hole_distance.insert({s1, d2});
        segments_hole_distance.insert({s2, d2});
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

  int64_t SegmentHoleDistance(const I2ClosedSegment& s) {
    auto it = segments_hole_distance.find(s);
    if (it != segments_hole_distance.end())
      return it->second;
    int64_t min_distance = (1ll << 60);
    if (CheckSegmentI(s)) {
      min_distance = SquaredDistanceL2(s.p1, s.p2);
    } else {
      for (unsigned i = 0; i < hole.Size(); ++i) {
        I2ClosedSegment s1(hole[i], s.p1);
        I2ClosedSegment s2(hole[i], s.p2);
        auto it1 = segments_hole_distance.find(s1), it2 = segments_hole_distance.find(s2);
        assert(it1 != segments_hole_distance.end());
        assert(it2 != segments_hole_distance.end());
        auto d = USqrt(it1->second) + USqrt(it2->second);
        auto dd = d * d;
        if (min_distance > dd) min_distance = dd;
      }
      auto cap = SquaredDistanceL2(s.p1, s.p2);
      if (min_distance < cap) min_distance = cap;
    }
    segments_hole_distance.insert({s, min_distance});
    return min_distance;
  }
};
