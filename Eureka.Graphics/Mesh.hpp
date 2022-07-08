#pragma once

#include "PipelineTypes.hpp"
#include "Buffer.hpp"
#include "Pipeline.hpp"
#include "Image.hpp"

namespace eureka::mesh
{

    static const std::array<PositionColorVertex, 3> COLORED_TRIANGLE_VERTEX_DATA
    {
        PositionColorVertex{ Eigen::Vector3f{  1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 1.0f, 0.6f, 0.4f } },
        PositionColorVertex{ Eigen::Vector3f{ -1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 0.7f, 1.0f, 0.7f } },
        PositionColorVertex{ Eigen::Vector3f{  0.0f, -1.0f, 0.0f }, Eigen::Vector3f{ 0.96f, 0.72f, 0.96f } }
    };

    static constexpr std::array<uint32_t, 3> COLORED_TRIANGLE_INDEX_DATA = { 0, 1, 2 };

}

namespace eureka
{
    struct CNTexturedPrimitiveBufferOffsets
    {
        uint64_t index_offset;
        uint64_t position_offset;
        uint64_t normal_offset;
        uint64_t uv_offset;
        uint64_t tangent_offset;
    };

    struct CNTexturedPrimitiveNode
    {
        CNTexturedPrimitiveBufferOffsets buffer_offsets;
        
        OwnedDescriptorSet               fragment_desc_set;
    };

    // CNR - Color, Normal Map, Metallic Roughness. Textured model with color map, normal map and metallic roughness map
    class CNTexturedModelGroup
    {
        VertexAndIndexTransferableDeviceBuffer buffer;
        std::vector<SampledImage2D> textures;
    };

    class PhongNormalShadedDrawables
    {


        std::shared_ptr<PhongShadedMeshWithNormalMapPipeline> _pipeline;
    };
}
