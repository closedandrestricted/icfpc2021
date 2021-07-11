#pragma once

#include "common/base.h"
#include "common/icfpc2021/solver/full_search.h"

namespace solver {
class PerfectScore : public FullSearch {
 public:
  using TBase = FullSearch;

 public:
  PerfectScore(const Task& _task) : FullSearch(_task) {}

 protected:
  bool SearchI(unsigned k) {
    if (k == task.hole.Size()) return TBase::Search();
    auto p = task.hole[k];
    for (unsigned i = 0; i < used_vertices.SetSize(); ++i) {
      if (used_vertices.HasKey(i)) continue;
      auto& v = valid_candidates[i].back();
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
