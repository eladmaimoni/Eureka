#include "RemoteLiveSlamUI.hpp"
#include <implot.h>
#include <implot_internal.h>
#include <imgui_internal.h>
#include <debugger_trace.hpp>
#include <basic_utils.hpp>
#include <asio/ip/address.hpp>
#include <ranges>

namespace ImGui
{
    struct ScopedItemWidth
    {
        ScopedItemWidth(float width)
        {
            PushItemWidth(width);
        }

        ~ScopedItemWidth()
        {
            PopItemWidth();
        }
    };
    

}

namespace ImPlot
{
    struct ScopedStyleColor
    {
        ScopedStyleColor(ImPlotCol_ idx, ImU32 col)
        {
            ImPlot::PushStyleColor(idx, col);
        }

        ~ScopedStyleColor()
        {
            ImPlot::PopStyleColor();
        }
    };

    //struct ScopedItem
    //{
    //    ScopedItem()
    //    {
    //        
    //    }
    //};

    struct Triangle
    {
        ImVec2 tip;
        ImVec2 left;
        ImVec2 right;
    };

    Triangle GenerateOrientationTriangle(const Eigen::Vector3f& dir3, const Eigen::Vector2f& tip, float scale = 1.0f)
    {
        Eigen::Vector2f triangle_opposite_dir(dir3.x(), dir3.z());
        triangle_opposite_dir.normalize();

        //Eigen::Vector2f prev = tip + triangle_opposite_dir;

        Eigen::Vector2f perp(triangle_opposite_dir.y(), -triangle_opposite_dir.x());
        Eigen::Vector2f left = tip - scale * (triangle_opposite_dir * 4.0f + perp);
        Eigen::Vector2f right = tip - scale * (triangle_opposite_dir * 4.0f - perp);

        Triangle triangle;
        triangle.tip = ImPlot::PlotToPixels(ImPlotPoint(tip.x(), tip.y()));
        triangle.left = ImPlot::PlotToPixels(ImPlotPoint(left.x(), left.y()));
        triangle.right = ImPlot::PlotToPixels(ImPlotPoint(right.x(), right.y()));
        return triangle;
    }

}

#define IMGUI_SCOPED(stmt) auto EUREKA_CONCAT(__imgui_scoped_var__, 123) = stmt

namespace eureka
{
    // https://www.rapidtables.com/web/color/RGB_Color.html
    // format 0x'A'B'G'R / IM_COL32(R, G, B, A);
    constexpr ImU32 IMGUI_COLOR_BLACK = IM_COL32(0, 0, 0, 255);
    constexpr ImU32 IMGUI_COLOR_WHITE = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 IMGUI_COLOR_GRAY = IM_COL32(128, 128, 128, 255);
    constexpr ImU32 IMGUI_COLOR_RED = IM_COL32(255, 0, 0, 255);
    constexpr ImU32 IMGUI_COLOR_GREEN = IM_COL32(0, 255, 0, 255);
    constexpr ImU32 IMGUI_COLOR_BLUE = IM_COL32(0, 0, 255, 255);
    constexpr ImU32 IMGUI_COLOR_CYAN = IM_COL32(0, 255, 255, 255);
    constexpr ImU32 IMGUI_COLOR_MAGENTA = IM_COL32(255, 0, 255, 255);
    constexpr ImU32 IMGUI_COLOR_YELLOW = IM_COL32(255, 255, 0, 255);

    constexpr ImU32 IMGUI_COLOR_ORANGE = IM_COL32(255, 165, 0, 255);
    constexpr ImU32 IMGUI_COLOR_PURPLE = IM_COL32(128, 0, 128, 128);
    constexpr ImU32 IMGUI_COLOR_SADDLE_BROWN = IM_COL32(139, 69, 19, 255);
    constexpr ImU32 IMGUI_COLOR_SANDY_BROWN = IM_COL32(244, 143, 96, 255);

    const ImColor IMPGUI_COLOR4_YELLOW = IMGUI_COLOR_YELLOW;

    constexpr ImU32 COLOR_OPTIMIZED_COLOR = IMGUI_COLOR_BLUE;
    constexpr ImU32 COLOR_CAUSAL_COLOR = IMGUI_COLOR_WHITE;
    constexpr ImU32 COLOR_PNP_OUTLIER_COLOR = IMGUI_COLOR_ORANGE;
    constexpr ImU32 COLOR_PNP_INLIER_COLOR = IMGUI_COLOR_GREEN;
    constexpr ImU32 COLOR_FILTER_COLOR = IMGUI_COLOR_CYAN;
    constexpr ImU32 COLOR_RESIDUAL_COLOR = IMGUI_COLOR_RED;


