#pragma once

#include "common/base.h"
#include "common/data_structures/unsigned_set.h"
#include "common/geometry/d2/point.h"
#include "common/icfpc2021/solver/full_search.h"

#include <vector>

namespace solver {
class PerfectScore : public FullSearch {
 public:
  using TBase = FullSearch;

 protected:
  std::vector<I2Point> vertexes_to_cover;
  ds::UnsignedSet covered_vertexes;

 public:
  PerfectScore(const Task& _task) : FullSearch(_task) {
    ResetSearch(_task.hole.v);
  }

  void ResetSearch() {
    TBase::ResetSearch();
    covered_vertexes.Clear();
  }

  void ResetSearch(const std::vector<I2Point>& _vertexes_to_cover) {
    vertexes_to_cover = _vertexes_to_cover;
    covered_vertexes.Resize(vertexes_to_cover.size());
    ResetSearch();
  }

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
          // auto d = SquaredDistanceL2(p, p1);
          auto d = cache.SegmentHoleDistance({p, p1});
          if (d <= cache.max_distance[index][u]) {
        //   if ((d >= cache.min_distance[index][u]) && (d <= cache.max_distance[index][u])) {
            vnext.push_back(p1);
          }
        }
        if (vnext.empty()) force_stop = true;
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
    if (k == vertexes_to_cover.size()) return TBase::Search();
    unsigned bestk = vertexes_to_cover.size(), bestkvalue = used_vertices.SetSize() + 1;
    if (k == 0) {
      int64_t max_distance = 0;
      for (unsigned ik = 0; ik < vertexes_to_cover.size(); ++ik) {
        auto p = vertexes_to_cover[ik];
        int64_t sd = 0;
        for (unsigned j = 0; j < vertexes_to_cover.size(); ++j)
          sd += cache.SegmentHoleDistance({p, vertexes_to_cover[j]});
        if (max_distance < sd) {
          max_distance = sd;
          bestk = ik;
        }
      }
    } else 
    {
      for (unsigned ik = 0; ik < vertexes_to_cover.size(); ++ik) {
        if (covered_vertexes.HasKey(ik)) continue;
        auto p = vertexes_to_cover[ik];
        unsigned count = 0;
        for (unsigned i = 0; i < used_vertices.SetSize(); ++i) {
          if (used_vertices.HasKey(i)) continue;
          auto& v = valid_candidates[i][valid_candidates_index[i]];
          if (v.size() == 0) return false;
          bool is_valid = false;
          for (auto& vp : v) {
            if (vp == p) {
              is_valid = true;
              break;
            }
          }
          if (is_valid) ++count;
        }
        if (count < bestkvalue) {
          bestkvalue = count;
          bestk = ik;
        }
      }
    }
    // std::cout << "Best k\t" << bestk << "\t" << bestkvalue << std::endl;
    if (bestkvalue == 0) return false;

    auto p = vertexes_to_cover[bestk];
    covered_vertexes.Insert(bestk);
    for (unsigned i = 0; i < used_vertices.SetSize(); ++i) {
      if (used_vertices.HasKey(i)) continue;
      auto& v = valid_candidates[i][valid_candidates_index[i]];
      bool is_valid = false;
      for (auto& vp : v) {
        if (vp == p) {
          is_valid = true;
          break;
        }
      }
      if (is_valid) {
        AddPoint(i, p);
        if (!force_stop) {
          if (SearchI(k + 1)) return true;
        }
        RemoveLastPoint();
      }
    }
    covered_vertexes.RemoveLast();
    return false;
  }

 public:
  bool Search() {
    if (used_vertices.SetSize() < vertexes_to_cover.size()) return false;
    return SearchI(0);
  }
};
}  // namespace solver
