#include "DescriptorSetsLayout.hpp"

namespace eureka
{
    DescriptorSetLayout::DescriptorSetLayout(const vkr::Device& device, const vk::DescriptorSetLayoutCreateInfo& info)
        : _layout(device.createDescriptorSetLayout(info))
    {
    }

    SingleVertexShaderUBODescriptorSetLayout::SingleVertexShaderUBODescriptorSetLayout(DeviceContext& deviceContext)
    {
        // describe the relation between the shader indices (set 0, binding 0)
        // to the host indices 
        // - host indices are the sets
        // - device indices are the set index and binding number within the set that can potentially be unordered
        // set 0, only has a single binding inside shader
        vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding
        {
            .binding = 0, // shader side index (why not named location??)
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1, // a single constant buffer
            .stageFlags = vk::ShaderStageFlagBits::eVertex
        };

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
        {
            .bindingCount = 1,
            .pBindings = &descriptorSetLayoutBinding
        };

        _layout = deviceContext.LogicalDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    }

    SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout::SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout(DeviceContext& deviceContext)
    {
        vk::DescriptorSetLayoutBinding binding
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment
        };

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
        {
            .bindingCount = 1,
            .pBindings = &binding
        };

        _layout = deviceContext.LogicalDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    }

    ColorAndNormalMapFragmentDescriptorSetLayout::ColorAndNormalMapFragmentDescriptorSetLayout(DeviceContext& deviceContext)
    {
        std::array<vk::DescriptorSetLayoutBinding, 2> descriptorSetLayoutBindings
        {
            vk::DescriptorSetLayoutBinding
            {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment
            },
            vk::DescriptorSetLayoutBinding
            {
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment
            }
        };

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
        {
            .bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size()),
            .pBindings = descriptorSetLayoutBindings.data()
        };

        _layout = deviceContext.LogicalDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    }



}

