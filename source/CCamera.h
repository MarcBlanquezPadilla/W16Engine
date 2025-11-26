#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils/Frustum.h"

class CCamera
{
public:
    CCamera();
    ~CCamera();

    void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);

    void LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    const glm::mat4& GetViewMatrix() const { return viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return projectionMatrix; }

    const Frustum& GetFrustum() const { return frustum; }

public:
    glm::vec3 position;
    glm::vec3 reference;
    glm::vec3 up;

    float fov;
    float aspectRatio;
    float zNear;
    float zFar;

private:
    void UpdateFrustum();

private:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    Frustum frustum;
};