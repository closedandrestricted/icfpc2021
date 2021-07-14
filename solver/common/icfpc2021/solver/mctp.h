#pragma once

#include "common/data_structures/unsigned_set.h"
#include "common/geometry/d2/point.h"
#include "common/icfpc2021/solution.h"
#include "common/icfpc2021/task.h"
#include "common/icfpc2021/task_cache.h"
#include "common/icfpc2021/stat.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace solver {
class MCTP {
 protected:
  Task task;
  unsigned task_id;
  std::string filename;
  TaskCache cache;
  ds::UnsignedSet used_vertices;
  std::vector<std::vector<std::vector<I2Point>>> valid_candidates;
  std::vector<unsigned> valid_candidates_index;
  std::vector<I2Point> solution;
  bool force_stop;
  std::vector<std::vector<Stat>> points_stats;
  std::vector<Stat> location_stats;
  unsigned nruns;
  double best_score;
  Solution best_solution;

 public:
  MCTP(const Task& _task, unsigned _task_id) {
    task = _task;
    task_id = _task_id;
    filename = "solutions/mctp/" + std::to_string(task_id) + ".json";
    cache.Init(task);
    nruns = 0;
    best_score = 0;
    InitSearch();
  }

 protected:
  void InitSearch() {
    unsigned size = task.g.Size();
    used_vertices.Resize(size);
    valid_candidates.clear();
    valid_candidates.resize(size);
    for (unsigned i = 0; i < size; ++i) {
      valid_candidates[i].resize(2 * size + 1); // For safety
      valid_candidates[i][0] = cache.GetValidPoints();
    }
    valid_candidates_index.clear();
    valid_candidates_index.resize(size, 0);
    solution.resize(size);
    force_stop = false;
    location_stats.clear();
    location_stats.resize(cache.MaxIndex());
    points_stats.clear();
    points_stats.resize(task.g.Size(), location_stats);
  }

  void ResetSearch() {
    used_vertices.Clear();
    std::fill(valid_candidates_index.begin(), valid_candidates_index.end(), 0);
    force_stop = false;
  }

  void UpdateStat(double score) {
    for (unsigned u = 0; u < task.g.Size(); ++u) {
      if (used_vertices.HasKey(u)) {
        auto index = cache.Index(solution[u]);
        location_stats[index].Add(score);
        points_stats[u][index].Add(score);
      }
    }
  }

  void AddPoint(unsigned index, const I2Point& p) {
    assert(!used_vertices.HasKey(index));
    used_vertices.Insert(index);
    solution[index] = p;
    for (auto e : task.g.EdgesEI(index)) {
      unsigned u = e.to;
      if (used_vertices.HasKey(u)) {
        // Verify only
        auto p1 = solution[u];
        auto d = SquaredDistanceL2(p, p1);
        if ((d < e.info.first) || (d > e.info.second) || !cache.CheckSegment(I2ClosedSegment(p, p1))) {
          std::cout << "SUS!" << std::endl;
          assert(false);    
        }
      } else {
        // Filter points
        auto& vcurrent = valid_candidates[u][valid_candidates_index[u]];
        auto& vnext = valid_candidates[u][++valid_candidates_index[u]];
        vnext.clear();
        for (auto p1 : vcurrent) {
          auto d = SquaredDistanceL2(p, p1);
          if ((d >= e.info.first) && (d <= e.info.second) && cache.CheckSegmentI(I2ClosedSegment(p, p1))) {
            vnext.push_back(p1);
          }
        }
        if (vnext.empty()) force_stop = true;
      }
    }
  }

  void RemoveLastPoint() {
    unsigned index = used_vertices.Last();
    for (auto e : task.g.EdgesEI(index)) {
      unsigned u = e.to;
      if (!used_vertices.HasKey(u)) {
        --valid_candidates_index[u];
      }
    }
    used_vertices.RemoveLast();
    force_stop = false;
  }

  void Run() {
    ResetSearch();
    double logn = log(double(++nruns));
    unsigned gsize = task.g.Size();
    for (; used_vertices.Size() < gsize;) {
      unsigned best_u = gsize;
      I2Point pnext;
      double best_score = 0.;
      if (used_vertices.Size() == 0) {
        for (auto p : cache.GetValidPoints()) {
          auto index = cache.Index(p);
          double d1 = location_stats[index].Score(logn);
          for (unsigned u = 0; u < gsize; ++u) {
            double d2 = points_stats[u][index].Score(logn);
            if (best_score < d1 + d2) {
              best_score = d1 + d2;
              best_u = u;
              pnext = p;
            }
          }
        }
      } else {
        unsigned min_size = cache.MaxIndex() + 1;
        for (unsigned u = 0; u < gsize; ++u) {
          if (used_vertices.HasKey(u)) continue;
          if (valid_candidates[u][valid_candidates_index[u]].size() < min_size) {
            min_size = valid_candidates[u][valid_candidates_index[u]].size();
            best_u = u;
          }
        }
        if (min_size == 0) break;
        for (auto p : valid_candidates[best_u][valid_candidates_index[best_u]]) {
          auto index = cache.Index(p);
          double d = points_stats[best_u][index].Score(logn);
          if (best_score < d) {
              best_score = d;
              pnext = p;
          }
        }
      }
      AddPoint(best_u, pnext);
      if (force_stop) break;
    }
    double dscore = 0.;
    if (used_vertices.Size() == gsize) {
      Solution s{solution};
      dscore = task.RawPoints(s);
      if (dscore > best_score) {
        std::cout << "New best solution: " << task.Score(s) << "\t" << nruns << std::endl;
        best_score = dscore;
        best_solution = s;
        auto js = best_solution.ToJson();
        std::ofstream of(filename);
        of << js;
      }
    }
    UpdateStat(dscore);
  }

 public:
  void Search() {
    for (;best_score < 1;) {
      Run();
    }
  }
};
}  // namespace solver
