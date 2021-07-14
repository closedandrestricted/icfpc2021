#pragma once

#include <cmath>

class Stat {
 public:
  unsigned n = 0;
  double s = 0;
  double s2 = 0;

  void Add(double x) {
      ++n;
      s += x;
      s2 += x * x;
  }

  double Score(double log_n_total) {
    static const double c = sqrt(2.0);
    if (n == 0) return 1;
    return sqrt(s2 / n) + s / n + c * sqrt(log_n_total / n);
  }
};
