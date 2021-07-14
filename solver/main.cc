#include "solver.h"
#include <gflags/gflags.h>

#include "common/icfpc2021/solver/bonus_hunting.h"
#include "common/icfpc2021/solver/mctp.h"

DEFINE_int32(test_idx, 1, "Test number");
DEFINE_bool(webedit_result, false, "Start from webedit result");
DEFINE_bool(fix_corners, false, "Fix corners from webedit result");
DEFINE_bool(alex, false, "Alex mode");
DEFINE_string(init, "", "file from initialization");
DEFINE_bool(only_border, false, "Only border");
DEFINE_bool(lazy, false, "Lazy visibility calculation");

void test_isect() {
    std::vector<Point> poly = {{0, 0}, {-2, -4}, {20, 0}, {-2, 4}};
    assert(!isect({0, 2}, {0, -2},poly ));
    assert(isect({-1, 3}, {-1, -3}, poly));
    assert(isect({-1, 2}, {-1, -2}, poly));
    assert(isect({0, 0}, {-10, -2}, poly));
    assert(isect({0, 0}, {-10, 2}, poly));
    assert(!isect({0, 0}, {-1, -3}, poly));
    assert(!isect({0, 0}, {-1, 3}, poly));
    assert(!isect({20, 0}, {18, 0}, poly));
    assert(isect({20, 0}, {18, 2}, poly));
    assert(isect({20, 0}, {18, -2}, poly));
    assert(isect({20, 0}, {22, -2}, poly));
    assert(isect({20, 0}, {22, 2}, poly));

    // std::cerr << Line({0, 0}, {0, 1}).reflect({1, 0}) << std::endl;
}

void CommonSolve(unsigned index) {
  std::string input = "problems/" + std::to_string(index) + ".json";
  Task t;
  t.Load(input);
  solver::BonusHunting slvr(t, index);
  // solver::MCTP slvr(t, index);
  slvr.Search();
}

