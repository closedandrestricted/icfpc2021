#pragma once

#include "common/base.h"
#include "common/icfpc2021/solver/full_search.h"

namespace solver {
class PerfectScore : public FullSearch {
 public:
  using TBase = FullSearch;

 public:
  PerfectScore(const Task& _task) : FullSearch(_task) {}

  void AddPoint(unsigned index, const I2Point& p) {
    assert(!used_vertices.HasKey(index));
    for (unsigned u = 0; u < used_vertices.SetSize(); ++u) {
      if (u == index) continue; 
      if (!used_vertices.HasKey(u)) {
        // Filter points
        auto& vcurrent = valid_candidates[u][valid_candidates_index[u]];
        auto& vnext = valid_candidates[u][++valid_candidates_index[u]];
        vnext.clear();
        for (auto p1 : vcurrent) {
          auto d = SquaredDistanceL2(p, p1);
          if ((d >= cache.min_distance[index][u]) && (d <= cache.max_distance[index][u])) {
            vnext.push_back(p1);
          }
        }
      }
    }
    TBase::AddPoint(index, p);
  }
  
  void RemoveLastPoint() {
    unsigned index = used_vertices.Last();
    TBase::RemoveLastPoint();
    for (unsigned u = 0; u < used_vertices.SetSize(); ++u) {
      if (u == index) continue;
      if (!used_vertices.HasKey(u)) {
        --valid_candidates_index[u];
      }
    }
  }

 protected:
  bool SearchI(unsigned k) {
    if (k == task.hole.Size()) return TBase::Search();
    auto p = task.hole[k];
    for (unsigned i = 0; i < used_vertices.SetSize(); ++i) {
      if (used_vertices.HasKey(i)) continue;
      auto& v = valid_candidates[i][valid_candidates_index[i]];
      // std::cout << k << "\t" << i << "\t" << v.size() << std::endl;
      if (v.size() == 0) return false;
      bool is_valid = false;
      for (auto& vp : v) {
        if (vp == p) {
          is_valid = true;
          break;
        }
      }
      if (is_valid) {
        AddPoint(i, p);
        if (SearchI(k + 1)) return true;
        RemoveLastPoint();
      }
    }
    return false;
  }

 public:
  bool Search() {
    assert(task.hole.Size() <= task.g.Size());
    return SearchI(0);
  }
};
}  // namespace solver
