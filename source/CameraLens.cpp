#include "CameraLens.h"
#include "Engine.h"
#include "Window.h"
#include "utils/Frustum.h"
#include "utils/Ray.h"
#include "utils/Log.h"

#include "glad/glad.h"

CameraLens::CameraLens()
{
    frustum = new Frustum();

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

CameraLens::~CameraLens()
{

}

void CameraLens::SetPerspective(float fov, float aspect, float near, float far)
{
    this->fov = fov;
    this->aspectRatio = aspect;
    this->zNear = near;
    this->zFar = far;

    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);

    UpdateFrustum();
}

void CameraLens::LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& _up)
{
    this->position = eye;
    this->reference = center;
    this->up = _up;

    viewMatrix = glm::lookAt(eye, center, up);

    UpdateFrustum();
}

void CameraLens::UpdateFrustum()
{
    glm::mat4 viewProj = projectionMatrix * viewMatrix;
    frustum->Update(viewProj);
}

Ray CameraLens::GetRayFromMouse(int mouseX, int mouseY, int width, int height)
{
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);

    glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    glm::vec3 ray_wor = glm::vec3(glm::inverse(viewMatrix) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    Ray ray;
    ray.origin = position;
    ray.direction = glm::normalize(ray_wor);

    return ray;
}

bool CameraLens::CleanUp()
{
    delete frustum;
    return true;
}

void CameraLens::SetRenderTarget(int width, int height)
{
    // 1. SEGURIDAD: Evitar recrear si el tamaño es el mismo (Opcional pero recomendado)
    if (this->textureWidth == width && this->textureHeight == height && fboID != 0)
        return;

    // 2. LIMPIEZA PREVIA (El patrón "Delete-Then-Create")
    // Si ya existían texturas viejas (ej: de cuando la ventana era más pequeña), las borramos.
    if (fboID != 0)
    {
        glDeleteFramebuffers(1, &fboID);
        fboID = 0;
    }
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    if (rboID != 0)
    {
        glDeleteRenderbuffers(1, &rboID);
        rboID = 0;
    }

    // 3. GUARDAR NUEVO TAMAÑO
    this->textureWidth = width;
    this->textureHeight = height;

    // 4. CREAR EL FRAMEBUFFER (El "Caballete")
    glGenFramebuffers(1, &fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    // 5. CREAR LA TEXTURA (El "Lienzo")
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Reservar memoria para la imagen (RGBA, tamaño width x height)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Configurar filtrado (para que se vea bien al escalar)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Pegar la textura al Framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    // 6. CREAR EL RENDERBUFFER (Para profundidad y stencil)
    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    // Pegar el RBO al Framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboID);

    // 7. COMPROBAR ERRORES
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG("Error: Framebuffer de la cámara no está completo.");
    }

    // 8. DESVINCULAR (Volver a la normalidad)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}