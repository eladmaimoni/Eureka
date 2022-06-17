#pragma once
#include "DeviceContext.hpp"

namespace eureka
{
    inline constexpr uint32_t SET0_ID_000_PER_VIEW = 000;
    inline constexpr uint32_t SET0_ID_001_PER_FONT = 001;
    //inline constexpr uint32_t PER_VIEW_SET_000 = 000;

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

        vk::DescriptorSetLayout RetrieveLayout(uint64_t set, uint32_t uniqueId);
    };


    class SingleVertexShaderUBODescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        SingleVertexShaderUBODescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(SingleVertexShaderUBODescriptorSetLayout);
    };

    class ColorAndNormalMapFragmentDescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        ColorAndNormalMapFragmentDescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(ColorAndNormalMapFragmentDescriptorSetLayout);
    };
}

