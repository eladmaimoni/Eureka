#include "DescriptorSetsLayout.hpp"
#include "move.hpp"

namespace eureka::vulkan
{
    DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device, const VkDescriptorSetLayoutCreateInfo& info)
        :
        _device(std::move(device)),
        _layout(_device->CreateDescriptorSetLayout(info))
    {

    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& that)
        : 
        _device(std::move(that._device)),
        _layout(steal(that._layout))
    {

    }

    DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& rhs)
    {
        _device = std::move(rhs._device);
        _layout = steal(rhs._layout);
        return *this;
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (_layout)
        {
            _device->DestroyDescriptorSetLayout(_layout);
        }
    }


    //DescriptorSetLayout::DescriptorSetLayout(const vkr::Device& device, const vk::DescriptorSetLayoutCreateInfo& info)
    //    : _layout(
    //        
    //        
    //        device.createDescriptorSetLayout(info))
    //{
    //}

    //SingleVertexShaderUBODescriptorSetLayout::SingleVertexShaderUBODescriptorSetLayout(DeviceContext& deviceContext)
    //{
    //    // describe the relation between the shader indices (set 0, binding 0)
    //    // to the host indices 
    //    // - host indices are the sets
    //    // - device indices are the set index and binding number within the set that can potentially be unordered
    //    // set 0, only has a single binding inside shader
    //    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding
    //    {
    //        .binding = 0, // shader side index (why not named location??)
    //        .descriptorType = vk::DescriptorType::eUniformBuffer,
    //        .descriptorCount = 1, // a single constant buffer
    //        .stageFlags = vk::ShaderStageFlagBits::eVertex
    //    };

    //    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
    //    {
    //        .bindingCount = 1,
    //        .pBindings = &descriptorSetLayoutBinding
    //    };

    //    _layout = deviceContext.LogicalDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    //}

    //ColorAndNormalMapFragmentDescriptorSetLayout::ColorAndNormalMapFragmentDescriptorSetLayout(DeviceContext& deviceContext)
    //{
    //    std::array<vk::DescriptorSetLayoutBinding, 2> descriptorSetLayoutBindings
    //    {
    //        vk::DescriptorSetLayoutBinding
    //        {
    //        .binding = 0,
    //        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
    //        .descriptorCount = 1,
    //        .stageFlags = vk::ShaderStageFlagBits::eFragment
    //        },
    //        vk::DescriptorSetLayoutBinding
    //        {
    //        .binding = 1,
    //        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
    //        .descriptorCount = 1,
    //        .stageFlags = vk::ShaderStageFlagBits::eFragment
    //        }
    //    };

    //    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
    //    {
    //        .bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size()),
    //        .pBindings = descriptorSetLayoutBindings.data()
    //    };

    //    _layout = deviceContext.LogicalDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    //}



    //DescriptorSetLayoutCreateInfoBundle DescriptorSetLayoutCache::CreateLayoutBundle(uint64_t /*set*/, uint32_t uniqueId)
    //{
    //    DescriptorSetLayoutCreateInfoBundle bundle;
    //    switch (uniqueId)
    //    {
    //    case SET0_ID_000_PER_VIEW:
    //        
    //        bundle.bindings.emplace_back(
    //            vk::DescriptorSetLayoutBinding 
    //            {
    //                .binding = 0, // shader side index (why not named location??)
    //                .descriptorType = vk::DescriptorType::eUniformBuffer,
    //                .descriptorCount = 1, // a single constant buffer
    //                .stageFlags = vk::ShaderStageFlagBits::eVertex
    //            }
    //        );
    //        break;
    //    case SET0_ID_001_PER_FONT:
    //        bundle.bindings.emplace_back(
    //            vk::DescriptorSetLayoutBinding
    //            {
    //                .binding = 0,
    //                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
    //                .descriptorCount = 1,
    //                .stageFlags = vk::ShaderStageFlagBits::eFragment
    //            }
    //        );
    //        break;
    //    default:
    //        throw std::invalid_argument("bad");

    //    }

    //    bundle.create_info.bindingCount = static_cast<uint32_t>(bundle.bindings.size());
    //    bundle.create_info.pBindings = bundle.bindings.data();

    //    return bundle;
    //}

    //vk::DescriptorSetLayout DescriptorSetLayoutCache::RetrieveLayout(uint64_t set, uint32_t uniqueId)
    //{
    //    auto& map = _layoutsMaps.at(set);

    //    std::scoped_lock lk(_mtx);
    //    auto layoutItr = map.find(uniqueId);
    //    if (layoutItr != map.end())
    //    {
    //        return layoutItr->second.Get();
    //    }
    //    else
    //    {
    //        auto layoutBundle = CreateLayoutBundle(set, uniqueId);
    //        DescriptorSetLayout layout(*_device, layoutBundle.create_info);
    //        auto desc = layout.Get();
    //        map.emplace(uniqueId, std::move(layout));
    //        return desc;
    //    }
    //}



}

