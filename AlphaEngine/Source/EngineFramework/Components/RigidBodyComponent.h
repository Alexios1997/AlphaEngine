#pragma once


#include <Jolt/Jolt.h> 
#include <Jolt/Physics/Body/BodyID.h>

namespace AlphaEngine
{
    struct RigidBodyComponent {
        JPH::BodyID bodyID;  
    };
}