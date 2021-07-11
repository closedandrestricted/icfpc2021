#include "solver/solver.h"
#include "imgui.h"
#include "implot.h"
#include "implot_internal.h"
#include "gflags/gflags.h"

DECLARE_int32(test_idx);
DECLARE_string(solution_file);

bool Vertex(const char* id, double* x, double* y, bool draggable, bool show_label, bool always_show_label, const ImVec4& col, float radius = 4.0) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "DragPoint() needs to be called between BeginPlot() and EndPlot()!");
    const float grab_size = ImMax(5.0f, 2*radius);
    const bool outside = !ImPlot::GetPlotLimits().Contains(*x,*y);
    if (outside)
        return false;
    const ImVec4 color = ImPlot::IsColorAuto(col) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : col;
    const ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
    ImDrawList& DrawList = *ImPlot::GetPlotDrawList();
    const ImVec2 pos = ImPlot::PlotToPixels(*x,*y);
    int yax = ImPlot::GetCurrentYAxis();
    ImVec2 old_cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 new_cursor_pos = ImVec2(pos - ImVec2(grab_size,grab_size)*0.5f);
    ImGui::GetCurrentWindow()->DC.CursorPos = new_cursor_pos;
    ImGui::InvisibleButton(id, ImVec2(grab_size, grab_size));
    ImGui::GetCurrentWindow()->DC.CursorPos = old_cursor_pos;
    ImPlot::PushPlotClipRect();
    if (ImGui::IsItemHovered() || ImGui::IsItemActive() || always_show_label) {
        DrawList.AddCircleFilled(pos, 1.5f*radius, (col32));
        gp.CurrentPlot->PlotHovered = false;
        if (show_label) {
            ImVec2 label_pos = pos + ImVec2(16 * GImGui->Style.MouseCursorScale, 8 * GImGui->Style.MouseCursorScale);
            char buff1[32];
            char buff2[32];
            ImPlot::LabelAxisValue(gp.CurrentPlot->XAxis, gp.XTicks, *x, buff1, 32);
            ImPlot::LabelAxisValue(gp.CurrentPlot->YAxis[yax], gp.YTicks[yax], *y, buff2, 32);
            gp.Annotations.Append(label_pos, ImVec2(0.0001f,0.00001f), col32, ImPlot::CalcTextColor(color), true, "%s = %s,%s", id, buff1, buff2);
        }
    }
    else {
        DrawList.AddCircleFilled(pos, radius, col32);
    }
    ImPlot::PopPlotClipRect();

    bool dragging = false;
    if (draggable && ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        *x = ImPlot::GetPlotMousePos().x;
        *y = ImPlot::GetPlotMousePos().y;
        *x = ImClamp(*x, gp.CurrentPlot->XAxis.Range.Min, gp.CurrentPlot->XAxis.Range.Max);
        *y = ImClamp(*y, gp.CurrentPlot->YAxis[yax].Range.Min, gp.CurrentPlot->YAxis[yax].Range.Max);
        dragging = true;
    }
    return dragging;
}

void Edge(double x1, double y1, double x2, double y2, const ImVec4& col, float thickness = 1) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "DragLineX() needs to be called between BeginPlot() and EndPlot()!");
    ImVec4 color = ImPlot::IsColorAuto(col) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : col;
    ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
    ImDrawList& DrawList = *ImPlot::GetPlotDrawList();
    const ImVec2 pos1 = ImPlot::PlotToPixels(x1, y1);
    const ImVec2 pos2 = ImPlot::PlotToPixels(x2, y2);
    ImPlot::PushPlotClipRect();
    DrawList.AddLine(pos1, pos2, col32, thickness);
    ImPlot::PopPlotClipRect();
}

struct Vis {
    std::vector<Point> points;
    std::vector<double> tx, ty, holex, holey;
    Problem p;

    Vis() {
        if (FLAGS_solution_file == "") {
            FLAGS_solution_file = "solutions/staging/" + std::to_string(FLAGS_test_idx) + ".json";
        }
        auto fn = "problems/" + std::to_string(FLAGS_test_idx) + ".json";
        p.parseJson(fn);
        // TODO save/load
        points = p.originalPoints;
        {
            try {
                std::ifstream in(FLAGS_solution_file);
                json sol;
                in >> sol;
                std::cerr << sol["vertices"].size() << " " << points.size() << std::endl;
                for (size_t i = 0; i < sol["vertices"].size(); ++i) {
                    points[i].x = sol["vertices"][i][0];
                    points[i].y = sol["vertices"][i][1];
                }
            } catch (...) {

            };
        }
        tx.resize(points.size());
        ty.resize(points.size());
        for (size_t i = 0; i < tx.size(); ++i) {
            tx[i] = points[i].x;
            ty[i] = points[i].y;
        }
        holex.resize(p.hole.size() + 1);
        holey.resize(p.hole.size() + 1);
        for (size_t i = 0; i <= p.hole.size(); ++i) {
            holex[i] = p.hole[i % p.hole.size()].x;
            holey[i] = p.hole[i % p.hole.size()].y;
        }
    }

    void save() {
        std::ofstream of(FLAGS_solution_file);
        json sol;
        for (size_t i = 0; i < points.size(); ++i) {
            sol["vertices"][i][0] = points[i].x;
            sol["vertices"][i][1] = points[i].y;
        }
        sol >> of;
    }

    void loop() {
        save();
        bool always_show = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Space));
        if (ImPlot::BeginPlot("", nullptr, nullptr, ImVec2(-1, -1), ImPlotFlags_Equal | ImPlotFlags_Crosshairs)) {
            for (size_t e = 0; e < p.edgeU.size(); ++e) {
                int u = p.edgeU[e], v = p.edgeV[e];
                auto col = ImVec4(0.0, 1.0, 0.0, 0.5);
                double nom = (tx[u] - tx[v]) * (tx[u] - tx[v]) + (ty[u] - ty[v]) * (ty[u] - ty[v]);
                double denom = dist2(p.originalPoints[u], p.originalPoints[v]);
                double r = nom / denom - 1.0;
                if (r < -p.eps) {
                    col = ImVec4(1.0, 0.0, 0.0, 0.5);
                }
                if (r > p.eps) {
                    col = ImVec4(0.0, 0.0, 1.0, 0.5);
                }
                Edge(tx[u], ty[u], tx[v], ty[v], col);
            }
            for (size_t i = 0; i < points.size(); ++i) {
                ImPlot::FitPoint(ImPlotPoint(tx[i], ty[i]));
                if (!Vertex(("A(" + std::to_string(i) + ")").c_str(), &tx[i], &ty[i], true, true, always_show, ImVec4(1.0, 0.0, 0.0, 0.5), 5.0)) {
                    tx[i] = std::round(tx[i]);
                    ty[i] = std::round(ty[i]);
                    points[i].x = int(tx[i]);
                    points[i].y = int(ty[i]);
                }
            }
            ImPlot::PlotLine("hole", &holex[0], &holey[0], holex.size());
            for (size_t i = 0; i <= p.hole.size(); ++i) {
                double x = p.hole[i].x, y = p.hole[i].y;
                Vertex(("H(" + std::to_string(i) + ")").c_str(), &x, &y, false, true, always_show, ImVec4(0.0, 0.0, 1.0, 0.5), 3.0);
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
    }
};