    inline constexpr double MAP_AXIS_LIMIT = 1000.0;



    bool PNPInlierPredicate(const uint32_t* edgeMeta)
    {
        static constexpr int META_OFFSET_TYPE = 2;
        static constexpr int META_OFFSET_IS_INLIER = 3;
        return (edgeMeta[META_OFFSET_TYPE] == 2 && edgeMeta[META_OFFSET_IS_INLIER] != 0);
    }

    bool PNPOutlierPredicate(const uint32_t* edgeMeta)
    {
        static constexpr int META_OFFSET_TYPE = 2;
        static constexpr int META_OFFSET_IS_INLIER = 3;
        return (edgeMeta[META_OFFSET_TYPE] == 2 && edgeMeta[META_OFFSET_IS_INLIER] == 0);
    }

    bool FilterPredicate(const uint32_t* edgeMeta)
    {
        static constexpr int META_OFFSET_TYPE = 2;
        static constexpr int META_OFFSET_IS_INLIER = 3;
        return (edgeMeta[META_OFFSET_TYPE] == 0);
    }

    void PlotEdge(ImDrawList* drawList, const float* edgeData, ImU32 color, float thickness = 1.0f)
    {
        Eigen::Vector2f refXZ(edgeData[0], edgeData[2]);
        Eigen::Vector2f inducedTgtXZ(edgeData[3], edgeData[5]);
        Eigen::Vector3f inducedTgtDir(edgeData[6], edgeData[7], edgeData[8]);
        Eigen::Vector2f optimizedTgtXZ(edgeData[9], edgeData[11]);
        auto triangle = ImPlot::GenerateOrientationTriangle(inducedTgtDir, inducedTgtXZ, 1.0f);

        ImVec2 p1 = ImPlot::PlotToPixels(ImPlotPoint(refXZ.x(), refXZ.y()));
        ImVec2 p2 = ImPlot::PlotToPixels(ImPlotPoint(inducedTgtXZ.x(), inducedTgtXZ.y()));
        ImVec2 p3 = ImPlot::PlotToPixels(ImPlotPoint(optimizedTgtXZ.x(), optimizedTgtXZ.y()));

        drawList->AddLine(p1, p2, color, thickness);
        drawList->AddTriangleFilled(triangle.tip, triangle.left, triangle.right, color);
        drawList->AddLine(p2, p3, COLOR_RESIDUAL_COLOR, thickness); // residual
    }


    template<typename Predicate>
    void PlotEdges(
        ImDrawList* drawList,
        const float* edgesData,
        const uint32_t* edgesMeta,
        int EN,
        const char* label,
        ImU32 color,
        Predicate predicate
    )
    {
        static constexpr int META_STRIDE = 4;
        static constexpr int DATA_STRIDE = 12;
        static constexpr int META_OFFSET_REF_ID = 0;
        static constexpr int META_OFFSET_TGT_ID = 1;

        IMGUI_SCOPED(ImPlot::ScopedStyleColor(ImPlotCol_Line, color));

        if (ImPlot::BeginItem(label, ImPlotCol_Line))
        {
            float thickness = ImPlot::IsLegendEntryHovered(label) ? 2.0f : 1.0f;
            // TODO CPP23 ranges approach
            // https://stackoverflow.com/questions/71143460/how-to-set-filter-from-another-view
            //std::ranges::views::zip(edgesData, edgesMeta);

            for (auto i = 0; i < EN; ++i)
            {
                auto edgeMeta = edgesMeta + i * META_STRIDE;
                auto edgeData = edgesData + i * DATA_STRIDE;

                if (predicate(edgeMeta))
                {
                    PlotEdge(drawList, edgeData, color, thickness);
                }
            }

            ImPlot::EndItem();
        }
    }

    RemoteLiveSlamUI::RemoteLiveSlamUI(LiveSlamUIMemo memo, std::shared_ptr<RemoteLiveSlamClient> handler) :
        _memo(memo),
        _remoteHandler(std::move(handler))
    {
        _model.top_view.input_ip_text.resize(20);
        _model.top_view.input_ip_text[0] = '\0';

        if (std::ranges::find(_memo.previously_used_ips, "localhost") == _memo.previously_used_ips.end())
        {
            _memo.previously_used_ips.push_back("localhost");
        }

        std::ranges::sort(_memo.previously_used_ips);
    }

    RemoteLiveSlamUI::~RemoteLiveSlamUI()
    {

    }

