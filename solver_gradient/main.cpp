#include <iostream>
#include <random>

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

    double minOptE = 1e100;

    static constexpr size_t NUM_CANDIDATES = 100;
    vector<SolutionCandidate> population(NUM_CANDIDATES);
    for (auto& c : population) {
        vector<int> idxs(p.pointsInside.size());
        for (int i = 0; i < p.pointsInside.size(); ++i) {
            idxs[i] = i;
        }
        std::shuffle(idxs.begin(), idxs.end(), gen);
        c.points.resize(p.originalPoints.size());
        for (int i = 0; i < c.points.size(); ++i) {
            c.points[i] = idxs[i];
        }
    }

    Initer init(p, 0.8);
    cerr << "points1: " << init.current.points.size() << endl;
    while (!init.step()) {

    }

    cerr << "points2: " << init.current.points.size() << endl;

    std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
    f << p.exportSol(init.current.points);

    population[0].points = init.current.points;

    /*
    std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
    f << p.exportSol(population[0].points);
    */

    return 0;
}
