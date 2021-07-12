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

    SolutionCandidate sc;
    const auto cmHole = massCenter(p.hole);
    cerr << "Hole MC: " << cmHole << endl;
    const auto cmOriginal = massCenter(p.originalPoints);
    cerr << "Original MC: " << cmOriginal << endl;

    const auto numPoints = p.originalPoints.size();

    Poly points = p.originalPoints;

    auto goodPoly = [&](const Poly& pl) {
        for (size_t i = 0; i < numPoints; ++i) {
            auto toPoint = p.pointInsideToIndex.find(pl[i]);
            if (toPoint == p.pointInsideToIndex.end()) {
                return false;
            }
        }
        return true;
    };

    auto reflectX = [&](int x0) {
        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i].x > x0) {
                points[i].x = x0 - (points[i].x - x0);
            }
        }
    };

    auto shiftX = [&](int dx) {
        for (size_t i = 0; i < points.size(); ++i) {
            points[i].x += dx;
        }
    };

    auto shiftY = [&](int dy) {
        for (size_t i = 0; i < points.size(); ++i) {
            points[i].y += dy;
        }
    };

    auto swapXY = [&]() {
        for (size_t i = 0; i < points.size(); ++i) {
            auto& pp = points[i];
            int tempX = pp.x;
            pp.x = pp.y;
            pp.y = p.maxx - tempX;
        }
    };

    if (FLAGS_test_idx == 122) {
        swapXY();
        shiftX(-40);
        shiftY(-40);
    }

    if (FLAGS_test_idx == 132) {
        reflectX(141);
        reflectX(100);
        reflectX(59);
        shiftX(20);
        shiftY(-3);
    }

    std::ofstream fTemp("../solutions/manual2/" + std::to_string(FLAGS_test_idx) + ".temp.json");
    fTemp << p.exportPoly(points);

    sc.points.resize(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {
        auto toPoint = p.pointInsideToIndex.find(points[i]);
        if (toPoint == p.pointInsideToIndex.end()) {
            cerr << "bad point: " << points[i] << endl;
            return 1;
        }
        sc.points[i] = toPoint->second;
    }

    if (p.violationsBnd(sc)) {
        cerr << "Bad bnd" << endl;
        return 1;
    }

    if (p.violationsLen(sc)) {
        cerr << "Bad len" << endl;
        return 1;
    }

    cerr << "found " << endl;;
    std::ofstream f("../solutions/manual2/" + std::to_string(FLAGS_test_idx) + ".json");
    f << p.exportSol(sc.points);

    return 0;
}
