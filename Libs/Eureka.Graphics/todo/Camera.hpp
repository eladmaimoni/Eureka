#pragma once
   
#include "DeviceContext.hpp"
#include "Buffer.hpp"
#include "PipelineTypes.hpp"

namespace eureka
{
    constexpr float PERSPECTIVE_CAMERA_DEFAULT_FOV = 3.14f / 4.0f;

    class CameraNode
    {
        /*
        // TODO note that if multiple frames are in flight, there should also be multiple camera nodes.
        does not perform any synchronization and must be externally synchronized
        */
        
        vk::Viewport                     _viewport{ .x = 0.0f, .y = 0.0f, .width = 100.0f, .height = 100.0f, .minDepth = 0.0f, .maxDepth = 1.0f };
        HostVisibleDeviceConstantBuffer  _constantBuffer;
        ViewProjection                   _viewProjection;
    public:
        CameraNode(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEONLY(CameraNode);
        std::pair<vk::DescriptorType, vk::DescriptorBufferInfo>  DescriptorInfo() const
        {
            return std::make_pair(vk::DescriptorType::eUniformBuffer, _constantBuffer.DescriptorInfo());
        }

        const vk::Viewport& Viewport() const;
        void SetViewport(float topLeftX, float topLeftY, float width, float height);
        void SetTransforms(const ViewProjection& viewProjection);

        //
        // may be potentially called after draw calls are submitted.
        // but user must ensure draws synchronization using semaphores
        //
        void SyncTransforms();
    };

    class PerspectiveCamera
    {
    private:
        float                       _fovY{ PERSPECTIVE_CAMERA_DEFAULT_FOV };
        float                       _zNear{ 0.01f };
        float                       _zFar{ 1000.0f };
        Eigen::Vector3f             _up{ 0.0f, 1.0f, 0.0f };
        Eigen::Vector3f             _front{ 0.0f, 0.0f, 10.0f };
        Eigen::Vector3f             _direction{ 0.0f, 0.0f, -1.0f };
        ViewProjection              _viewProjection;
        
        std::shared_ptr<CameraNode> _node;
    public:
        PerspectiveCamera(std::shared_ptr<CameraNode> node);
        void SetPosition(const Eigen::Vector3f& position);
        void SetLookDirection(const  Eigen::Vector3f& direction);
        void SetDirectionRelativeToBase(const  Eigen::Vector3f& baseDirection, const  Eigen::Vector3f& xyzRotationRad);
        void SetViewport(float topLeftX, float topLeftY, float width, float height);
        void SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height);
        void SetNearFar(float zNear, float zFar);
        void SetVerticalFov(float verticalFovRadians);
        const std::shared_ptr<CameraNode>& GetNode() {
            return _node;
        }
    };



}