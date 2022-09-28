#pragma once
#include <IImGuiLayout.hpp>
#include <AppTypes.hpp>
#include "RemoteLiveSlamClient.hpp"

namespace eureka
{
    struct TopViewModelLayer
    {
        std::string input_ip_text;
        std::size_t current_ip_combo_idx = 0;
    };

    struct MapViewModelLayer
    {
        std::shared_ptr<PoseGraphVisualizationUpdateMsg> last_pose_graph_msg;
    };

    struct ModelLayer
    {
        TopViewModelLayer top_view;
        MapViewModelLayer map_view;
    };

    class RemoteLiveSlamUI : public IImGuiLayout
    {
    private:
        bool                                           _first{ true };
        ImguiLayoutProps                               _props;
        LiveSlamUIMemo                                 _memo; // memo is persistent data between runs
        ModelLayer                                     _model;

        std::shared_ptr<RemoteLiveSlamClient>               _remoteHandler;
  
        sigslot::scoped_connection                          _newPoseGraphConnection;

        void TopView();
        void TopViewConnect();

        void MapView();
        void SideMenuView();
        void SetupDefaultDocking(uint32_t mainDockSpaceId);
        void PlotMapContent();
        void PlotOptimizedCausalPoses();
        void PlotPoseConstraints();

   
        future_t<void> DoConnect();
    public:
        RemoteLiveSlamUI(LiveSlamUIMemo memo, std::shared_ptr<RemoteLiveSlamClient> handler);
        ~RemoteLiveSlamUI();
        void OnActivated(const ImguiLayoutProps& props) override;
        void OnDeactivated() override;
        void UpdateLayout() override;
        void UpdateMemo(LiveSlamUIMemo& liveslam) const;
    };

}

