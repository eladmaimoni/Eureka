#include "Camera.hpp"
#include <Eigen/Geometry>

namespace eureka
{

    PerspectiveCamera::PerspectiveCamera(DeviceContext& deviceContext) :
        _constantBuffer(deviceContext, BufferConfig{ .byte_size = static_cast<uint32_t>(sizeof(ViewProjection)) }),
        _updateQueue(deviceContext.UpdateQueue())
    {
        Eigen::Vector3f center = _position + _direction;
        _viewProjection.view = Eigen::lookAt(_position, center, _up);
        auto aspectRatio = _viewport.width / _viewport.height;
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

    }

    void PerspectiveCamera::SetPosition(const Eigen::Vector3f& position)
    {
        _position = position;
        Eigen::Vector3f center = _position + _direction;
        _viewProjection.view = Eigen::lookAt(_position, center, _up);
        SyncTransforms();
    }

    void PerspectiveCamera::SetLookDirection(const Eigen::Vector3f& direction)
    {
        _direction = direction;
        Eigen::Vector3f center = _position + _direction;
        _viewProjection.view = Eigen::lookAt(_position, center, _up);
        SyncTransforms();
    }

    void PerspectiveCamera::SetDirectionRelativeToBase(const Eigen::Vector3f& baseDirection, const Eigen::Vector3f& xyzRotationRad)
    {
        Eigen::Matrix3f rotation = (
            Eigen::AngleAxisf(xyzRotationRad[0], Eigen::Vector3f::UnitZ()) *
            Eigen::AngleAxisf(xyzRotationRad[1], Eigen::Vector3f::UnitX()) *
            Eigen::AngleAxisf(xyzRotationRad[2], Eigen::Vector3f::UnitZ())
        ).toRotationMatrix();
        
        
        SetLookDirection(rotation * baseDirection);

        SyncTransforms();
    }

    void PerspectiveCamera::SetFullViewport(float topLeftX, float topLeftY, float width, float height)
    {
        _viewport.width = width;
        _viewport.height = height;
        _viewport.x = topLeftX;
        _viewport.y = topLeftY;
        auto aspectRatio = width / height;
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

        SyncTransforms();
    }

    void PerspectiveCamera::SetFullViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
    {
        SetFullViewport(
            static_cast<float>(topLeftX),
            static_cast<float>(topLeftY),
            static_cast<float>(width), 
            static_cast<float>(height)
        );
    }

    void PerspectiveCamera::SetNearFar(float zNear, float zFar)
    {

        _zNear = zNear;
        _zFar = zFar;
        auto aspectRatio = _viewport.width / _viewport.height;
  
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

        SyncTransforms();

    }

    void PerspectiveCamera::SetVerticalFov(float verticalFovRadians)
    {
        _fovY = verticalFovRadians;
        auto aspectRatio = _viewport.width / _viewport.height;
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

        SyncTransforms();
    }

    const vk::Viewport& PerspectiveCamera::Viewport() const
    {
        return _viewport;
    }

    void PerspectiveCamera::SyncTransforms()
    {
        _updateQueue->EnqueueUpdate(
            [this, viewProjection = _viewProjection]()
            {
                _constantBuffer.Assign(viewProjection);
            }
        );
    }

}