#pragma once

#include <macros.hpp>
#include "Result.hpp"
#include "Device.hpp"

namespace eureka::vulkan
{
    class LinearCommandBufferHandle
    {
        VkCommandBuffer _commandBuffer{ nullptr };

        LinearCommandBufferHandle(VkCommandBuffer commandBuffer);
        friend class LinearCommandPool;
    public:
        VkCommandBuffer Get() const { return _commandBuffer; }
        void Begin();
        void End();
        void PipelineBarrier(
            VkPipelineStageFlags                        srcStageMask,
            VkPipelineStageFlags                        dstStageMask,
            VkDependencyFlags                           dependencyFlags,
            uint32_t                                    memoryBarrierCount,
            const VkMemoryBarrier* pMemoryBarriers,
            uint32_t                                    bufferMemoryBarrierCount,
            const VkBufferMemoryBarrier* pBufferMemoryBarriers,
            uint32_t                                    imageMemoryBarrierCount,
            const VkImageMemoryBarrier* pImageMemoryBarriers
        
        )
        {
            vkCmdPipelineBarrier(_commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        }
    
        void PipelineBarrier(const VkDependencyInfo& dependencyInfo)
        {
            vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
        }

        void CopyBufferToImage(
            VkBuffer                 srcBuffer,
            VkImage                  dstImage,
            VkImageLayout            dstImageLayout,
            uint32_t                 regionCount,
            const VkBufferImageCopy* pRegions
        )
        {
            vkCmdCopyBufferToImage(_commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        }

        void CopyBufferToImage(
            const VkCopyBufferToImageInfo2& info
        )
        {
            vkCmdCopyBufferToImage2(_commandBuffer, &info);
        }
        
        void CopyBuffer(
            VkBuffer            srcBuffer,
            VkBuffer            dstBuffer,
            uint32_t            regionCount,
            const VkBufferCopy* pRegions)
        {
            vkCmdCopyBuffer(_commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        }

        void Bind(
            VkPipelineBindPoint bindPoint,
            VkPipelineLayout pipelineLayout,
            VkDescriptorSet set,
            uint32_t index
        ) const
        {
            vkCmdBindDescriptorSets(
                _commandBuffer,
                bindPoint,
                pipelineLayout,
                index,
                1u,
                &set,
                0u,
                nullptr
            );
        }

        void BeginRenderPass(const VkRenderPassBeginInfo& beginInfo) const
        {
            vkCmdBeginRenderPass(
                _commandBuffer,
                &beginInfo,
                VK_SUBPASS_CONTENTS_INLINE
            );
        }

        void BindGraphicsPipeline(VkPipeline pipeline) const
        {
            vkCmdBindPipeline(
                _commandBuffer,
                VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline
            );
        }
        
        void SetViewport(const VkViewport& viewport) const
        {
            vkCmdSetViewport(_commandBuffer, 0u, 1u, &viewport);
        }

        void BindIndexBuffer(
            VkBuffer buffer,
            VkIndexType indexType,
            uint64_t offset = 0
            )
        {
            vkCmdBindIndexBuffer(
                _commandBuffer,
                buffer,
                offset,
                indexType
            );
        }
        void BindVertexBuffers(
            dcspan< VkBuffer> buffers,
            dcspan<uint64_t> offsets,
            uint32_t firstBinding = 0
        )
        {
            vkCmdBindVertexBuffers(
                _commandBuffer,
                firstBinding,
                static_cast<uint32_t>(buffers.size()),
                buffers.data(),
                offsets.data()
            );
        }

        void BindVertexBuffer(
            VkBuffer buffer,
            uint64_t offset,
            uint32_t firstBinding = 0
        )
        {
            vkCmdBindVertexBuffers(
                _commandBuffer,
                firstBinding,
                1u,
                &buffer,
                &offset
            );
        }
        template<typename BlockT>
        void PushConstants(VkPipelineLayout pipelineLayout, VkShaderStageFlagBits stages, const BlockT& block, uint32_t offset = 0) const
        {
            vkCmdPushConstants(
                _commandBuffer,
                pipelineLayout,
                stages,
                offset,
                sizeof(BlockT),
                &block
            );
        }

        void SetScissor(const VkRect2D& scissorRect) const
        {
            vkCmdSetScissor(_commandBuffer, 0u, 1, &scissorRect);
        }

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) const
        {
            vkCmdDrawIndexed(_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

        }

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const
        {
            vkCmdDraw(_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        void EndRenderPass() const
        {
            vkCmdEndRenderPass(_commandBuffer);
        }
    };

    class ScopedCommands
    {
        LinearCommandBufferHandle& _commandBuffer;
    public:
        EUREKA_NO_COPY_NO_MOVE(ScopedCommands)
        ScopedCommands(LinearCommandBufferHandle& commandBuffer)
            : _commandBuffer(commandBuffer)
        {
            _commandBuffer.Begin();
        }
        ~ScopedCommands()
        {
            _commandBuffer.End();
        }
    };

    class LinearCommandPool
    {    
        std::shared_ptr<Device> _device{nullptr};
        VkCommandPool _pool{nullptr};
    public:
        LinearCommandPool(LinearCommandPool&& that);
        LinearCommandPool& operator=(LinearCommandPool&& rhs);
        LinearCommandPool(const LinearCommandPool& that) = delete;
        LinearCommandPool& operator=(const LinearCommandPool& rhs) = delete;
        LinearCommandPool(std::shared_ptr<Device> device, uint32_t queueFamilyIndex);
        ~LinearCommandPool();
        void Reset();
        VkCommandPool Get() const { return _pool; } 
        LinearCommandBufferHandle AllocatePrimaryCommandBuffer() const;
    };



}