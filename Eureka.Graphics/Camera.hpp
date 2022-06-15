#pragma once
   
#include <glm/gtc/matrix_transform.hpp>
#include <eigen_graphics.hpp>

namespace eureka
{
    constexpr float PERSPECTIVE_CAMERA_DEFAULT_FOV = 3.14f / 4.0f;

    class PerspectiveCamera
    {
    private:
        float        _fov;
        float        _zNear;
        float        _zFar;
        vk::Viewport _viewport;
    public:
        void SetPosition(const Eigen::Vector3f& position);
        void SetLookDirection(const  Eigen::Vector3f& direction);
        void SetDirectionRelativeToBase(const  Eigen::Vector3f& baseDirection, const  Eigen::Vector3f& xyzRotationDegrees);
        void SetFullViewport(float topLeftX, float topLeftY, float width, float height);
        void SetNearFar(float nearVal, float farVal);
        void SetVerticalFov(float verticalFovRadians);
        void SetHorizontalFov(float horizontalFovRadians);


    };
}