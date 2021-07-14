#pragma once

#include <cmath>

class Stat {
 public:
  unsigned n = 0;
  double s = 0;
  // double s2 = 0;

  void Add(double x) {
      ++n;
      s += x;
      // s2 += x * x;
  }

  double Score(double log_n_total, double max_score) {
    static const double c = sqrt(2.0);
    if (n == 0) return (c + 1) * max_score;
    // return sqrt(s2 / n) + s / n + c * sqrt(log_n_total / n);
    return s / n + c * max_score * sqrt(log_n_total / n);
  }
};
