#include "DescriptorLayoutCache.hpp"
#include <stdexcept>

namespace eureka::vulkan
{

    DescriptorSetLayoutPreset::DescriptorSetLayoutPreset(DescriptorSet0PresetType setPresetId)
    {
        switch(setPresetId)
        {
        case DescriptorSet0PresetType::ePerViewUniform:

            _bindings.emplace_back(VkDescriptorSetLayoutBinding {
                .binding = 0, // shader side index (why not named location??)
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1, // a single constant buffer
                .stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
            });
            break;
        case DescriptorSet0PresetType::ePerFont:
            _bindings.emplace_back(VkDescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
            });
            break;
        default:
            throw std::invalid_argument("bad");
        }

        _createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        _createInfo.bindingCount = static_cast<uint32_t>(_bindings.size());
        _createInfo.pBindings = _bindings.data();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    VkDescriptorSetLayout DescriptorSetLayoutCache::GetLayoutHandle(DescriptorSet0PresetType preset) const
    {
        return _setsLayout.at(0).at(static_cast<int>(preset))->Get();
    }

    DescriptorSetLayoutCache::DescriptorSetLayoutCache(std::shared_ptr<Device> device) :
        _device(std::move(device))
    {
        // TODO iterate over enums

        for(auto preset : {DescriptorSet0PresetType::ePerViewUniform, DescriptorSet0PresetType::ePerFont})
        {
            DescriptorSetLayoutPreset presetVal(preset);

            _setsLayout.at(0).emplace_back(std::make_shared<DescriptorSetLayout>(_device, presetVal.GetCreateInfo()));
        }
    }

} // namespace eureka::vulkan
