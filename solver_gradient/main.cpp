#include <gflags/gflags.h>

#include <iostream>
#include <random>

#include "../solver/solver.h"
#include "json.hpp"

DEFINE_int32(test_idx, 1, "Test number");
DEFINE_string(init, "", "file from initialization");
DEFINE_bool(corner, false, "file from initialization");

using namespace std;

using json = nlohmann::json;

double violationsLenSoft(const Problem& p, const SolutionCandidate& current) {
    double n = 0;
    for (int i = 0; i < p.edgeU.size(); ++i) {
        int u = p.edgeU[i];
        int v = p.edgeV[i];
        auto p1 = p.pointsInside[current.points[u]];
        auto p2 = p.pointsInside[current.points[v]];
        double distMeasure = std::abs(1.0 * dist2(p1, p2) / dist2(p.originalPoints[u], p.originalPoints[v]) - 1.0);
        distMeasure = std::max(0.0, distMeasure - p.eps - 1e-12);
        if (distMeasure > 0) {
            n = n + 1.0 + distMeasure;
        }
    }
    return n;
}

double e(const Problem& p, const SolutionCandidate& sc) {
    static constexpr double INF = 1000000.0;
    double result = (static_cast<double>(p.violationsBnd(sc) + violationsLenSoft(p, sc))) * INF;
    if (!FLAGS_corner) {
        if (result) {
            return result + INF;
        }
    }

    for (size_t h = 0; h < p.hole.size(); ++h) {
        int mind = 1000000000;
        for (size_t i = 0; i < p.originalPoints.size(); ++i) {
            mind = std::min(mind, dist2(p.hole[h], p.pointsInside[sc.points[i]]));
        }
        result += mind;
    }

    return result;
}

struct PhysicalWorld {
    struct Ball {
        double x;
        double y;
        double vx;
        double vy;
        double fx;
        double fy;

        Ball() {
            x = 0;
            y = 0;
            vx = 0;
            vy = 0;
            fx = 0;
            fy = 0;
        }

        void step(double dt) {
            vx += fx*dt;
            vy += fy*dt;
            x += vx*dt;
            y += vy*dt;
        }

        Point toPoint() const {
            return Point(x, y);
        }
    };

    struct Spring {
        int idx1;
        int idx2;
        double length0;
    };

    const Problem& p;
    vector<Ball> balls;
    vector<Spring> springs;

    PhysicalWorld(const Problem& p) : p(p) {
        balls.resize(p.originalPoints.size());
        for (size_t i = 0; i < balls.size(); ++i) {
            balls[i].x = p.originalPoints[i].x;
            balls[i].y = p.originalPoints[i].y;
        }
        springs.resize(p.edgeU.size());
        for (size_t i = 0; i < springs.size(); ++i) {
            auto& spring = springs[i];
            spring.idx1 = p.edgeU[i];
            spring.idx2 = p.edgeV[i];
            spring.length0 = dist(p.originalPoints[spring.idx1], p.originalPoints[spring.idx2]);
        }
    }

    void initFromCandidate(const SolutionCandidate& sc) {
        for (size_t i = 0; i < balls.size(); ++i) {
            auto& ball = balls[i];
            const auto& pt = p.pointsInside[sc.points[i]];
            ball.x = pt.x;
            ball.y = pt.y;
            ball.vx = 0;
            ball.vy = 0;
            ball.fx = 0;
            ball.fy = 0;
        }
    }

    void updateCandidate(SolutionCandidate& sc) {
        for (size_t i = 0; i < balls.size(); ++i) {
            const auto& ball = balls[i];
            auto toPoint = p.pointInsideToIndex.find(ball.toPoint());
            if (toPoint != p.pointInsideToIndex.end()) {
                sc.points[i] = toPoint->second;
            }
        }
    }

    void step(double dt) {
        for (auto& b: balls) {
            b.fx = 0;
            b.fy = 0;
        }
        for (size_t i = 0; i < springs.size(); ++i) {
            const auto& spring = springs[i];
            auto& ball1 = balls[spring.idx1];
            auto& ball2 = balls[spring.idx2];
            const double length = dist(ball1, ball2);
            const double force = length - spring.length0;
            const double px = (ball1.x - ball2.x) / length;
            const double py = (ball1.y - ball2.y) / length;
            ball1.fx += -px * force;
            ball1.fy += -py * force;
            ball2.fx +=  px * force;
            ball2.fy +=  py * force;
        }
        for (auto& b: balls) {
            b.step(dt);
        }
    }

    void friction(double dt) {
        double mul = exp(-dt);
        for (auto& b: balls) {
            b.vx *= mul;
            b.vy *= mul;
        }
    }
};

ostream& operator<<(ostream& s, const PhysicalWorld::Ball& b) {
    s << "(" << b.x << ", " << b.y << ")";
    return s;
}

void testPsysics() {
    Problem p;
    p.originalPoints = {{-1, 0}, {1, 0}};
    p.edgeU = {0};
    p.edgeV = {1};
    PhysicalWorld pw(p);
    pw.balls[0].x = -2;
    pw.balls[1].x = 2;
    for (size_t i = 0; i < 100; ++i) {
        pw.step(0.1);
        cout << pw.balls[0] << " " << pw.balls[1] << endl;
        pw.friction(0.1);
    }
}

