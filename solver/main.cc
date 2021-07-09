#include "solver.h"
#include <gflags/gflags.h>

DEFINE_int32(test_idx, 1, "Test number");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Problem p;
    std::cerr << FLAGS_test_idx << " ";
    p.parseJson("problems/" + std::to_string(FLAGS_test_idx) + ".json");
    p.preprocess();
    return 0;
}