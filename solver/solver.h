#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <future>
#include <random>

#include <boost/dynamic_bitset.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::random_device r;
std::mt19937 gen(r());

struct Point {
    int x, y;

    Point(): x(0), y(0) {}

    Point(int x, int y): x(x), y(y) {}

    Point operator-(const Point& o) const {
        return {x - o.x, y - o.y};
    }

    Point operator+(const Point& o) const {
        return {x + o.x, y + o.y};
    }

    bool operator==(const Point& o) const {
        return x == o.x && y == o.y;
    }
};

std::ostream& operator<<(std::ostream& o, const Point& p) {
    o << "<" << p.x << ", " << p.y << ">";
    return o;
}

using Poly = std::vector<Point>;

int vmul(const Point& u, const Point& v) {
    return u.x * v.y - u.y * v.x;
}

int smul(const Point& u, const Point& v) {
    return u.x * v.x + u.y * v.y;
}

int dist2(Point a, Point b) {
    return smul(a - b, a - b);
}
    
int signum(int a) {
    return a > 0 ? 1 : a == 0 ? 0 : -1;
}

bool inside(Point p, const Poly& poly) {
    bool ret = false;
    for (size_t i = 0; i < poly.size(); ++i) {
        const auto& u = poly[i];
        const auto& v = poly[i + 1 == poly.size() ? 0 : i + 1];
        if (u == p) {
            return true;
        }
        if (u.y == p.y && v.y == p.y && signum(u.x - p.x) * signum(v.x - p.x) <= 0) {
            return true;
        }
        if ((u.y > p.y) != (v.y > p.y)) {
            int slope = vmul(u - p, v - p);
            if (slope == 0) {
                return true;
            }
            ret ^= (slope > 0) == (u.y <= p.y);
        }
    }
    return ret;
}

bool isect(Point ua, Point ub, Point va, Point vb) {
    return
        signum(vmul(ub - ua, va - ua)) * signum(vmul(ub - ua, vb - ua)) < 0 &&
        signum(vmul(vb - va, ua - va)) * signum(vmul(vb - va, ub - va)) < 0;
}

bool between(Point a, Point mid, Point b) {
    auto v1 = a - mid;
    auto v2 = b - mid;
    return vmul(v1, v2) == 0 && smul(v1, v2) <= 0;
}

bool isect(Point ua, Point ub, Poly poly) {
    for (size_t i = 0; i < poly.size(); ++i) {
        const auto& a = poly[i];
        const auto& b = poly[i + 1 == poly.size() ? 0 : i + 1];
        if (between(ua, b, ub)) {
            const auto& c = poly[i + 2 >= poly.size() ? i + 2 - poly.size() : i + 2];
            auto interior = [&](Point p) {
                if (vmul(c - b, a - b) >= 0) {
                    return vmul(c - b, p - b) >= 0 && vmul(p - b, a - b) >= 0;
                } else {
                    return vmul(c - b, p - b) >= 0 || vmul(p - b, a - b) >= 0;
                }
            };
            if (!interior(ua) || !interior(ub)) {
                return true;
            }
        } else if(between(ua, a, ub)) {
            continue;
        } else if(between(a, ua, b)) {
            if (vmul(a - b, ub - b) >= 0) {
                return true;
            }
        } else if(between(a, ub, b)) {
            if (vmul(a - b, ua - b) >= 0) {
                return true;
            }
        } else if (isect(ua, ub, a, b)) {
            return true;
        }
    }
    return false;
}

struct SolutionCandidate {
    std::vector<int> points;
    double constE = 0.0, optE = 0.0;
};

struct Problem {
    Poly hole;
    std::vector<Point> originalPoints;
    std::vector<std::vector<int>> adjEdgeIds;
    std::vector<int> edgeU, edgeV;
    double eps;

