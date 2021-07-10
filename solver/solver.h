#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <future>

#include <boost/dynamic_bitset.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

bool isect(Point ua, Point ub, Poly poly) {
    for (size_t i = 0; i < poly.size(); ++i) {
        const auto& a = poly[i];
        const auto& b = poly[i + 1 == poly.size() ? 0 : i + 1];
        if (isect(ua, ub, a, b)) {
            return true;
        }
        if (vmul(ua - b, ub - b) == 0) {
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
        }
    }
    return false;
}

struct Problem {
    Poly hole;
    std::vector<Point> originalPoints;
    std::vector<std::vector<int>> adjEdgeIds;
    std::vector<int> edgeU, edgeV;
    int eps;

    void parseJson(const std::string& fn) {
        std::ifstream is(fn);
        json rawProblem;
        is >> rawProblem;
        eps = rawProblem["epsilon"];
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
        for (int x = minx; x <= maxx; ++x) {
            for (int y = miny; y <= maxy; ++y) {
                Point p(x, y);
                if (inside(p, hole)) {
                    pointsInside.push_back(p);
                }
            }
        }
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
        for (int block = 0; block < pointsInside.size(); block += BLOCK_SIZE) {
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
};

struct EdgeInfo {
    int64_t constraintE;
};

struct SolutionCandidate {
    const Problem& problem;

    int64_t constraintE, optE;
    std::vector<Point> points;
    std::vector<EdgeInfo> edges;
};