int main(int argc, char** argv) {
    test_isect();
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::cerr << FLAGS_test_idx << " ";
    auto fn = "problems/" + std::to_string(FLAGS_test_idx) + ".json";
    if (FLAGS_alex) {
        CommonSolve(FLAGS_test_idx);
        return 0;
    } else {
        Problem p;
        p.parseJson(fn);
        p.preprocess(!FLAGS_lazy, FLAGS_only_border);
        // p.recSolve2();
        std::vector<double> invTs{0.0, 2.2, 5.0, 10.0, 20.0, 40.0, 100.0, 300.0, 50000.0};
        // std::vector<double> invTs{0.0, 0.05, 0.1, 0.2, 0.3, 0.4, 0.6, 0.9, 1.5, 2.5, 5.0, 10.0};
        std::vector<double> invTsFeasible{0.0, 0.005, 0.01, 0.05, 0.1,  0.2,  0.4,  0.8,
                                          1.6, 3.2,   6.4,  12.8, 25.6, 51.2, 102.4};
        std::vector<GibbsChain> mcmcs;
        std::vector<GibbsChain> mcmcsFeasible;
        for (double invT : invTs) {
            mcmcs.emplace_back(p, false, invT);
        }
        for (double invT : invTsFeasible) {
            mcmcsFeasible.emplace_back(p, true, invT);
        }

        Initer z(p);

        auto readIniterFromFile = [&](const std::string& file) {
            std::ifstream is(file);
            json webedit_solution;
            is >> webedit_solution;
            std::vector<Point> initial;
            initial.resize(webedit_solution["vertices"].size());
            for (size_t i = 0; i < initial.size(); ++i) {
                initial[i] = {webedit_solution["vertices"][i][0], webedit_solution["vertices"][i][1]};
            }
            z.setInitialCandidate(initial, FLAGS_fix_corners);
        };

        if (FLAGS_webedit_result) {
            auto sol_file = "solutions/webedit/" + std::to_string(FLAGS_test_idx) + ".json";
            readIniterFromFile(sol_file);
        } else if (FLAGS_init.size()) {
            readIniterFromFile(FLAGS_init);
        }
        else {
            z.setInitialCandidate(p.originalPoints, false);
            while (!z.step()){};
        }

        {
            std::ofstream f("solutions/staging/" + std::to_string(FLAGS_test_idx) + ".json");
            f << p.exportSol(z.current.points);
        }
        std::cout << "INITIALIZED" << std::endl;
        // exit(0);

        SolutionCandidate sol0 = z.current;
        // sol0.points.assign(p.originalPoints.size(), 0);

        if (FLAGS_test_idx == 68) {
            std::vector<std::pair<int, int>> fixedCorners{
                {39, 9},  {26, 10}, {17, 11}, {12, 12}, {7, 13},  {11, 14}, {19, 15}, {30, 16}, {28, 17},
                {36, 18}, {43, 19}, {41, 20}, {49, 21}, {54, 22}, {52, 23}, {55, 24}, {58, 25}, {59, 26},
                {63, 27}, {64, 28}, {62, 29}, {60, 30}, {56, 31}, {61, 32}, {57, 33}, {53, 34}, {46, 35},
                {45, 36}, {42, 37}, {50, 39}, {40, 40}, {18, 46}, {29, 47}, {25, 48}, {34, 49}, {27, 50},
                {16, 51}, {10, 52}, {9, 53},  {6, 54},  {3, 55},  {1, 56},  {0, 0},   {4, 1},   {2, 2},
                {5, 3},   {13, 4},  {22, 5},  {24, 6},  {23, 7},  {14, 8}};
            for (auto pr : fixedCorners) {
                if (p.fixed[pr.first]) {
                    std::cerr << "wtf " << pr.first << " " << pr.second << std::endl;
                }
                p.fixed[pr.first] = true;
                sol0.points[pr.first] = p.cornerToIdx(pr.second);
            }
            for (size_t e = 0; e < p.edgeU.size(); ++e) {
                int u = p.edgeU[e];
                int v = p.edgeV[e];
                if (p.fixed[u] && p.fixed[v]) {
                    double d = std::fabs(1.0 * dist2(p.pointsInside[sol0.points[u]], p.pointsInside[sol0.points[v]]) /
                                             dist2(p.originalPoints[u], p.originalPoints[v]) -
                                         1.0);
                    std::cerr << u << " " << v << " " << d << std::endl;
                }
            }
        }
        for (auto& mcmc : mcmcs) {
            mcmc.init(sol0);
        }
        std::vector<double> stats(invTs.size() - 1, 0.0);
        std::vector<double> avgE(invTs.size(), 0.0);
        std::vector<double> statsFeasible(invTsFeasible.size() - 1, 0.0);
        std::vector<double> avgEFeasible(invTsFeasible.size(), 0.0);
        double minOptE = 1e100;
        for (int it = 0;; it++) {
            if (it % 1000 == 0) {
                std::cerr << it << std::endl;
                for (double x : stats) {
                    std::cerr << (x / 1000.0) << " ";
                }
                std::cerr << std::endl;
                stats.assign(stats.size(), 0.0);
                for (double x : avgE) {
                    std::cerr << (x / 1000.0) << " ";
                }
                std::cerr << std::endl;
                avgE.assign(avgE.size(), 0.0);
                for (double x : statsFeasible) {
                    std::cerr << (x / 1000.0) << " ";
                }
                std::cerr << std::endl;
                statsFeasible.assign(statsFeasible.size(), 0.0);
                for (double x : avgEFeasible) {
                    std::cerr << (x / 1000.0) << " ";
                }
                std::cerr << std::endl;
                avgEFeasible.assign(avgEFeasible.size(), 0.0);
            }
            auto dojob = [&](int i) {
                auto& mcmc = mcmcs[i];
                mcmc.step();
            };
            std::vector<std::future<void>> fs;
            for (size_t i = 0; i < mcmcs.size(); ++i) {
                fs.emplace_back(std::async(std::launch::async, dojob, i));
            }
            if (mcmcsFeasible[0].initialized) {
                auto dojobFeasible = [&](int i) {
                    auto& mcmc = mcmcsFeasible[i];
                    mcmc.step();
                };
                std::vector<std::future<void>> fsFeasible;
                for (size_t i = 0; i < mcmcsFeasible.size(); ++i) {
                    fsFeasible.emplace_back(std::async(std::launch::async, dojobFeasible, i));
                }
                for (size_t i = 0; i < mcmcsFeasible.size(); ++i) {
                    auto& mcmc = mcmcsFeasible[i];
                    fsFeasible[i].wait();
                    avgEFeasible[i] += mcmc.current.optE;
                    if (mcmc.current.optE < minOptE) {
                        minOptE = mcmc.current.optE;
                        std::cerr << "f" << i << " " << mcmc.current.constE << " " << mcmc.current.optE << "\n";
                        std::ofstream f("solutions/staging/" + std::to_string(FLAGS_test_idx) + ".json");
                        f << p.exportSol(mcmc.current.points);
                        // return 0;
                    }
                }
                for (size_t i = 0; i < mcmcsFeasible.size() - 1; ++i) {
                    auto& c1 = mcmcsFeasible[i];
                    auto& c2 = mcmcsFeasible[i + 1];
                    double p = std::min(
                        1.0, std::exp((c1.E(c1.current) + c2.E(c2.current)) - (c1.E(c2.current) + c2.E(c1.current))));
                    if (std::uniform_real_distribution()(gen) <= p) {
                        statsFeasible[i]++;
                        std::swap(c1.current, c2.current);
                    }
                }
            }
            for (size_t i = 0; i < mcmcs.size(); ++i) {
                auto& mcmc = mcmcs[i];
                fs[i].wait();
                avgE[i] += mcmc.current.constE;
                if (mcmc.current.constE == 0.0) {
                    if (mcmc.current.optE < minOptE) {
                        minOptE = mcmc.current.optE;
                        std::cerr << mcmc.current.constE << " " << mcmc.current.optE << "\n";
                        std::ofstream f("solutions/staging/" + std::to_string(FLAGS_test_idx) + ".json");
                        f << p.exportSol(mcmc.current.points);
                    }
                    // return 0;
                    if (!mcmcsFeasible[0].initialized) {
                        for (auto& mcmcFeasible : mcmcsFeasible) {
                            mcmcFeasible.init(mcmc.current);
                        }
                    } else if (i == mcmcs.size() - 1) {
                        std::swap(mcmc.current, mcmcsFeasible[0].current);
                    }
                }
            }
            for (size_t i = 0; i < mcmcs.size() - 1; ++i) {
                auto& c1 = mcmcs[i];
                auto& c2 = mcmcs[i + 1];
                double p = std::min(
                    1.0, std::exp((c1.E(c1.current) + c2.E(c2.current)) - (c1.E(c2.current) + c2.E(c1.current))));
                if (std::uniform_real_distribution()(gen) <= p) {
                    stats[i]++;
                    std::swap(c1.current, c2.current);
                }
            }
            if (minOptE == 1e100) {
                std::ofstream f("solutions/debug/" + std::to_string(FLAGS_test_idx) + ".json");
                f << p.exportSol(mcmcs.back().current.points);
            }
        }
    }
    return 0;
}