    void parseJson(const std::string& fn) {
        std::ifstream is(fn);
        json rawProblem;
        is >> rawProblem;
        eps = int(rawProblem["epsilon"]) / 1000000.0;
        hole.resize(rawProblem["hole"].size());
        for (size_t i = 0; i < hole.size(); ++i) {
            hole[i] = {rawProblem["hole"][i][0], rawProblem["hole"][i][1]};
        }
        auto figure = rawProblem["figure"];
        size_t n = figure["vertices"].size();
        originalPoints.resize(n);
        adjEdgeIds.resize(n);
        for (size_t i = 0; i < n; ++i) {
            originalPoints[i] = {figure["vertices"][i][0], figure["vertices"][i][1]};
        }
        for (auto e : figure["edges"]) {
            int u = e[0], v = e[1];
            adjEdgeIds[u].push_back(edgeU.size());
            adjEdgeIds[v].push_back(edgeU.size());
            edgeU.push_back(u);
            edgeV.push_back(v);
        }
    }

    std::vector<Point> pointsInside;
    std::vector<uint8_t> pointsInsideIsCorner;
    std::vector<boost::dynamic_bitset<>> visibility;

    void preprocess() {
        int sum = 0;
        for (size_t i = 0; i < hole.size(); ++i) {
            const auto& a = hole[i];
            const auto& b = hole[i + 1 == hole.size() ? 0 : i + 1];
            sum += vmul(a, b);
        }
        if (sum < 0) {
            std::reverse(hole.begin(), hole.end());
        }
        int minx = hole[0].x, maxx = hole[0].x, miny = hole[0].y, maxy = hole[0].y;
        for (const auto& p : hole) {
            minx = std::min(minx, p.x);
            maxx = std::max(maxx, p.x);
            miny = std::min(miny, p.y);
            maxy = std::max(maxy, p.y);
        }
        pointsInside.resize(0);
        pointsInsideIsCorner.resize(0);
        for (int x = minx; x <= maxx; ++x) {
            for (int y = miny; y <= maxy; ++y) {
                Point p(x, y);
                if (inside(p, hole)) {
                    pointsInside.push_back(p);
                    pointsInsideIsCorner.push_back(std::find(hole.begin(), hole.end(), p) != hole.end());
                }
            }
        }
        // while (true) {
        //     std::cerr << isect(Point(45, 53), Point(38, 55), hole) << std::endl;
        //     throw 42;
        // }
        visibility.assign(pointsInside.size(), {});
        std::atomic<int> edges = 0;
        auto dojob = [&](int i) {
            visibility[i].resize(pointsInside.size());
            auto p = pointsInside[i];
            for (size_t j = 0; j < pointsInside.size(); ++j) {
                auto q = pointsInside[j];
                if (!isect(p, q, hole)) {
                    visibility[i].set(j);
                    edges++;
                }
            }
        };
        constexpr int BLOCK_SIZE = 64;
        for (size_t block = 0; block < pointsInside.size(); block += BLOCK_SIZE) {
            std::vector<std::future<void>> jobs;
            for (size_t i = block; i < pointsInside.size() && i < block + BLOCK_SIZE; ++i) {
                jobs.emplace_back(std::async(std::launch::async, dojob, i));
            }
            for (auto& f : jobs) {
                f.wait();
            }
        }
        std::cerr << pointsInside.size() << " " << edges << "\n";
    }

    json exportSol(const std::vector<int> ps) {
        json sol;
        for (auto i : ps) {
            const auto& p = pointsInside[i];
            sol["vertices"].push_back({p.x, p.y});
        }
        return sol;
    }

