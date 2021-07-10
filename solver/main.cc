#include "solver.h"
#include <gflags/gflags.h>

DEFINE_int32(test_idx, 1, "Test number");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Problem p;
    std::cerr << FLAGS_test_idx << " ";
    p.parseJson("problems/" + std::to_string(FLAGS_test_idx) + ".json");
    p.preprocess();
    GibbsChain mcmc(p, false, 100.0);
    SolutionCandidate sol0;
    sol0.points.assign(p.originalPoints.size(), 0);
    mcmc.init(sol0);
    while (true) {
        mcmc.step();
        if (mcmc.current.constE == 0.0) {
            std::cerr << mcmc.current.constE << "\n";
        }
    }
    return 0;
}