    void RemoteLiveSlamUI::OnActivated(const ImguiLayoutProps& props)
    {
        _props = props;

        




        ImPlot::CreateContext();

        // assumed on UI thread, TODO

        _newPoseGraphConnection = _remoteHandler->ConnectPoseGraphSlot([this]
        (std::shared_ptr<PoseGraphVisualizationUpdateMsg> msg)
            {
                _model.map_view.last_pose_graph_msg = std::move(msg);
            }
        );

        DoConnect();
       

    }

    void RemoteLiveSlamUI::OnDeactivated()
    {
        _newPoseGraphConnection.disconnect();
        
        _remoteHandler->Disconnect();

        ImPlot::DestroyContext();
    }

    static constexpr char MAIN_MENU_WINDOW_NAME[] = "Main Menu";
    static constexpr char TOP_MENU_WINDOW_NAME[] = "##Top Menu";
    static constexpr char MAP_WINDOW_NAME[] = "Map";
    static constexpr float MAIN_DOCKING_DEFAULT_SPLIT_RATIO = 0.2f;
    static constexpr float BOTTOM_DOCKING_DEFAULT_SPLIT_RATIO = 0.1f;


    void MainMenuSize(ImGuiSizeCallbackData* data)
    {
        auto csize = data->CurrentSize;
        DEBUGGER_TRACE("MainMenuSize current size {} {} ", csize.x, csize.y);
    }

    void RemoteLiveSlamUI::UpdateLayout()
    {
        auto vp = ImGui::GetMainViewport();
        auto mainDockSpaceId = ImGui::DockSpaceOverViewport(vp, ImGuiDockNodeFlags_PassthruCentralNode);
        if (_first)
        {
            if (!_props.has_ini_file)
            {
                SetupDefaultDocking(mainDockSpaceId);
            }
        }
     
        SideMenuView();
        MapView();
        TopView();
        ImGui::ShowDemoWindow();
        ImPlot::ShowDemoWindow();
        _first = false;
    }

    void RemoteLiveSlamUI::UpdateMemo(LiveSlamUIMemo& liveslam) const
    {
        liveslam = _memo;
    }

    void RemoteLiveSlamUI::TopView()
    {
     
        ImGui::Begin(TOP_MENU_WINDOW_NAME, nullptr);
       
        TopViewConnect();

        ImGui::End();
    }



    void RemoteLiveSlamUI::TopViewConnect()
    {
        ImGui::Text("LiveSlam IP");
        ImGui::SameLine();

        IMGUI_SCOPED(ImGui::ScopedItemWidth(160.0f));

        if (ImGui::BeginCombo("##IP_COMBO", _memo.previously_used_ips[_model.top_view.current_ip_combo_idx].c_str(), ImGuiComboFlags_None))
        {
            for (auto i = 0; i < _memo.previously_used_ips.size(); ++i)
            {
                const bool isSelected = (_model.top_view.current_ip_combo_idx == i);
                if (ImGui::Selectable(_memo.previously_used_ips[i].c_str(), isSelected))
                {
                    _model.top_view.current_ip_combo_idx = i;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }

            
            }
            auto frameHeight = ImGui::GetFrameHeight();
            auto width = ImGui::CalcItemWidth();
            ImGui::SetNextItemWidth(width - frameHeight);

            ImGui::InputTextWithHint("##IP_ADD2", "enter ip address", _model.top_view.input_ip_text.data(), _model.top_view.input_ip_text.size());
        
            ImGui::SameLine();

            if (ImGui::Button("+", ImVec2(frameHeight, frameHeight)))
            {
                asio::error_code ec;
                asio::ip::address::from_string(_model.top_view.input_ip_text, ec);
                if (!ec)
                {
                    _memo.previously_used_ips.emplace_back(_model.top_view.input_ip_text);
                }
                else
                {
                    DEBUGGER_TRACE("INVALID IP = {}", _model.top_view.input_ip_text);
                }
      
            }
            ImGui::EndCombo();
        }
   
        ImGui::SameLine();

 
        auto connectionState = _remoteHandler->GetConnectionState();

        switch (connectionState)
        {
        case ConnectionState::Disconnected:
        {
            if (ImGui::Button("Connect"))
            {
                DoConnect();
                DEBUGGER_TRACE("Disconnect");
            }
            break;
        }
        case ConnectionState::Connected:
        {
            if (ImGui::Button("Disconnect"))
            {
                _remoteHandler->Disconnect();
                DEBUGGER_TRACE("Disconnect");
            }
            ImGui::SameLine();
            ImGui::Text("Connected");
            break;
        }
        case ConnectionState::Connecting:
        {
            if (ImGui::Button("Cancel"))
            {
                _remoteHandler->CancelConnecting();
            }
            ImGui::SameLine();
            ImGui::Text("Waiting For Connection ...");
            break;
        }
        }

    }

