#pragma once

#include "common/icfpc2021/task.h"
#include "common/geometry/d2/utils/box.h"

#include <iostream>
#include <string>

inline void PrintTasksInfo() {
  for (unsigned i = 1; i < 107; ++i) {
    std::string s = "problems/" + std::to_string(i) + ".json";
    Task t;
    t.Load(s);
    auto b = Box(t.hole.v);
    std::cout << i << "\t" << t.hole.Size() << "\t" << t.g.Size() << "\t" << t.g.EdgesSize() 
    << "\t" << b.p2.x - b.p1.x + 1 << "\t" << b.p2.y - b.p1.y + 1 << std::endl;
  }
}
