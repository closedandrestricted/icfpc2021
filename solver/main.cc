#include "solver.h"
#include <gflags/gflags.h>

DEFINE_int32(test_idx, 1, "Test number");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Problem p;
    std::cerr << FLAGS_test_idx << " ";
    p.parseJson("problems/" + std::to_string(FLAGS_test_idx) + ".json");
    p.preprocess();
    p.recSolve();
    std::vector<double> invTs{0.0, 2.2, 5.0, 10.0, 20.0, 40.0, 100.0, 300.0, 50000.0};
    std::vector<GibbsChain> mcmcs;
    for (double invT : invTs) {
        mcmcs.emplace_back(p, false, invT);
    }
    SolutionCandidate sol0;
    sol0.points.assign(p.originalPoints.size(), 0);
    for (auto& mcmc : mcmcs) {
        mcmc.init(sol0);
    }
    std::vector<double> stats(invTs.size() - 1, 0.0);
    std::vector<double> avgE(invTs.size(), 0.0);
    for (int it = 0; ; it++) {
        if (it % 1000 == 0) {
            std::cerr << it;
            for (double x : stats) {
                std::cerr << " " << (x / 1000.0);
            }
            std::cerr << std::endl;
            stats.assign(stats.size(), 0.0);
            for (double x : avgE) {
                std::cerr << (x / 1000.0) << " ";
            }
            std::cerr << std::endl;
            avgE.assign(avgE.size(), 0.0);
        }
        auto dojob = [&](int i) {
            auto& mcmc = mcmcs[i];
            mcmc.step();
        };
        std::vector<std::future<void>> fs;
        for (size_t i = 0; i < mcmcs.size(); ++i) {
            fs.emplace_back(std::async(std::launch::async, dojob, i));
        }
        for (size_t i = 0; i < mcmcs.size(); ++i) {
            auto& mcmc = mcmcs[i];
            fs[i].wait();
            avgE[i] += mcmc.current.constE;
            if (mcmc.current.constE == 0.0) {
                std::cerr << mcmc.current.constE << "\n";
                std::ofstream f("solutions/feasible/" + std::to_string(FLAGS_test_idx) + ".json");
                f << p.exportSol(mcmc.current.points);
                return 0;
            }
        }
        for (size_t i = 0; i < mcmcs.size() - 1; ++i) {
            auto& c1 = mcmcs[i];
            auto& c2 = mcmcs[i + 1];
            double p = std::min(1.0, std::exp((c1.E(c1.current) + c2.E(c2.current)) - (c1.E(c2.current) + c2.E(c1.current))));
            if (std::uniform_real_distribution()(gen) <= p) {
                stats[i]++;
                std::swap(c1.current, c2.current);
            }
        }
    }
    return 0;
}