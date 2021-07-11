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
#include <random>
#include <unordered_set>
#include <vector>

#include "common/geometry/d2/point_io.h"

class TaskCache {
 protected:
  std::default_random_engine re;
  I2ARectangle box;
  std::vector<std::vector<unsigned>> valid_points_map;
  std::vector<I2Point> valid_points;
  std::vector<std::vector<std::vector<I2Point>>> valid_segments_map;
  std::unordered_set<I2ClosedSegment> valid_segments_set;

 public:
  std::vector<std::vector<int64_t>> min_distance;
  std::vector<std::vector<int64_t>> max_distance;

 public:
  const std::vector<I2Point>& GetValidPoints() const {
    return valid_points;
  }

  void Init(const Task& task) {
    re.seed();
    box = Box(task.hole.v);
    valid_points_map.clear();
    valid_points_map.resize(box.p2.x - box.p1.x + 1);
    valid_segments_map.clear();
    valid_segments_map.resize(valid_points_map.size());
    // Valid points
    for (unsigned i = 0; i < valid_points_map.size(); ++i) {
      valid_points_map[i].resize(box.p2.y - box.p1.y + 1, 0);
      valid_segments_map[i].resize(valid_points_map[i].size());
      for (unsigned j = 0; j < valid_points_map[i].size(); ++j) {
        I2Point p0(box.p1.x + i, box.p1.y + j);
        if (geometry::d2::Inside(p0, task.hole)) {
          valid_points_map[i][j] = 1;
          valid_points.push_back(p0);
        //   std::cout << p0 << std::endl;
        }
      }
    }
    // Valid segments
    for (auto p1 : valid_points) {
      auto& vm = valid_segments_map[p1.x - box.p1.x][p1.y - box.p1.y];
      for (auto p2: valid_points) {
        I2ClosedSegment s(p1, p2);
        if (geometry::d2::Inside(s, task.hole)) {
        //   std::cout << "\t" << p1 << "\t" << p2 << std::endl;
          vm.push_back(p2);
          valid_segments_set.insert(s);
        }
      }
      std::shuffle(vm.begin(), vm.end(), re);
    }
    // Init min/max distance between vertexes for figure.
    UndirectedGraphEI<int64_t> gf(task.g.Size());
    for (unsigned u = 0; u < gf.Size(); ++u) {
      for (auto e : task.g.EdgesEI(u)) {
        gf.AddEdge(u, e.to, USqrt(e.info.second - 1) + 1);
      }
    }
    auto vvd = DistanceAllPairsPositiveCost(gf, 1ll << 30);
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

  bool CheckSegment(const I2ClosedSegment& s) const {
    return valid_segments_set.find(s) != valid_segments_set.end();
  }
};
