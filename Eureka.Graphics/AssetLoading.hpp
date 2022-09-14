#pragma once

#include "DeviceContext.hpp"
#include "GraphicsDefaults.hpp"
#include "Commands.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "OneShotCopySubmission.hpp"
#include "UploadRingBuffer.hpp"
#include "CommandsUtils.hpp"
#include "Mesh.hpp"
#include "Pipeline.hpp"
#include "Descriptors.hpp"
#include "AsyncDataLoader.hpp"

namespace tinygltf
{
    class Model;
    struct Primitive;
}

namespace eureka
{
    template<typename T>
    using dynamic_span = std::span<T, std::dynamic_extent>;

    template<typename T>
    using dynamic_cspan = std::span<const T, std::dynamic_extent>;

    template<typename T>
    dynamic_cspan<uint8_t> to_raw_span(const dynamic_cspan<T>& s)
    {
        return dynamic_cspan<uint8_t>(
            reinterpret_cast<const uint8_t*>(s.data()),
            s.size_bytes()
            );
    }

    struct ModelLoadingConfig
    {
        std::stop_token cancel;
    };

    struct LoadedModel
    {

    };

    struct PrimitiveDataView
    {
        dynamic_cspan<uint16_t> index_view;
        dynamic_cspan<float>    position_view;
        dynamic_cspan<float>    normal_view;
        dynamic_cspan<float>    uv_view;
        dynamic_cspan<float>    tangent_view;
    };

    struct PreparedModelImages
    {
        uint64_t                           total_image_memory = 0;
        std::vector<ImageStageUploadDesc>  upload_descriptors;
        std::vector<SampledImage2D>        device_images;
    };

    class AssetLoader
    {
    public:
        AssetLoader(
            DeviceContext& deviceContext,
            Queue queue,
            std::shared_ptr<AsyncDataLoader>          asyncDataLoader,
            std::shared_ptr<PipelineCache >           xpipelineCache,
            std::shared_ptr<MTDescriptorAllocator>    xdescPool,

            IOExecutor ioExecutor,
            PoolExecutor poolExecutor     
        );

        future_t<LoadedModel> LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config);

        PrimitiveBufferOffsets PreparePrimitiveVertexData(
            const PrimitiveDataView& primitiveView,
            uint64_t& totalIndexBufferMemory,
            svec10<dynamic_cspan<uint8_t>>& indicesUploadDesc,
            uint64_t& totalVertexBufferMemory,
            svec10<dynamic_cspan<uint8_t>>& vertexDataUploadDesc
        )
        {
            PrimitiveBufferOffsets bufferOffsets{};
            bufferOffsets.index_offset = totalIndexBufferMemory;
            totalIndexBufferMemory += indicesUploadDesc.emplace_back(to_raw_span(primitiveView.index_view)).size_bytes();
            bufferOffsets.position_offset = totalVertexBufferMemory;
            totalVertexBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.position_view)).size_bytes();
            bufferOffsets.normal_offset = totalVertexBufferMemory;
            totalVertexBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.normal_view)).size_bytes();
            bufferOffsets.uv_offset = totalVertexBufferMemory;
            totalVertexBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.uv_view)).size_bytes();
            bufferOffsets.tangent_offset = totalVertexBufferMemory;
            totalVertexBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.tangent_view)).size_bytes();

            return bufferOffsets;
        }


    private:
        std::atomic_bool                                     _busy{ false };
        DeviceContext&                                       _deviceContext;
        Queue                                                _copyQueue;
        std::shared_ptr<AsyncDataLoader>       _asyncDataLoader;
        std::shared_ptr<MTDescriptorAllocator>                      _descPool;
        std::shared_ptr<PipelineCache>                       _pipelineCache;


        IOExecutor                                           _ioExecutor;
        PoolExecutor                                         _poolExecutor;

        CommandPool                                          _uploadCommandPool;


        PreparedModelImages PrepareImages(tinygltf::Model& gltfModel);

    };

}