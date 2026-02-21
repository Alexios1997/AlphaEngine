#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <jolt/Physics/Collision/ContactListener.h>
#include "EngineFramework/Logger.h"
#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/GameplayEvents.h"
#include "EngineFramework/EventBus.h"

#include <string>

namespace AlphaEngine
{

	// We use event queue because it is critical for Thread Safety,
	// Deterministic Logic and Stability
	class MainContactListener : public JPH::ContactListener
	{
	public:
		struct ContactData {
			JPH::BodyID id1, id2;
			bool isStart;
			bool isSensor;
		};

		std::vector<ContactData> m_EventQueue;
		std::mutex m_QueueMutex;
		JPH::PhysicsSystem* m_PhysicsSystem;


		MainContactListener(JPH::PhysicsSystem* physicsSystem)
			: m_PhysicsSystem(physicsSystem) {
		}

		// This is gonna be called when two bodies first touch
		// ContactManifold -> contains the Geometry of the hit
		// ContactSettings -> this allws us to change the physics at the moment of impact
		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			JPH::BodyID id1 = inBody1.GetID();
			JPH::BodyID id2 = inBody2.GetID();
			bool isSensor = inBody1.IsSensor() || inBody2.IsSensor();

			std::lock_guard<std::mutex> lock(m_QueueMutex);
			m_EventQueue.push_back({ id1, id2, true, isSensor });
			
		}

		// This is gonna be called when the bodies no longer touching
		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			std::lock_guard<std::mutex> lock(m_QueueMutex);
			m_EventQueue.push_back({
				inSubShapePair.GetBody1ID(),
				inSubShapePair.GetBody2ID(),
				false, 
				false  
				});
		}

	};

	class IgnoreSensorsFilter : public JPH::BodyFilter {
	public:
		IgnoreSensorsFilter(JPH::BodyInterface& bi) : mBI(bi) {}

		virtual bool ShouldCollide(const JPH::BodyID& inBodyID) const override {
			// Skip sensors so the ray passes right through them
			return !mBI.IsSensor(inBodyID);
		}
	private:
		JPH::BodyInterface& mBI;
	};
}