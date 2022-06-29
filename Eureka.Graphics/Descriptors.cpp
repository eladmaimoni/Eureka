#include "Descriptors.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{
    // TODO: these numbers should be much higher and possibly deduces from the scene
    // roughly - the number of sets should be at least the number of different 
    // models we draw as every model usually requires a different set of textures
    inline constexpr uint32_t DEFAULT_MAX_DESCRIPTOR_SETS = 20;
    inline constexpr uint32_t DEFAULT_MAX_UBO_DESCRIPTORS = 3;
    inline constexpr uint32_t DEFAULT_MAX_COMBINED_IMAGE_SAMPLERS_DESCRIPTORS = 50;


    DescriptorPool::DescriptorPool(DeviceContext& deviceContext)
        : _device(deviceContext.LogicalDevice())
    {
        // We need to tell the API the number of max. requested descriptors per type
        std::array<vk::DescriptorPoolSize, 2> perTypeMaxCount{};
        perTypeMaxCount[0] = vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = DEFAULT_MAX_UBO_DESCRIPTORS };
        perTypeMaxCount[1] = vk::DescriptorPoolSize{ .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = DEFAULT_MAX_COMBINED_IMAGE_SAMPLERS_DESCRIPTORS };

        // For additional types you need to add new entries in the type count list
        // E.g. for two combined image samplers :
        // typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // typeCounts[1].descriptorCount = 2;

        // Create the global descriptor pool
        // All descriptors used in this example are allocated from this pool
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo
        {
            .maxSets = DEFAULT_MAX_DESCRIPTOR_SETS,
            .poolSizeCount = static_cast<uint32_t>(perTypeMaxCount.size()),
            .pPoolSizes = perTypeMaxCount.data()
        };
        _pool = _device->createDescriptorPool(descriptorPoolCreateInfo);
    }


    vkr::DescriptorSet DescriptorPool::AllocateSet(vk::DescriptorSetLayout layout)
    {
        vk::DescriptorSetAllocateInfo allocInfo
        {
            .descriptorPool = *_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout

        };
        auto device = **_device;
        vk::DescriptorSet descriptorSet{};

        VK_CHECK(vkAllocateDescriptorSets(
            device,
            (VkDescriptorSetAllocateInfo*)&allocInfo,
            (VkDescriptorSet*)&descriptorSet)
        );

        return vkr::DescriptorSet(*_device, descriptorSet, *_pool);
    }
}