int main(int argc, char* argv[]) {
    // testPsysics();
    // return 0;

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
            init.setInitialCandidate(initial);
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

    PhysicalWorld pw(p);

    double bestE = 1e100;
    static constexpr double INVALID_E = -1;
    const int numPoints = population[0].points.size();
    std::uniform_int_distribution<int> deltaDistr10(-10, 10);
    std::uniform_int_distribution<int> pointDistr(0, numPoints - 1);
    std::uniform_int_distribution<int> candDistr(0, NUM_CANDIDATES);
    std::uniform_int_distribution<int> distr100(0, 100);
    std::uniform_int_distribution<int> insideDistr(0, p.pointsInside.size() - 1);
    std::uniform_int_distribution<int> deltaDistr3(-3, 3);
    std::uniform_int_distribution<int> edgeDistr(0, p.edgeU.size() - 1);
    std::uniform_int_distribution<int> xDistr(p.minx, p.maxx);
    std::uniform_int_distribution<int> yDistr(p.miny, p.maxy);
    std::uniform_int_distribution<int> cornersDistr(0, p.corners.size() - 1);
    std::uniform_int_distribution<int> distrRadius(5, 100);
    std::uniform_int_distribution<int> distr10(1, 10);
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
                        newC.optE = INVALID_E;
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
                newC.optE = INVALID_E;
                population.emplace_back(newC);
            }
        };

        move(-1, 0);
        move(1, 0);
        move(0, -1);
        move(0, 1);

        // crossingover
        for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
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
            newC.optE = INVALID_E;
            population.emplace_back(newC);
        }

        for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
            auto idx1 = candDistr(gen);
            SolutionCandidate newC = population[idx1];
            newC.points[pointDistr(gen)] = insideDistr(gen);
            newC.optE = INVALID_E;
            population.emplace_back(newC);
        }

        for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
            auto idx1 = candDistr(gen);
            SolutionCandidate newC = population[idx1];
            newC.points[pointDistr(gen)] = p.corners[cornersDistr(gen)];
            newC.optE = INVALID_E;
            population.emplace_back(newC);
        }

        auto groupMove = [&](int rad, int dx, int dy) {
            for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
                auto idx1 = candDistr(gen);
                SolutionCandidate newC = population[idx1];
                auto idx2 = pointDistr(gen);
                const Point center = p.pointsInside[idx2];
                for (auto& point : newC.points) {
                    auto d2 = dist2(center, p.pointsInside[point]);
                    if (d2 < rad * rad) {
                        Point np = p.pointsInside[point];
                        np.x += dx;
                        np.y += dy;
                        auto toNewPoint = p.pointInsideToIndex.find(np);
                        if (toNewPoint != p.pointInsideToIndex.end()) {
                            point = toNewPoint->second;
                        }
                    }
                }
                newC.optE = INVALID_E;
                population.emplace_back(newC);
            }
        };
        groupMove(distrRadius(gen), -distr10(gen), 0);
        groupMove(distrRadius(gen), distr10(gen), 0);
        groupMove(distrRadius(gen), 0, -distr10(gen));
        groupMove(distrRadius(gen), 0, distr10(gen));

        for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
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
            newC.optE = INVALID_E;
            population.emplace_back(newC);
        }

        for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
            auto idx3 = candDistr(gen);
            SolutionCandidate newC = population[idx3];
            auto idx1 = pointDistr(gen);
            auto idx2 = pointDistr(gen);
            if (idx1 != idx2) {
                swap(newC.points[idx1], newC.points[idx2]);
                newC.optE = INVALID_E;
                population.emplace_back(newC);
            }
        }

        auto addSpringSimulation = [&](double dt, int steps) {
            for (size_t i = 0; i < NUM_CANDIDATES * 10; ++i) {
                auto idx3 = candDistr(gen);
                SolutionCandidate newC = population[idx3];
                pw.initFromCandidate(newC);

                for (size_t j = 0; j < steps; ++j) {
                    pw.step(dt);
                }

                pw.updateCandidate(newC);

                newC.optE = INVALID_E;
                population.emplace_back(newC);
            }
        };

        addSpringSimulation(0.01, 5);
        addSpringSimulation(0.1, 5);
        addSpringSimulation(1, 5);
        addSpringSimulation(10, 5);

        sort(population.begin(), population.end(), [](const auto& a, const auto& b) { return a.points < b.points; });
        auto toUnique = unique(population.begin(), population.end());
        const size_t dups = population.end() - toUnique;
        if (toUnique < population.begin() + NUM_CANDIDATES) {
            toUnique = population.begin() + NUM_CANDIDATES;
        }
        population.erase(toUnique, population.end());

        jobs.clear();
        static const size_t NUM_THREADS = 16;

        auto calcScores = [&](size_t iThread) {
            size_t begin = (iThread * population.size()) / NUM_THREADS;
            size_t end = ((iThread + 1) * population.size()) / NUM_THREADS;
            for (size_t j = begin; j < end; ++j) {
                if (population[j].optE == INVALID_E) {
                    population[j].optE = e(p, population[j]);
                }
            }
        };

        for (size_t i = 0; i < NUM_THREADS; ++i) {
            jobs.emplace_back(std::async(std::launch::async, calcScores, i));
        }
        for (auto& f : jobs) {
            f.wait();
        }

        sort(population.begin(), population.end(), [](const auto& a, const auto& b) { return a.optE < b.optE; });

        population.erase(population.begin() + NUM_CANDIDATES, population.end());
        cerr << iGen << ": " << population.front().optE << " - " << population.back().optE << "["
             << p.violationsBnd(population.front()) << ", " << p.violationsLen(population.front()) << ", "
             << violationsLenSoft(p, population.front()) << "] "
             << " dups: " << dups << endl;

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