    void rec(std::vector<int>& ps, uint64_t mask, int& minOpt) {
        size_t bestAt = ps.size(), bestAtCnt = pointsInside.size() + 1;
        for (size_t at = 0; at < ps.size(); ++at) {
            if (mask & (1ULL << at)) {
                continue;
            }
            size_t cnt = 0;
            boost::dynamic_bitset<> candidates;
            candidates.resize(pointsInside.size(), true);
            for (auto e : adjEdgeIds[at]) {
                int j = edgeU[e] ^ edgeV[e] ^ at;
                if (mask & (1ULL << j)) {
                    candidates &= visibility[ps[j]];
                }
            }
            for (ps[at] = candidates.find_first(); ps[at] != candidates.npos; ps[at] = candidates.find_next(ps[at])) {
                bool good = true;
                for (auto e : adjEdgeIds[at]) {
                    if (edgeU[e] != at && (mask & (1ULL << edgeU[e])) == 0 ||
                        edgeV[e] != at && (mask & (1ULL << edgeV[e])) == 0) {
                        continue;
                    }
                    good &= std::abs(1.0 * dist2(pointsInside[ps[edgeU[e]]], pointsInside[ps[edgeV[e]]]) / dist2(originalPoints[edgeU[e]], originalPoints[edgeV[e]]) - 1.0) <= eps;
                }
                if (good) {
                    cnt++;
                }
            }
            if (cnt == 0) {
                return;
            }
            if (cnt < bestAtCnt) {
                bestAt = at;
                bestAtCnt = cnt;
            }
        }
        size_t at = bestAt;
        if (at == ps.size()) {
            int opt = 0;
            for (size_t i = 0; i < hole.size(); ++i) {
                int d = 1000000000;
                for (size_t j = 0; j < ps.size(); ++j) {
                    d = std::min(d, dist2(hole[i], pointsInside[ps[j]]));
                }
                opt += d;
            }
            if (opt < minOpt) {
                minOpt = opt;
                std::cerr << minOpt << " " << exportSol(ps) << std::endl;
            }
            return;
        }
        boost::dynamic_bitset<> candidates;
        candidates.resize(pointsInside.size(), true);
        for (auto e : adjEdgeIds[at]) {
            int j = edgeU[e] ^ edgeV[e] ^ at;
            if (mask & (1ULL << j)) {
                candidates &= visibility[ps[j]];
            }
        }
        std::vector<int> cs;
        for (int i = candidates.find_first(); i != candidates.npos; i = candidates.find_next(i)) {
            cs.push_back(i);
        }
        std::shuffle(cs.begin(), cs.end(), gen);
        // for (ps[at] = candidates.find_first(); ps[at] != candidates.npos; ps[at] = candidates.find_next(ps[at])) {
        for (int x : cs) {
            ps[at] = x;
            // if (!pointsInsideIsCorner[ps[at]]) {
            //     continue;
            // }
            bool good = true;
            for (auto e : adjEdgeIds[at]) {
                if (edgeU[e] != at && (mask & (1ULL << edgeU[e])) == 0 ||
                    edgeV[e] != at && (mask & (1ULL << edgeV[e])) == 0) {
                    continue;
                }
                good &= visibility[ps[edgeU[e]]][ps[edgeV[e]]];
                good &= std::abs(1.0 * dist2(pointsInside[ps[edgeU[e]]], pointsInside[ps[edgeV[e]]]) / dist2(originalPoints[edgeU[e]], originalPoints[edgeV[e]]) - 1.0) <= eps;
            }
            if (good) {
                rec(ps, mask | (1ULL << at), minOpt);
            }
        }
        // for (ps[at] = candidates.find_first(); ps[at] != candidates.npos; ps[at] = candidates.find_next(ps[at])) {
        //     if (pointsInsideIsCorner[ps[at]]) {
        //         continue;
        //     }
        //     bool good = true;
        //     for (auto e : adjEdgeIds[at]) {
        //         if (edgeU[e] != at && (mask & (1ULL << edgeU[e])) == 0 ||
        //             edgeV[e] != at && (mask & (1ULL << edgeV[e])) == 0) {
        //             continue;
        //         }
        //         good &= visibility[ps[edgeU[e]]][ps[edgeV[e]]];
        //         good &= std::abs(1.0 * dist2(pointsInside[ps[edgeU[e]]], pointsInside[ps[edgeV[e]]]) / dist2(originalPoints[edgeU[e]], originalPoints[edgeV[e]]) - 1.0) <= eps;
        //     }
        //     if (good) {
        //         rec(ps, mask | (1ULL << at), minOpt);
        //     }
        // }
    }

    void recSolve() {
        int minOpt = 1000000000;
        std::vector<int> ps(originalPoints.size(), -1);
        rec(ps, 0, minOpt);
    }
};

struct GibbsChain {
    Problem& problem;
    bool onlyFeasible;
    double invT;
    SolutionCandidate current;
    bool initialized = false;

    GibbsChain(Problem& problem, bool onlyFeasible, double invT): problem(problem), onlyFeasible(onlyFeasible), invT(invT) {
    }

    void init(SolutionCandidate initial) {
        current = initial;
        initialized = true;
    }

