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

    sc.points.resize(p.originalPoints.size());
    for (int shiftX = -1000; shiftX < 1000; ++shiftX) {
        cerr << "..." << shiftX << endl;
        for (int shiftY = -1000; shiftY < 1000; ++shiftY) {
            for (int trans = 0; trans < 2; ++trans) {
                auto good = [&](int dx, int dy, bool trans) {
                    for (size_t i = 0; i < p.originalPoints.size(); ++i) {
                        const Point op = p.originalPoints[i];
                        Point np = op;
                        if (!trans) {
                            np.x = op.x + dx;
                            np.y = op.y + dy;
                        } else {
                            np.y = op.x + dx;
                            np.x = op.y + dy;
                        }
                        auto toPoint = p.pointInsideToIndex.find(np);
                        if (toPoint != p.pointInsideToIndex.end()) {
                            sc.points[i] = toPoint->second;
                        } else {
                            // cerr << "No solution " << i << " (" << np.x << ", " << np.y << ")" << endl;
                            return false;
                        }
                    }

                    if (p.violationsBnd(sc)) {
                        return false;
                    }

                    if (p.violationsLen(sc)) {
                        return false;
                    }

                    cerr << "found " << dx << " " << dy << endl;
                    std::ofstream f("../solutions/move/" + std::to_string(FLAGS_test_idx) + ".json");
                    f << p.exportSol(sc.points);

                    return true;
                };

                if (good(shiftX, shiftY, trans)) {
                    return 0;
                }
            }
        }
    }

    return 0;
}
