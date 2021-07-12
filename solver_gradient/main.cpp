#include <iostream>
#include <random>

#include "../solver/solver.h"

#include "json.hpp"

#include <gflags/gflags.h>

DEFINE_int32(test_idx, 1, "Test number");
DEFINE_string(init, "", "file from initialization");

using namespace std;

using json = nlohmann::json;

double e(const Problem& p, const SolutionCandidate& sc) {
    double result = 0;

    for (size_t h = 0; h < p.hole.size(); ++h) {
        int mind = 1000000000;
        for (size_t i = 0; i < p.originalPoints.size(); ++i) {
            mind = std::min(mind, dist2(p.hole[h], p.pointsInside[sc.points[i]]));
        }
        result += mind;
    }

    return result + static_cast<double>(p.violationsBnd(sc) + p.violationsLen(sc)) * 1000000.0;
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
    std::vector<std::future<void>> jobs;
    auto genRandomCandidate = [&](size_t i) {
        auto& c = population[i];
        Initer init(p);
        if (FLAGS_init.size()) {
            std::ifstream is(FLAGS_init);
            json webedit_solution;
            is >> webedit_solution;
            std::vector<Point> initial;
            initial.resize(webedit_solution["vertices"].size());
            for (size_t i = 0; i < initial.size(); ++i) {
                initial[i] = {webedit_solution["vertices"][i][0], webedit_solution["vertices"][i][1]};
            }
            init.set_initial_candidate(initial);
        } else {
            vector<int> idxs(p.pointsInside.size());
            for (int i = 0; i < p.pointsInside.size(); ++i) {
                idxs[i] = i;
            }
            std::shuffle(idxs.begin(), idxs.end(), gen);
            c.points.resize(p.originalPoints.size());
            for (int i = 0; i < c.points.size(); ++i) {
                c.points[i] = idxs[i];
            }
            init.current.points = c.points;
            while (!init.step()) {
            }
        }
        c.points = init.current.points;
        c.optE = e(p, c);
        cerr << i << "/" << NUM_CANDIDATES << " " << c.optE << endl;
    };
    for (size_t i = 0; i < NUM_CANDIDATES; ++i) {
        jobs.emplace_back(std::async(std::launch::async, genRandomCandidate, i));
    }
    for (auto& f : jobs) {
        f.wait();
    }

    sort(population.begin(), population.end(), [](const auto& a, const auto& b) { return a.optE < b.optE; });

    auto save = [&]() {
        std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
        f << p.exportSol(population[0].points);
    };

    double bestE = 1e100;
    const int numPoints = population[0].points.size();
    std::uniform_int_distribution<int> deltaDistr10(-10, 10);
    std::uniform_int_distribution<int> pointDistr(0, numPoints - 1);
    std::uniform_int_distribution<int> candDistr(0, NUM_CANDIDATES);
    std::uniform_int_distribution<int> distr100(0, 100);
    std::uniform_int_distribution<int> insideDistr(0, p.pointsInside.size() - 1);
    std::uniform_int_distribution<int> deltaDistr3(-3, 3);
    std::uniform_int_distribution<int> edgeDistr(0, p.edgeU.size() - 1);
    for (int iGen = 0; iGen < 1000; ++iGen) {
        auto shake = [&](auto& distr) {
            for (size_t i = 0; i < NUM_CANDIDATES; ++i) {
                for (size_t j = 0; j < 10; ++j) {
                    int dx = distr(gen);
                    int dy = distr(gen);
                    int idxPoint = pointDistr(gen);
                    Point newPoint(p.pointsInside[population[i].points[idxPoint]]);
                    newPoint.x += dx;
                    newPoint.y += dy;
                    auto toNewPoint = p.pointInsideToIndex.find(newPoint);
                    if (toNewPoint != p.pointInsideToIndex.end()) {
                        SolutionCandidate newC = population[i];
                        newC.points[idxPoint] = toNewPoint->second;
                        newC.optE = e(p, newC);
                        population.emplace_back(newC);
                    }
                }
            }
        };

        shake(deltaDistr10);
        shake(deltaDistr3);

        auto move = [&](int dx, int dy) {
            for (size_t i = 0; i < NUM_CANDIDATES; ++i) {
                SolutionCandidate newC = population[i];
                for (size_t j = 0; j < numPoints; ++j) {
                    Point newPoint(p.pointsInside[population[i].points[j]]);
                    newPoint.x += dx;
                    newPoint.y += dy;
                    auto toNewPoint = p.pointInsideToIndex.find(newPoint);
                    if (toNewPoint != p.pointInsideToIndex.end()) {
                        newC.points[j] = toNewPoint->second;
                    }
                }
                newC.optE = e(p, newC);
                population.emplace_back(newC);
            }
        };

        move(-1, 0);
        move(1, 0);
        move(0, -1);
        move(0, 1);

        for (size_t i = 0; i < NUM_CANDIDATES*10; ++i) {
            auto idx1 = candDistr(gen);
            auto idx2 = candDistr(gen);
            if (idx1 == idx2) {
                continue;
            }
            SolutionCandidate newC = population[idx1];
            for (size_t j = 0; j < numPoints; ++j) {
                if (distr100(gen) < 20) {
                    newC.points[j] = population[idx2].points[j];
                }
            }
            newC.optE = e(p, newC);
            population.emplace_back(newC);
        }

        for (size_t i = 0; i < NUM_CANDIDATES*10; ++i) {
            auto idx1 = candDistr(gen);
            SolutionCandidate newC = population[idx1];
            newC.points[pointDistr(gen)] = insideDistr(gen);
            newC.optE = e(p, newC);
            population.emplace_back(newC);
        }

        for (size_t i = 0; i < NUM_CANDIDATES*10; ++i) {
            auto idx1 = edgeDistr(gen);
            Line l(p.pointsInside[p.edgeU[idx1]], p.pointsInside[p.edgeV[idx1]]);
            auto idx2 = candDistr(gen);
            SolutionCandidate newC = population[idx2];
            for (size_t j = 0; j < numPoints; ++j) {
                const Point oldPoint = p.pointsInside[newC.points[j]];
                if (l.sdist(oldPoint) > 0) {
                    Point newPoint(l.reflect(oldPoint));
                    auto toNewPoint = p.pointInsideToIndex.find(newPoint);
                    if (toNewPoint != p.pointInsideToIndex.end()) {
                        newC.points[j] = toNewPoint->second;
                    } else {
                        newC.points[j] = population[idx2].points[j];
                    }
                }
            }
        }

        sort(population.begin(), population.end(), [](const auto& a, const auto& b) { return a.optE < b.optE; });
        population.erase(population.begin() + NUM_CANDIDATES, population.end());
        cerr << iGen << ": " << population.front().optE << " - " << population.back().optE << endl;

        if (population[0].optE < bestE) {
            bestE = population[0].optE;
            save();
            if (!bestE) {
                return 0;
            }
        }
    }


    /*
    std::ofstream f("../solutions/gradient/" + std::to_string(FLAGS_test_idx) + ".json");
    f << p.exportSol(population[0].points);
    */

    return 0;
}