    void RemoteLiveSlamUI::MapView()
    {
        ImVec2 mapPlotSize(-1, -1);
        ImPlotAxisFlags axisFlags = ImPlotAxisFlags_None;

        //if (_fitMap)
        //{
        //    axisFlags |= ImPlotAxisFlags_AutoFit;
        //    _fitMap = false;
        //}

        if (ImGui::Begin(MAP_WINDOW_NAME, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration))
        {
            if (ImPlot::BeginPlot("Map", mapPlotSize, ImPlotFlags_Equal))
            {
                ImPlot::SetupAxesLimits(-MAP_AXIS_LIMIT, MAP_AXIS_LIMIT, -MAP_AXIS_LIMIT, MAP_AXIS_LIMIT);
                ImPlot::SetupAxes("x(cm)", "z(cm)", axisFlags, axisFlags);

                PlotMapContent();

                ImPlot::EndPlot();
            }  
        }
        ImGui::End();
       
    }

    void RemoteLiveSlamUI::SideMenuView()
    {
        ImGui::Begin(MAIN_MENU_WINDOW_NAME, nullptr);


        if (ImGui::CollapsingHeader("Pose Graph"))
        {
            ImGui::Checkbox("gpo optimized", &_memo.show_gpo_optimized);
            ImGui::Checkbox("pnp inliers", &_memo.show_pnp_inliers); 
            ImGui::Checkbox("pnp outliers", &_memo.show_pnp_outliers);
            ImGui::Checkbox("filter constraints", &_memo.show_filter_constraints);
        }

        //if (ImGui::Button("Start Read Poses"))
        //{
        //    _remoteHandler->StartReadingPoseGraphUpdates();
        //}
        //if (ImGui::Button("Stop Read Poses"))
        //{
        //    _remoteHandler->StopReadingPoseGraphUpdates();
        //}
        ImGui::End();
    }

    void RemoteLiveSlamUI::SetupDefaultDocking(uint32_t mainDockSpaceId)
    {
        DEBUGGER_TRACE("NO .ini file, setting default layout");

        ImGuiID mainMenuDockId = 0;
        ImGuiID mapDockingNodeId = 0;
        ImGuiID upperMenuDockingNodeId = 0;
        ImGui::DockBuilderSplitNode(mainDockSpaceId, ImGuiDir_Up, BOTTOM_DOCKING_DEFAULT_SPLIT_RATIO, &upperMenuDockingNodeId, &mapDockingNodeId);
        DEBUGGER_TRACE("splitted mainDockSpaceId, mapDockingNodeId = {:#x}, upperMenuDockingNodeId = {:#x}",
            mapDockingNodeId,
            upperMenuDockingNodeId
        );
        ImGui::DockBuilderSplitNode(mapDockingNodeId, ImGuiDir_Left, MAIN_DOCKING_DEFAULT_SPLIT_RATIO, &mainMenuDockId, &mapDockingNodeId);
        DEBUGGER_TRACE("splitted mapDockingNodeId, mainMenuDockId = {:#x}, mapDockingNodeId = {:#x}",
            mainDockSpaceId,
            mapDockingNodeId
        );
        // hide the tab bar for the map view and  menu
        ImGui::DockBuilderGetNode(mapDockingNodeId)->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
        ImGui::DockBuilderGetNode(mainMenuDockId)->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;

        ImGui::DockBuilderDockWindow(MAIN_MENU_WINDOW_NAME, mainMenuDockId);
        ImGui::DockBuilderDockWindow(MAP_WINDOW_NAME, mapDockingNodeId);
        ImGui::DockBuilderDockWindow(TOP_MENU_WINDOW_NAME, upperMenuDockingNodeId);
    }

    void RemoteLiveSlamUI::PlotMapContent()
    {
        if (_model.map_view.last_pose_graph_msg)
        {
            if (_memo.show_gpo_optimized)
            {
                PlotOptimizedCausalPoses();
            }
      

            PlotPoseConstraints();
       
        }
      
    }

