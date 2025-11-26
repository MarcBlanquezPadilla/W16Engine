#include "CCamera.h"

CCamera::CCamera()
{
    position = glm::vec3(0.0f, 0.0f, 5.0f);
    reference = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);

    fov = 60.0f;
    aspectRatio = 16.0f / 9.0f;
    zNear = 0.1f;
    zFar = 1000.0f;

    LookAt(position, reference, up);
    SetPerspective(fov, aspectRatio, zNear, zFar);
}

CCamera::~CCamera()
{

}

void CCamera::SetPerspective(float fov, float aspect, float near, float far)
{
    this->fov = fov;
    this->aspectRatio = aspect;
    this->zNear = near;
    this->zFar = far;

    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);

    UpdateFrustum();
}

void CCamera::LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& _up)
{
    this->position = eye;
    this->reference = center;
    this->up = _up;

    viewMatrix = glm::lookAt(eye, center, up);

    UpdateFrustum();
}

void CCamera::UpdateFrustum()
{
    glm::mat4 viewProj = projectionMatrix * viewMatrix;

    frustum.Update(viewProj);
}