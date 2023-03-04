#pragma once
#include <IImGuiLayout.hpp>
#include <AppTypes.hpp>
#include "RemoteLiveSlamClient.hpp"

namespace eureka::ui
{
    struct TopViewModelLayer
    {
        std::string input_ip_text;
        std::size_t current_ip_combo_idx = 0;
    };

    struct MapViewModelLayer
    {
        MapViewModelLayer()
        {
            realtime_poses_t.reserve(1024 * 512);
            realtime_poses_r.reserve(1024 * 512);
            upper_right_text.reserve(1024);
        }

        std::shared_ptr<rgoproto::PoseGraphStreamingMsg> last_pose_graph_msg;

        std::vector<float> realtime_poses_t;
        std::vector<float> realtime_poses_r;
        Eigen::Vector3f    realtime_last_txtytz = Eigen::Vector3f::Zero();
        Eigen::Vector3f    realtime_last_rxryrz = Eigen::Vector3f::Zero();

        std::string        upper_right_text;

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

        std::shared_ptr<rpc::RemoteLiveSlamClient>          _remoteHandler;
  
        sigslot::scoped_connection                          _newPoseGraphConnection;
        sigslot::scoped_connection                          _newRealtimePoseConnection;
        sigslot::scoped_connection                          _channelStateConnection;

        void TopView();
        void TopViewConnect();

        void MapView();
        void SideMenuView();
        void SetupDefaultDocking(uint32_t mainDockSpaceId);
        void PlotMapContent();

        void PlotMapText();

        void PlotOptimizedCausalPoses();
        void PlotPoseConstraints();
        void PlotRealtimePose();

   
        void InitiateConnection();
        void HandleConnectionStateChange(rpc::ConnectionState state);
    public:
        RemoteLiveSlamUI(LiveSlamUIMemo memo, std::shared_ptr<rpc::RemoteLiveSlamClient> handler);
        ~RemoteLiveSlamUI();
        void OnActivated(const ImguiLayoutProps& props) override;
        void OnDeactivated() override;
        void UpdateLayout() override;
        void UpdateMemo(LiveSlamUIMemo& liveslam) const;
    };

}

