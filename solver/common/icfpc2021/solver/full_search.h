#pragma once

#include "common/data_structures/unsigned_set.h"
#include "common/geometry/d2/distance/distance_l2.h"
#include "common/geometry/d2/point.h"
#include "common/icfpc2021/task.h"
#include "common/icfpc2021/task_cache.h"

#include <vector>

#include "common/geometry/d2/point_io.h"

namespace solver {
class FullSearch {
 protected:
  Task task;
  TaskCache cache;
  ds::UnsignedSet used_vertices;
  std::vector<std::vector<std::vector<I2Point>>> valid_candidates;
  std::vector<I2Point> solution;

 public:
  FullSearch(const Task& _task) {
    task = _task;
    cache.Init(task);
    ResetSearch();
  }

  FullSearch(const Task& _task, const TaskCache& _cache) {
    task = _task;
    cache = _cache;
    ResetSearch();
  }

  void ResetSearch() {
    unsigned size = task.g.Size();
    used_vertices.Resize(size);
    used_vertices.Clear();
    valid_candidates.clear();
    valid_candidates.resize(size);
    for (unsigned i = 0; i < size; ++i) {
      valid_candidates[i].push_back(cache.GetValidPoints());
    }
    solution.resize(size);
  }

  void AddPoint(unsigned index, const I2Point& p) {
    // {
    //   for (unsigned i = 0; i < used_vertices.Size(); ++i) std::cout << "\t";
    //   std::cout << index << "\t" << p << std::endl;
    // }
    assert(!used_vertices.HasKey(index));
    used_vertices.Insert(index);
    solution[index] = p;
    for (auto e : task.g.EdgesEI(index)) {
      unsigned u = e.to;
      if (used_vertices.HasKey(u)) {
        // Verify only
        auto p1 = solution[u];
        auto d = SquaredDistanceL2(p, p1);
        if (!task.CheckDistance(e.info, d) || !cache.CheckSegment(I2ClosedSegment(p, p1))) {
          std::cout << "SUS!" << std::endl;
          assert(false);    
        }
      } else {
        // Filter points
        std::vector<I2Point> vnext;
        auto& vcurrent = valid_candidates[u].back();
        vnext.reserve(vcurrent.size());
        for (auto p1 : vcurrent) {
          auto d = SquaredDistanceL2(p, p1);
          if (task.CheckDistance(e.info, d) && cache.CheckSegment(I2ClosedSegment(p, p1))) {
            vnext.push_back(p1);
          }
        }
        valid_candidates[u].push_back(vnext);
      }
    }
  }
  
  void RemoveLastPoint() {
    unsigned index = used_vertices.Last();
    for (auto e : task.g.EdgesEI(index)) {
      unsigned u = e.to;
      if (!used_vertices.HasKey(u)) {
        valid_candidates[u].pop_back();
      }
    }
    used_vertices.RemoveLast();
  }

protected:
  bool SearchI() {
    if (used_vertices.Size() == task.g.Size()) {
      return true;
    }
    unsigned min_size = cache.GetValidPoints().size() + 1;
    unsigned min_index = used_vertices.SetSize();
    for (unsigned i = 0; i < used_vertices.SetSize(); ++i) {
      if (used_vertices.HasKey(i)) continue;
      if (valid_candidates[i].back().size() < min_size) {
        min_size = valid_candidates[i].back().size();
        min_index = i;
      }
    }
    // std::cout << "\t" << min_index << "\t" << min_size << std::endl;
    assert(min_index < used_vertices.SetSize());
    if (min_size == 0) return false;
    auto v = valid_candidates[min_index].back(); // for safety
    for (auto& p : v) {
      AddPoint(min_index, p);
      if (SearchI()) return true;
      RemoveLastPoint();
    }
    return false;
  }

 public:
  bool Search() { return SearchI(); }

  const std::vector<I2Point>& GetSolution() const { return solution; }
};
}  // namespace solver
