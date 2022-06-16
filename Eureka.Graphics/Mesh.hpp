#pragma once

#include "PipelineTypes.hpp"
#include "Buffer.hpp"


namespace eureka::mesh
{

    static const std::array<PositionColorVertex, 3> COLORED_TRIANGLE_VERTEX_DATA
    {
        PositionColorVertex{ Eigen::Vector3f{  1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 1.0f, 0.6f, 0.4f } },
        PositionColorVertex{ Eigen::Vector3f{ -1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 0.7f, 1.0f, 0.7f } },
        PositionColorVertex{ Eigen::Vector3f{  0.0f, -1.0f, 0.0f }, Eigen::Vector3f{ 0.96f, 0.72f, 0.96f } }
    };
    //static const std::array<PositionColorVertex, 3> COLORED_TRIANGLE_VERTEX_DATA
    //{
    //    PositionColorVertex{ Eigen::Vector3f{  1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 1.0f, 0.0f, 0.0f } },
    //    PositionColorVertex{ Eigen::Vector3f{ -1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 0.0f, 1.0f, 0.0f } },
    //    PositionColorVertex{ Eigen::Vector3f{  0.0f, -1.0f, 0.0f }, Eigen::Vector3f{ 0.0f, 0.0f, 1.0f } }
    //};

    static constexpr std::array<uint32_t, 3> COLORED_TRIANGLE_INDEX_DATA = { 0, 1, 2 };

}
