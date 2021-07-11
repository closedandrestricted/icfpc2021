#include <iostream>

#include "../solver/solver.h"

#include "json.hpp"

#include <gflags/gflags.h>

DEFINE_int32(test_idx, 1, "Test number");

using namespace std;

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Problem p;
    std::cerr << FLAGS_test_idx << " ";
    p.parseJson("../problems/" + std::to_string(FLAGS_test_idx) + ".json");
    p.preprocess(false);
    cerr << endl;

    vector<SolutionCandidate> population;

    std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
    std::vector<int> points;
    f << p.exportSol(points);

    return 0;
}
