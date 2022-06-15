#pragma once



namespace eureka
{
    struct vector3
    {
        float x;
        float y;
        float z;
    };

    struct ViewProjection
    {
        Eigen::Matrix4f view;
        Eigen::Matrix4f projection;
    };

    struct PositionColorVertex
    {
        Eigen::Vector3f position;
        Eigen::Vector3f color;
    };
}