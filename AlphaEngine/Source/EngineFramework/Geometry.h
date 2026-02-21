#pragma once

#include <glm/glm.hpp>


namespace AlphaEngine {

    // For Spheres
    struct Sphere { glm::vec3 center; float radius; };

    // For boxes
    struct AABB { glm::vec3 min; glm::vec3 max; };

    // For rays
    struct Ray { glm::vec3 origin; glm::vec3 direction; };

    // For Planes
    struct Plane {
        glm::vec3 normal;
        float distance;
        float GetDistance(const glm::vec3& p) const { return glm::dot(normal, p) + distance; }
    };

    // For Frustum
    struct Frustum { Plane planes[6]; };
}