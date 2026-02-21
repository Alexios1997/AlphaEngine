#pragma once 

#include "Engineframework/Geometry.h"

namespace AlphaEngine
{
	class Intersection
	{
	public:
        // Culling Test
        static bool Intersects(const Frustum& f, const Sphere& s);
        static bool Intersects(const Frustum& f, const AABB& b);
	};
}