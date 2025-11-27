#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Frustum;
struct Ray;

class CameraLens
{
public:
    CameraLens();
    ~CameraLens();

    void SetRenderTarget(int width, int height);
    void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
    void SetProjectionMatrix(glm::mat4 projectionMatrix);
    void SetViewMatrix(glm::mat4 viewMatrix);
    void LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    const glm::mat4& GetViewMatrix() const { return viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return projectionMatrix; }

    const Frustum* GetFrustum() const { return frustum; }
    bool GetActiveCamera() const { return activeCamera; }

    Ray GetRayFromMouse(int mouseX, int mouseY, int width, int height);

    bool CleanUp();

private:
    void UpdateFrustum();

public:
    unsigned int fboID = 0;
    unsigned int rboID = 0;
    unsigned int textureID = 0;
    int textureWidth = 0;
    int textureHeight = 0;

    glm::vec3 position;
    glm::vec3 reference;
    glm::vec3 up;

    float fov;
    float aspectRatio;
    float zNear;
    float zFar;    
    bool activeCamera;

private:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    Frustum* frustum = nullptr;


};