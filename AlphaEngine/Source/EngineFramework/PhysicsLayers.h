#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <cstdint>

namespace AlphaEngine 
{
    // Jolt uses 2-step filtering system
    // BroadPhaseLayer -> Like Layer Groups "folders"
    // ObjectLayer -> Tags kinda.


    // Category ID (e.g., "Player", "Debris", or "WAll"
    // We are doing that because we dont want to check every single object against
    // every other object (O(n^2)). Jolt uses these IDs to quickly ignore pairs.
    namespace PhysObjectLayers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }

    // Collision detection heppens in two steps: Broad Phase and Narrow Phase
    // Broad Phase is like a high level map. It uses simple boxes (AABBs) to see
    // which objects are roughly near each other.
    namespace PhysBroadPhaseLayers {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr unsigned int NUM_LAYERS = 2;
    }

    // Here is the logic of "Can ObjectLayer A hit ObjectLayerB?"
    class PhysObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override {
            switch (inLayer1) {
            case PhysObjectLayers::NON_MOVING: return inLayer2 == PhysObjectLayers::MOVING;
            case PhysObjectLayers::MOVING: return true;
            default: return false;
            }
        }
    };


    // This is like the Tranlator. Like "If in an object i ggive an ObjectLAYER (like "Player"),
    // which BroadPhaseLayer does it belong to?"
    class PhysBPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        PhysBPLayerInterfaceImpl() {
            mObjectToBroadPhase[PhysObjectLayers::NON_MOVING] = PhysBroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[PhysObjectLayers::MOVING] = PhysBroadPhaseLayers::MOVING;
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            switch ((JPH::BroadPhaseLayer::Type)inLayer)
            {
            case (JPH::BroadPhaseLayer::Type)PhysBroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)PhysBroadPhaseLayers::MOVING:		return "MOVING";
            default:														return "INVALID";
            }
        }
#endif
        virtual unsigned int GetNumBroadPhaseLayers() const override { return PhysBroadPhaseLayers::NUM_LAYERS; }
        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { return mObjectToBroadPhase[inLayer]; }

    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[PhysObjectLayers::NUM_LAYERS];
    };

    // This class is responsible to check if a BroadPhase group (like MOVING) should even bother
    // looking at another BroadPhase group (Like NON_Moving)
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
            switch (inLayer1) {
            case PhysObjectLayers::NON_MOVING:
                return inLayer2 == PhysBroadPhaseLayers::MOVING;
            case PhysObjectLayers::MOVING:
                return true;
            default:
                return false;
            }
        }
    };
}