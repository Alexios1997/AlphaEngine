#pragma once

#include <cstdint>
#include <glm/gtc/type_ptr.hpp>

namespace AlphaEngine {

    struct CameraComponent {

        // So we can use LookAt
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Lens / Projection Properties
        float fov = 45.0f;
        float aspect = 16.0f / 9.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        bool isPrimary = true;

        // Matrices
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        glm::mat4 projectionMatrix = glm::mat4(1.0f);
        glm::mat4 viewProj = glm::mat4(1.0f);
    };
}