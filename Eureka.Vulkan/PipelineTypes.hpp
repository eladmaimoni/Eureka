#pragma once
#include <Eigen/Core>
#include <cstdint>
#include <containers_aliases.hpp>

namespace eureka::vulkan
{
    struct ImGuiVertex
    {
        Eigen::Vector2f position;
        Eigen::Vector2f uv;
        uint32_t        color;
    };

    struct ScaleTranslatePushConstantsBlock
    {
        Eigen::Vector2f scale;
        Eigen::Vector2f translate;
    };


    //struct vector3
    //{
    //    float x;
    //    float y;
    //    float z;
    //};

    //struct ViewProjection
    //{
    //    Eigen::Matrix4f view;
    //    Eigen::Matrix4f projection;
    //};

    //struct PositionColorVertex
    //{
    //    Eigen::Vector3f position;
    //    Eigen::Vector3f color;
    //};





    struct VertexLayout
    {
        svec5<VkVertexInputBindingDescription>   bindings;
        svec5<VkVertexInputAttributeDescription> attributes;
    };


} // namespace eureka::vulkan