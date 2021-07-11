#pragma once

#include "common/geometry/d2/point.h"
#include "common/icfpc2021/solver/perfect_score.h"
#include "common/icfpc2021/solution.h"
#include "common/icfpc2021/task.h"

#include <vector>

namespace solver {
class BonusHunting : public PerfectScore {
 public:
  using TBase = PerfectScore;

 protected:
  unsigned task_id;

 public:
  BonusHunting(const Task& _task, unsigned _task_id) : PerfectScore(_task), task_id(_task_id) {}

  void Search() {
    unsigned n = task.bonuses_to_unlock.size(), p2n = (1u << n);
    std::vector<unsigned> vf(p2n, 0);
    std::vector<I2Point> vt = task.hole.v;
    for (unsigned i = 0; i < p2n; ++i) {
      vt.resize(task.hole.Size());
      bool b;
      if (i == 0) {
        TBase::ResetSearch(vt);
        std::cout << "Solving T" << task_id << " with BM = " << i << std::endl;
        b = TBase::Search();
        std::cout << "Done. " << b << std::endl;
        vf[i] = b;
      } else {
        unsigned j = 0;
        for (; (i & (1u << j)) == 0;) ++j;
        unsigned ia = i ^ (1u << j);
        if (!vf[ia]) continue; // No solution for subset
        for (; (1u << j) <= i; ++j) {
          if (i & (1u << j)) {
            I2Point pnew = task.bonuses_to_unlock[j];
            bool duplicate = false;
            for (auto p : vt) {
              if (p == pnew) {
                duplicate = true;
                break;
              }
            }
            if (!duplicate) {
              vt.push_back(pnew);
            }
          }
        }
        TBase::ResetSearch(vt);
        std::cout << "Solving T" << task_id << " with BM = " << i << std::endl;
        b = TBase::Search();
        std::cout << "Done. " << b << std::endl;
        vf[i] = b;
      }
      if (b) {
        auto v = GetSolution();
        Solution s{v};
        auto js = s.ToJson();
        auto filename = "solutions/soptimal/" + std::to_string(task_id) + (i ? "_" + std::to_string(i) : "")  + ".json";
        std::ofstream of(filename);
        of << js;
      }
    }
  }
};
}  // namespace solver
