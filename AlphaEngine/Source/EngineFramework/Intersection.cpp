#pragma once 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>          
#include <glm/gtx/component_wise.hpp> 
#include "Engineframework/Intersection.h"
#include <algorithm>

namespace AlphaEngine
{
    bool Intersection::Intersects(const Frustum& f, const Sphere& s) {
        for (int i = 0; i < 6; i++) {
            // If the sphere is behind any plane by more than its radius, it's outside
            if (f.planes[i].GetDistance(s.center) < -s.radius)
                return false;
        }
        return true;
    }

    bool Intersection::Intersects(const Frustum& f, const AABB& b) {
        for (int i = 0; i < 6; i++) {
            // Find the positive vertex (the one furthest along the plane normal)
            glm::vec3 p = b.min;
            if (f.planes[i].normal.x >= 0) p.x = b.max.x;
            if (f.planes[i].normal.y >= 0) p.y = b.max.y;
            if (f.planes[i].normal.z >= 0) p.z = b.max.z;

            if (f.planes[i].GetDistance(p) < 0)
                return false;
        }
        return true;
    }


}