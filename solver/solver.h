#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <CGAL/Cartesian.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using K = CGAL::Cartesian<int>;
using Point = CGAL::Point_2<K>;
using Poly = CGAL::Polygon_2<K>;

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

    void preprocess() {
        int minx = hole[0].x(), maxx = hole[0].x(), miny = hole[0].y(), maxy = hole[0].y();
        for (const auto& p : hole) {
            minx = std::min(minx, p.x());
            maxx = std::max(maxx, p.x());
            miny = std::min(miny, p.y());
            maxy = std::max(maxy, p.y());
        }
        pointsInside.resize(0);
        for (int x = minx; x <= maxx; ++x) {
            for (int y = miny; y <= maxy; ++y) {
                Point p(x, y);
                if (CGAL::bounded_side_2(hole.begin(), hole.end(), p) != CGAL::ON_UNBOUNDED_SIDE) {
                    pointsInside.push_back(p);
                }
            }
        }
        std::vector<K::Segment_2> segments;
        for (size_t i = 0; i < hole.size(); ++i) {
            segments.emplace_back(hole[i], hole[(i + 1) % hole.size()]);
        }
        using Arrangement_2 = CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<K>>;
        Arrangement_2 env;
        CGAL::insert_non_intersecting_curves(env, segments.begin(), segments.end());
        int edges = 0;
        assert(env.number_of_faces() == 2);
        auto faceHandle = env.faces_begin();
        while (faceHandle == env.unbounded_face()) {
            ++faceHandle;
        }
        using NSPV = CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false>;
        NSPV non_regular_visibility(env);
        Arrangement_2 non_regular_output;
        for (const auto& p : pointsInside) {
            non_regular_visibility.compute_visibility(p, faceHandle, non_regular_output);
            CGAL::Arr_naive_point_location<Arrangement_2> pl(non_regular_output);
            for (const auto& q : pointsInside) {
                auto res = pl.locate(q);
        //         Poly line;
        //         line.push_back(p);
        //         line.push_back(q);
        //         if (!CGAL::do_intersect(hole, line)) {
        //             edges++;
        //         }
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
