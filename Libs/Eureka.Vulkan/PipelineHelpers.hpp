#pragma once
#include "ShadersCache.hpp"
#include "PipelineTypes.hpp"

#include <boost/hana/for_each.hpp>
#include <containers_aliases.hpp>
#include <boost/hana/adapt_struct.hpp>
//#include <boost/hana/members.hpp>
//#include <boost/hana/size.hpp>

BOOST_HANA_ADAPT_STRUCT(eureka::vulkan::ImGuiVertex, position, uv, color);

namespace eureka::vulkan
{


    //
    // CountFields - count the number of fields in a struct
    // works only on boost::hana adapted struct
    // 
    template <typename T>
    constexpr std::size_t CountFields()
    {
        namespace hana = boost::hana;
        std::size_t count = 0;

        hana::for_each(hana::accessors<T>(), [&](auto) { ++count; });

        return count;
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //                        Vertex Layout
    //
    //////////////////////////////////////////////////////////////////////////



    template <typename T>
    constexpr VkFormat GetAttributeDefaultFormat()
    {
        if constexpr(std::is_same_v<T, uint32_t>)
        {
            return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        }
        else if constexpr(std::is_same_v<T, Eigen::Vector2f>)
        {
            return VkFormat::VK_FORMAT_R32G32_SFLOAT;
        }
        else if constexpr(std::is_same_v<T, Eigen::Vector3f>)
        {
            return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
        }
        else if constexpr(std::is_same_v<T, Eigen::Vector4f>)
        {
            return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        else
        {
            [] <bool flag = false>()
            {
                static_assert(flag, "no match");
            }();
        }
    }

    //
    // Creates VertexLayout struct with binding point per attribute
    //
    template <typename... Attribute>
    auto MakeDeinterleavedVertexLayout()
    {
        constexpr std::size_t AttributeCount = sizeof...(Attribute);

        VertexLayout layout {};
        layout.bindings.resize(AttributeCount);
        layout.attributes.resize(AttributeCount);
        uint32_t i = 0;

        // while (i < AttributeCount)
        (
            [&] {
                layout.bindings[i] = VkVertexInputBindingDescription {
                    .binding = i,
                    .stride = sizeof(Attribute),
                    .inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX,
                };

                layout.attributes[i] = VkVertexInputBindingDescription {
                    .location = i,
                    .binding = i,
                    .format = GetAttributeDefaultFormat<Attribute>(),
                    .offset = 0,
                };
                ++i;
            }(),
            ...);

        return layout;
    }



    //
    // Creates VertexLayout struct with one binding point for all attributes
    //
    template <typename VertexStruct>
    auto MakeInterleavedVertexLayout()
    {
        VertexStruct value {}; // dummy, TODO maybe we can somehow remove this
        namespace hana = boost::hana;
        constexpr std::size_t AttributeCount = CountFields<VertexStruct>();

        VertexLayout layout {};
        layout.bindings.resize(1);
        layout.attributes.resize(AttributeCount);
        layout.bindings[0] = VkVertexInputBindingDescription {
            .binding = 0,
            .stride = sizeof(VertexStruct),
            .inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX,
        };

        uint32_t i = 0;

        hana::for_each(hana::accessors<VertexStruct>(), [&](auto pair) {
            auto member_access = hana::second(pair);
            using value_t = std::decay_t<decltype(member_access(value))>;
            auto offset = reinterpret_cast<uint64_t>(&member_access(value)) - reinterpret_cast<uint64_t>(&value);
            layout.attributes[i] = VkVertexInputAttributeDescription {
                .location = i,
                .binding = 0,
                .format = GetAttributeDefaultFormat<value_t>(),
                .offset = static_cast<uint32_t>(offset),
            };
            ++i;
        });
        return layout;
    }




    template <std::size_t COUNT>
    auto MakeShaderPipeline(std::array<ShaderId, COUNT> ids, ShaderCache& shaderCache)
    {
        ShadersPipeline shaderPipeline;
        shaderPipeline.modules.resize(COUNT);
        shaderPipeline.stages.resize(COUNT);

        for(auto i = 0u; i < ids.size(); ++i)
        {
            shaderPipeline.modules[i] = shaderCache.LoadShaderModule(ids[i]);
            shaderPipeline.stages[i] = VkPipelineShaderStageCreateInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = ids[i].shader_type,
                .module = shaderPipeline.modules[i].Get(),
                .pName = "main",
            };
        }
        return shaderPipeline;
    }
} // namespace eureka::vulkan
