#pragma once
   
#include "DeviceContext.hpp"
#include "Mesh.hpp"
#include <eigen_graphics.hpp>

namespace eureka
{
    constexpr float PERSPECTIVE_CAMERA_DEFAULT_FOV = 3.14f / 4.0f;

    class PerspectiveCamera
    {
    private:
        float           _fovY{PERSPECTIVE_CAMERA_DEFAULT_FOV};
        float           _zNear{0.01f};
        float           _zFar{1000.0f};
        Eigen::Vector3f _up{0.0f, 1.0f, 0.0f};
        Eigen::Vector3f _position{0.0f, 0.0f, 10.0f};
        Eigen::Vector3f _direction{0.0f, 0.0f, -1.0f};

        ViewProjection  _viewProjection;
        vk::Viewport    _viewport{.x = 0.0f, .y = 0.0f, .width = 100.0f, .height = 100.0f, .minDepth = 0.0f, .maxDepth = 1.0f };

        HostVisibleDeviceConstantBuffer             _constantBuffer;
        std::shared_ptr<RenderingThreadUpdateQueue> _updateQueue;
    public:
        PerspectiveCamera(DeviceContext& deviceContext);
        void SetPosition(const Eigen::Vector3f& position);
        void SetLookDirection(const  Eigen::Vector3f& direction);
        void SetDirectionRelativeToBase(const  Eigen::Vector3f& baseDirection, const  Eigen::Vector3f& xyzRotationRad);
        void SetFullViewport(float topLeftX, float topLeftY, float width, float height);
        void SetFullViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height);
        void SetNearFar(float zNear, float zFar);
        void SetVerticalFov(float verticalFovRadians);

        std::pair<vk::DescriptorType,vk::DescriptorBufferInfo>  DescriptorInfo() const
        {
            return std::make_pair(vk::DescriptorType::eUniformBuffer, _constantBuffer.DescriptorInfo());
        }
        const vk::Viewport& Viewport() const;
    private:
        void SyncTransforms();
    };
}