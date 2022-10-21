#pragma once
#include "../Eureka.Windowing/WindowTypes.hpp"

namespace eureka
{
    enum class StamEnum
    {
        eOne, eTwo
    };

    struct LiveSlamUIMemo
    {
        std::vector<std::string> previously_used_ips;
        bool show_filter_constraints = true;
        bool show_pnp_inliers = true;
        bool show_pnp_outliers = true;
        bool show_gpo_optimized = true;
        bool show_realtime = true;
        bool show_height = true;
    };

    struct AppMemo
    {
        WindowConfig window_config;
        LiveSlamUIMemo liveslam;
 
    };

}

