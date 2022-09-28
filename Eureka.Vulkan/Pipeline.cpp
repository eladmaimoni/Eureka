#include "Pipeline.hpp"
#include "move.hpp"

//#include "PipelineTypes.hpp"
//
//#include "PipelineHelpers.hpp"
//#include "ShaderReflection.hpp"

namespace eureka::vulkan
{
    PipelineLayout::PipelineLayout(PipelineLayout&& that)
        : 
        _device(std::move(that._device)),
        _pipelineLayout(steal(that._pipelineLayout))
    {
    }

    PipelineLayout& PipelineLayout::operator=(PipelineLayout&& rhs)
    {
        _device = std::move(rhs._device);
        _pipelineLayout = steal(rhs._pipelineLayout);

        return *this;
    }

    PipelineLayout::PipelineLayout(
        std::shared_ptr<Device> device,
        const VkPipelineLayoutCreateInfo& pipelineLayoutCreateInfo
    )
        : _device(std::move(device))
    {
        _pipelineLayout = _device->CreatePipelineLayout(pipelineLayoutCreateInfo);
    }

    PipelineLayout::~PipelineLayout()
    {
        if (_pipelineLayout)
        {
            _device->DestroyPipelineLayout(_pipelineLayout);
        }
    }

    Pipeline::Pipeline(
        std::shared_ptr<Device> device,
        std::shared_ptr<PipelineLayout> layout,
        std::shared_ptr<RenderPass> renderPass,
        const VkGraphicsPipelineCreateInfo& pipelineCreateInfo
    ) :
        _device(std::move(device)),
        _pipelineLayout(std::move(layout)),
        _renderPass(std::move(renderPass)),
        _pipeline(_device->CreatePipeline(pipelineCreateInfo))
    {

    }

    Pipeline::~Pipeline()
    {
        if (_pipeline)
        {
            _device->DestroyPipeline(_pipeline);
        }
    }

    Pipeline::Pipeline(Pipeline&& that)
        :
        _device(std::move(that._device)),
        _pipelineLayout(std::move(that._pipelineLayout)),
        _renderPass(std::move(that._renderPass)),
        _pipeline(steal(that._pipeline))
    {

    }

    Pipeline& Pipeline::operator=(Pipeline&& rhs)
    {
        _device = std::move(rhs._device);
        _pipelineLayout = std::move(rhs._pipelineLayout);
        _renderPass = std::move(rhs._renderPass);
        _pipeline = steal(rhs._pipeline);
        return *this;
    }


}