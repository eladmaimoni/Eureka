#include "Camera.hpp"
#include <Eigen/Geometry>
#include <eigen_graphics.hpp>

namespace eureka
{

    CameraNode::CameraNode(DeviceContext& deviceContext) :
        _constantBuffer(deviceContext.Allocator(), BufferConfig{ .byte_size = static_cast<uint32_t>(sizeof(ViewProjection)) })
    {

    }

    const vk::Viewport& CameraNode::Viewport() const
    {
        return _viewport;
    }

    void CameraNode::SetViewport(float topLeftX, float topLeftY, float width, float height)
    {
        _viewport.width = width;
        _viewport.height = height;
        _viewport.x = topLeftX;
        _viewport.y = topLeftY;
    }

    void CameraNode::SetTransforms(const ViewProjection& viewProjection)
    {
        _viewProjection = viewProjection;
    }

    void CameraNode::SyncTransforms()
    {
        _constantBuffer.Assign(_viewProjection);
    }




    PerspectiveCamera::PerspectiveCamera(std::shared_ptr<CameraNode> node)
        :
        _node(std::move(node))
    {
        auto viewport = _node->Viewport();
        Eigen::Vector3f center = _front + _direction;
        _viewProjection.view = Eigen::lookAt(_front, center, _up);
        auto aspectRatio = viewport.width / viewport.height;
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

    }

    void PerspectiveCamera::SetPosition(const Eigen::Vector3f& position)
    {
        _front = position;
        Eigen::Vector3f center = _front + _direction;
        _viewProjection.view = Eigen::lookAt(_front, center, _up);
        _node->SetTransforms(_viewProjection);
    }

    void PerspectiveCamera::SetLookDirection(const Eigen::Vector3f& direction)
    {
        _direction = direction;
        Eigen::Vector3f center = _front + _direction;
        _viewProjection.view = Eigen::lookAt(_front, center, _up);
        _node->SetTransforms(_viewProjection);
    }

    void PerspectiveCamera::SetDirectionRelativeToBase(const Eigen::Vector3f& baseDirection, const Eigen::Vector3f& xyzRotationRad)
    {
        Eigen::Matrix3f rotation = (
            Eigen::AngleAxisf(xyzRotationRad[0], Eigen::Vector3f::UnitZ()) *
            Eigen::AngleAxisf(xyzRotationRad[1], Eigen::Vector3f::UnitX()) *
            Eigen::AngleAxisf(xyzRotationRad[2], Eigen::Vector3f::UnitZ())
        ).toRotationMatrix();
        
        
        SetLookDirection(rotation * baseDirection);

        _node->SetTransforms(_viewProjection);
    }

    void PerspectiveCamera::SetViewport(float topLeftX, float topLeftY, float width, float height)
    {
        auto aspectRatio = width / height;
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

        _node->SetViewport(topLeftX, topLeftY, width, height);
        _node->SetTransforms(_viewProjection);
    }

    void PerspectiveCamera::SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
    {
        SetViewport(
            static_cast<float>(topLeftX),
            static_cast<float>(topLeftY),
            static_cast<float>(width), 
            static_cast<float>(height)
        );
    }

    void PerspectiveCamera::SetNearFar(float zNear, float zFar)
    {
        auto viewport = _node->Viewport();
        _zNear = zNear;
        _zFar = zFar;
        auto aspectRatio = viewport.width / viewport.height;
  
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

        _node->SetTransforms(_viewProjection);

    }

    void PerspectiveCamera::SetVerticalFov(float verticalFovRadians)
    {
        auto viewport = _node->Viewport();
        _fovY = verticalFovRadians;
        auto aspectRatio = viewport.width / viewport.height;
        _viewProjection.projection = Eigen::perspective(_fovY, aspectRatio, _zNear, _zFar);

        _node->SetTransforms(_viewProjection);
    }





}