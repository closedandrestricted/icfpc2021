#include <iostream>
#include <random>

#include "../solver/solver.h"

#include "json.hpp"

#include <gflags/gflags.h>

DEFINE_int32(test_idx, 1, "Test number");

using namespace std;

using json = nlohmann::json;

double e(const Problem& p, SolutionCandidate& sc) {
    double result = 0;

    for (size_t h = 0; h < p.hole.size(); ++h) {
        int mind = 1000000000;
        for (size_t i = 0; i < p.originalPoints.size(); ++i) {
            mind = std::min(mind, dist2(p.hole[h], p.pointsInside[sc.points[i]]));
        }
        result += mind;
    }

    return result;
}

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
        Initer init(p);
        init.current.points = c.points;
        while (!init.step()) {
        }
        c.points = init.current.points;
        c.optE = e(p, c);
        cerr << c.optE << endl;
    }

    std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
    f << p.exportSol(population[0].points);

    /*
    std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
    f << p.exportSol(population[0].points);
    */

    return 0;
}