    void RemoteLiveSlamUI::PlotOptimizedCausalPoses()
    {
        const auto& poses = _model.map_view.last_pose_graph_msg->poses();

        static constexpr int POSES_STRIDE = 7;
        static constexpr int OFFSET_ID = 0;
        static constexpr int OFFSET_TX = 1;
        static constexpr int OFFSET_TY = 2;
        static constexpr int OFFSET_TZ = 3;
        static constexpr int OFFSET_RX = 4;
        static constexpr int OFFSET_RY = 5;
        static constexpr int OFFSET_RZ = 6;

        auto N = poses.size() / POSES_STRIDE;

        IMGUI_SCOPED(ImPlot::ScopedStyleColor(ImPlotCol_Line, IMGUI_COLOR_BLUE));

        ImPlot::PlotLine("optimized", poses.data() + OFFSET_TX, poses.data() + OFFSET_TZ, N, 0, POSES_STRIDE * sizeof(float));

        auto drawlist = ImPlot::GetPlotDrawList();
        for (auto i = OFFSET_TX; i < poses.size(); i += POSES_STRIDE)
        {
            auto tx = poses[i];
            auto tz = poses[i + 2];
            Eigen::Vector2f tip(tx, tz);
            Eigen::Vector3f dir(poses[i + 3], poses[i + 4], poses[i + 5]);

            auto triangle = ImPlot::GenerateOrientationTriangle(dir, tip);

            drawlist->AddTriangleFilled(triangle.tip, triangle.left, triangle.right, IMGUI_COLOR_BLUE);
        }

        //if (_model->annotate_current_position && N > 1)

        if (N > 1)
        {
            auto last_id = reinterpret_cast<const uint32_t&>(poses[(N - 1) * POSES_STRIDE + OFFSET_ID]);
            auto last_tx = poses[(N - 1) * POSES_STRIDE + OFFSET_TX];
            auto last_tz = poses[(N - 1) * POSES_STRIDE + OFFSET_TZ];

            auto pre_tx = poses[(N - 2) * POSES_STRIDE + OFFSET_TX];
            auto pre_tz = poses[(N - 2) * POSES_STRIDE + OFFSET_TZ];

            Eigen::Vector2f dir = (Eigen::Vector2f(last_tx, last_tz) - Eigen::Vector2f(pre_tx, pre_tz)).normalized();

            dir *= -15.0f;

            ImPlot::Annotation(
                static_cast<double>(last_tx), static_cast<double>(last_tz),
                IMPGUI_COLOR4_YELLOW,
                ImVec2(dir.x(), dir.y()),
                false,
                "%lu = (%.2f %.2f)",
                last_id,
                last_tx,
                last_tz
            );
        }
    }

    void RemoteLiveSlamUI::PlotPoseConstraints()
    {
        static constexpr int META_STRIDE = 4;
        static constexpr int DATA_STRIDE = 12;
        static constexpr int META_OFFSET_REF_ID = 0;
        static constexpr int META_OFFSET_TGT_ID = 1;
        static constexpr int META_OFFSET_TYPE = 2;
        static constexpr int META_OFFSET_IS_INLIER = 3;

        static constexpr char PNP_INLIER_LABEL[] = "pnp induced ref->target (inlier)";
        static constexpr char PNP_OUTLIER_LABEL[] = "pnp induced ref->target (outlier)";
        static constexpr char FILTER_RELATIVE_LABEL[] = "filter induced ref->target";


        const auto& edgesMeta = _model.map_view.last_pose_graph_msg->edges_meta();
        const auto& edgesData = _model.map_view.last_pose_graph_msg->edges_data();
        const auto EN = edgesMeta.size() / META_STRIDE;

        auto drawList = ImPlot::GetPlotDrawList();

        if (_memo.show_pnp_inliers)
        {
            PlotEdges(drawList, edgesData.data(), edgesMeta.data(), EN, PNP_INLIER_LABEL, COLOR_PNP_INLIER_COLOR, PNPInlierPredicate);
        }
        if (_memo.show_pnp_outliers)
        {
            PlotEdges(drawList, edgesData.data(), edgesMeta.data(), EN, PNP_OUTLIER_LABEL, COLOR_PNP_OUTLIER_COLOR, PNPOutlierPredicate);
        }
        if (_memo.show_filter_constraints)
        {
            PlotEdges(drawList, edgesData.data(), edgesMeta.data(), EN, FILTER_RELATIVE_LABEL, COLOR_FILTER_COLOR, FilterPredicate);
        }


    }

    future_t<void> RemoteLiveSlamUI::DoConnect()
    {
        try
        {
            co_await _remoteHandler->ConnectAsync(_memo.previously_used_ips[_model.top_view.current_ip_combo_idx]);

            _remoteHandler->StartReadingPoseGraphUpdates();
        }
        catch (const std::exception& err)
        {
            DEBUGGER_TRACE("DoConnect {}", err.what());
        }

        co_return;

    }

}



