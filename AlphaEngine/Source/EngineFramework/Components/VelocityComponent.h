#pragma once

#include <glm/glm.hpp>

namespace AlphaEngine
{
    struct VelocityComponent {
        glm::vec3 linearVelocity = glm::vec3(0.0f);
        float gravity = -30.0f;
        bool useGravity = true;
        bool isGrounded = false;
    };
}