#pragma once
#include "DeviceContext.hpp"

namespace eureka
{
    inline constexpr uint32_t PER_VIEW_SET_000 = 000;

    struct DescriptorSetLayoutCreateInfoBundle
    {
        DescriptorSetLayoutCreateInfoBundle() = default;
        DescriptorSetLayoutCreateInfoBundle(std::size_t bindingCount)
            : bindings(bindingCount)
        {

        }
        uint32_t                                    set_num;
        vk::DescriptorSetLayoutCreateInfo           create_info;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };

    class DescriptorSetLayout
    {
    protected:
        vkr::DescriptorSetLayout _layout{ nullptr };
    public:
        EUREKA_DEFAULT_MOVEONLY(DescriptorSetLayout);
        DescriptorSetLayout(const vkr::Device& device, const vk::DescriptorSetLayoutCreateInfo& info);
        vk::DescriptorSetLayout Get() const { return *_layout; }
    };



    class DescriptorSetLayoutCache
    {
        static constexpr uint64_t MAX_SET_SLOTS = 4;
        using SetLayoutMap = std::map<uint32_t, DescriptorSetLayout>;

        mutable std::mutex _mtx;
        std::array<SetLayoutMap, MAX_SET_SLOTS> _layoutsMaps;
        std::shared_ptr<vkr::Device>            _device;

        DescriptorSetLayoutCreateInfoBundle CreateLayoutBundle(uint64_t set, uint32_t uniqueId);

    public:
        DescriptorSetLayoutCache(std::shared_ptr<vkr::Device> device)
            : _device(device)
        {

        }

        vk::DescriptorSetLayout RetrieveLayout(uint64_t set, uint32_t uniqueId) 
        {
            auto& map = _layoutsMaps.at(set);

            std::scoped_lock lk(_mtx);
            auto layoutItr = map.find(uniqueId);
            if (layoutItr != map.end())
            {
                return layoutItr->second.Get();
            }
            else
            {
                auto layoutBundle = CreateLayoutBundle(set, uniqueId);
                DescriptorSetLayout layout(*_device, layoutBundle.create_info);
                auto desc = layout.Get();
                map.emplace(uniqueId, std::move(layout));
                return desc;
            }
        }


        //vk::DescriptorSetLayout CreateLayout(const vk::DescriptorSetLayoutCreateInfo& createInfo, uint64_t set, uint32_t uniqueId)
        //{
        //    auto& map = _layoutsMaps.at(set);
        //    std::scoped_lock lk(_mtx);

        //    auto layoutItr = map.find(uniqueId);
        //    if (layoutItr == map.end())
        //    {
        //        DescriptorSetLayout layout(*_device, createInfo);
        //        auto desc = layout.Get();
        //        map.emplace(uniqueId, std::move(layout));
        //        return desc;
        //    }
        //    else
        //    {
        //        return layoutItr->second.Get();
        //    }

        //}

        //vk::DescriptorSetLayout CreateLayout(uint64_t set, uint32_t uniqueId)
        //{

        //    auto layoutBundle = CreateLayoutBundle(set, uniqueId);
        //    return CreateLayout(layoutBundle.create_info, set, uniqueId);
        //}

    };


    class SingleVertexShaderUBODescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        SingleVertexShaderUBODescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(SingleVertexShaderUBODescriptorSetLayout);
    };

    class SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout);
    };

    class ColorAndNormalMapFragmentDescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        ColorAndNormalMapFragmentDescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(ColorAndNormalMapFragmentDescriptorSetLayout);
    };
}