    double E(const SolutionCandidate& sc) {
        if (onlyFeasible && sc.constE > 0) {
            return std::numeric_limits<double>::infinity();
        }
        return (onlyFeasible ? sc.optE : sc.constE) * invT;
    }

    void step() {
        if (!initialized) {
            throw std::runtime_error("Can't step uninitialized MCMC chain");
        }
        std::vector<int> scan(problem.originalPoints.size());
        for (size_t i = 0; i < scan.size(); ++i) {
            scan[i] = i;
        }
        std::shuffle(scan.begin(), scan.end(), gen);
        std::vector<uint8_t> first(problem.edgeU.size(), true), cnt(problem.edgeU.size(), 0);
        std::vector<int> distNotI(problem.hole.size(), 1000000000);
        for (int i : scan) {
            for (auto e : problem.adjEdgeIds[i]) {
                first[e] = cnt[e]++ == 0;
            }
            boost::dynamic_bitset<> candidates;
            candidates.resize(problem.pointsInside.size(), true);
            for (auto e : problem.adjEdgeIds[i]) {
                int j = problem.edgeU[e] ^ problem.edgeV[e] ^ i;
                candidates &= problem.visibility[current.points[j]];
            }
            double selW = -std::numeric_limits<double>::infinity();
            size_t selCandidate = -1;
            if (onlyFeasible) {
                distNotI.assign(problem.hole.size(), 1000000000);
                for (size_t j = 0; j < problem.originalPoints.size(); ++j) {
                    if (i == j) {
                        continue;
                    }
                    for (size_t h = 0; h < problem.hole.size(); ++h) {
                        distNotI[h] = std::min(distNotI[h], dist2(problem.hole[h], problem.pointsInside[current.points[j]]));
                    }
                }
            }
            for (size_t curCandidate = candidates.find_first(); curCandidate != candidates.npos; curCandidate = candidates.find_next(curCandidate)) {
                double w = 0.0;
                bool skip = false;
                for (auto e : problem.adjEdgeIds[i]) {
                    int j = problem.edgeU[e] ^ problem.edgeV[e] ^ i;
                    // TODO cache denom
                    double distMeasure = std::abs(1.0 * dist2(problem.pointsInside[curCandidate], problem.pointsInside[current.points[j]]) / dist2(problem.originalPoints[i], problem.originalPoints[j]) - 1.0);
                    distMeasure = std::max(0.0, distMeasure - problem.eps - 1e-12);
                    if (onlyFeasible) {
                        if (distMeasure > 0) {
                            skip = true;
                            break;
                        }
                    } else {
                        w += distMeasure;
                    }
                }
                if (skip) {
                    continue;
                }
                if (onlyFeasible) {
                    for (size_t h = 0; h < problem.hole.size(); ++h) {
                        w -= std::max(0, distNotI[h] - dist2(problem.hole[h], problem.pointsInside[curCandidate]));
                    }
                }

                w *= -invT;
                double mx = std::max(selW, w);
                double p = 1.0 / (1.0 + std::exp(selW - w));
                if (std::uniform_real_distribution()(gen) <= p) {
                    selCandidate = curCandidate;
                }
                selW = std::log(std::exp(selW - mx) + std::exp(w - mx)) + mx;
            }
            current.points[i] = selCandidate;
        }
        current.optE = 0;
        current.constE = 0;
        for (size_t h = 0; h < problem.hole.size(); ++h) {
            int mind = 1000000000;
            for (size_t i = 0; i < problem.originalPoints.size(); ++i) {
                mind = std::min(mind, dist2(problem.hole[h], problem.pointsInside[current.points[i]]));
            }
            current.optE += mind;
        }
        for (size_t e = 0; e < problem.edgeU.size(); ++e) {
            int i = problem.edgeU[e];
            int j = problem.edgeV[e];
            // TODO cache denom
            double distMeasure = std::abs(1.0 * dist2(problem.pointsInside[current.points[i]], problem.pointsInside[current.points[j]]) / dist2(problem.originalPoints[i], problem.originalPoints[j]) - 1.0);
            distMeasure = std::max(0.0, distMeasure - problem.eps - 1e-12);
            current.constE += distMeasure;
        }
    }
};