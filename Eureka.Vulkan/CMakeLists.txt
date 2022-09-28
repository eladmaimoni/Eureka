


set_source_group(
	runtime 
	"Instance.hpp" 
	"Instance.cpp"
	"Result.hpp" 
	"Result.cpp"
)

set_source_group(
	device 
	"Device.hpp" 
	"Device.cpp"
	"Queue.hpp" 
	"Queue.cpp"
	"Commands.hpp"
	"Commands.cpp"
)

set_source_group(
	objects
	"RenderPass.hpp"
    "RenderPass.cpp"
	"RenderTarget.hpp"
    "RenderTarget.cpp"
	"Pipeline.hpp"
    "Pipeline.cpp"
	"ShaderModule.hpp"
	"ShaderModule.cpp"
	"Synchronization.hpp"
	"Synchronization.cpp"
	"SwapChain.hpp"
	"SwapChain.cpp"
)

set_source_group(
	resources
	"ResourceAllocator.hpp"
	"ResourceAllocator.cpp"
	"ResourceUpload.hpp"
    "ResourceUpload.cpp"
	"Buffer.hpp"
	"Buffer.cpp"
	"Image.hpp"
	"Image.cpp"
	"Descriptor.hpp"
    "Descriptor.cpp"
	"DescriptorPool.hpp"
    "DescriptorPool.cpp"
	"DescriptorAllocators.hpp"
    "DescriptorAllocators.cpp"
	"DescriptorSetsLayout.hpp"
    "DescriptorSetsLayout.cpp" 
)

set_source_group(
    pipeline 
	"DescriptorLayoutCache.hpp"
	"DescriptorLayoutCache.cpp"
	"PipelineCache.hpp"
	"PipelineCache.cpp"
	"PipelineHelpers.hpp"
	"PipelinePresets.hpp"
	"PipelinePresets.cpp"
	"PipelineTypes.hpp"
	"ShadersCache.cpp"
    "ShadersCache.hpp"
)

set_source_group(
    infrastructure 
    "FrameContext.hpp"
    "FrameContext.cpp"
    "StageZone.hpp"
    "StageZone.cpp"
	"DescriptorLayoutCache.hpp"
	"DescriptorLayoutCache.cpp"
)


add_library(
	Eureka.Vulkan 
	${device}
	${infrastructure}
	${objects}
	${pipeline}
	${resources}
	${runtime}
) 
 
set_compiler_flags(Eureka.Vulkan) 

target_include_directories(
    Eureka.Vulkan
    PUBLIC 
)

target_link_libraries(
     Eureka.Vulkan 
     PUBLIC
	 Eureka.Core
	 Eureka.Shaders
     Eureka.Windowing 
	 volk::volk volk::volk_headers	 
	 unofficial::vulkan-memory-allocator::vulkan-memory-allocator
)
