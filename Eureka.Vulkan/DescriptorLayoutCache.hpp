#pragma once
#include "DescriptorSetsLayout.hpp"
#include "Device.hpp"
#include <array>
#include <mutex>
#include <unordered_map>

namespace eureka::vulkan
{
    // this is an identifier used to retrieve a descriptor set layout
    // usually by pipeline presets

    enum class DescriptorSet0PresetType
    {
        ePerViewUniform, // MVP
        ePerFont // all shaders use the same first texture
    };

    static constexpr uint64_t MAX_SET_SLOTS = 4;

    /*
    General Idea:
    descriptor layout preset define the shader 'signature'
    shader parameters (descritors) appear on shader side in the following format:
    - 'sets' / 'binding' /   point where each set contains a maximum number of 'slots'
    - a predetermined amount of slots for each set
    

    HLSL side: cbuffer ubo : register(b0, space0) { UBO ubo; }; set 0, slot 0
    GLSL side: (binding = 0) ???
    Host side: 

    */

    class DescriptorSetLayoutPreset
    {
        //
        // struct for creating a descriptor set layout according to a preset
        // deterimines the number of slots (descriptors) and types for this set
        //
        VkDescriptorSetLayoutCreateInfo           _createInfo {};
        std::vector<VkDescriptorSetLayoutBinding> _bindings;

    public:
        DescriptorSetLayoutPreset(DescriptorSet0PresetType setPresetId);
        DescriptorSetLayoutPreset(const DescriptorSetLayoutPreset&) = delete;
        DescriptorSetLayoutPreset& operator=(const DescriptorSetLayoutPreset&) = delete;
        DescriptorSetLayoutPreset(DescriptorSetLayoutPreset&&) = default;
        DescriptorSetLayoutPreset&             operator=(DescriptorSetLayoutPreset&&) = default;
        const VkDescriptorSetLayoutCreateInfo& GetCreateInfo() const
        {
            return _createInfo;
        }
    };

    class DescriptorSetLayoutCache
    {
        std::shared_ptr<Device>                                                      _device;
        std::array<std::vector<std::shared_ptr<DescriptorSetLayout>>, MAX_SET_SLOTS> _setsLayout; // [set_num][preset]
        //mutable std::mutex                                                           _mtx;

    public:
        DescriptorSetLayoutCache(std::shared_ptr<Device> device);
        VkDescriptorSetLayout GetLayoutHandle(DescriptorSet0PresetType preset) const;
    };
} // namespace eureka::vulkan
