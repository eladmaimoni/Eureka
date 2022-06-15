#pragma once
   
#include <glm/gtc/matrix_transform.hpp>
namespace eureka
{

    class PerspectiveCamera
    {
    private:

        bool updated = false;

        float _fov;
        float _zNear, _zFar;
        glm::vec3 _rotation = glm::vec3();
        glm::vec3 _position = glm::vec3();
        glm::vec4 _viewPosition = glm::vec4();
        glm::mat4 _perspective;
        glm::mat4 _view;
        void UpdateViewMatrix()
        {
            glm::mat4 rotM = glm::mat4(1.0f);
            glm::mat4 transM;

            rotM = glm::rotate(rotM, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            rotM = glm::rotate(rotM, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rotM = glm::rotate(rotM, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            glm::vec3 translation = _position;

            transM = glm::translate(glm::mat4(1.0f), translation);
             _view = transM * rotM;


            _viewPosition = glm::vec4(_position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

            updated = true;
        };
    public:
        float NearClip() const
        {
            return _zNear;
        }

        float FarClip() const
        {
            return _zFar;
        }

        void SetPerspective(float fov, float aspect, float znear, float zfar)
        {
            _fov = fov;
            _zNear = znear;
            _zFar = zfar;
            _perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);

        };

        void UpdateAspectRatio(float aspect)
        {
            _perspective = glm::perspective(glm::radians(_fov), aspect, _zNear, _zFar);
   
        }

        void setPosition(glm::vec3 position)
        {
            this->_position = position;
            UpdateViewMatrix();
        }

        void setRotation(glm::vec3 rotation)
        {
            this->_rotation = rotation;
            UpdateViewMatrix();
        }

        void rotate(glm::vec3 delta)
        {
            this->_rotation += delta;
            UpdateViewMatrix();
        }

        void setTranslation(glm::vec3 translation)
        {
            this->_position = translation;
            UpdateViewMatrix();
        };

        void translate(glm::vec3 delta)
        {
            this->_position += delta;
            UpdateViewMatrix();
        }


    };
}