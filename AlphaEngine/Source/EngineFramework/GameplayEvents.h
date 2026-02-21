#pragma once

#include "EngineFramework/Event.h"
#include "EngineFramework/ECS/ECS.h"
#include <format>

namespace AlphaEngine
{
	class CollisionEvent : public Event
	{
	public:

		CollisionEvent(Entity e1, Entity e2, bool entered, bool sensor)
			: m_EntityA(e1), m_EntityB(e2), m_IsStart(entered), m_IsSensor(sensor) {
		}

		// inline is a hint to the compiler to replace the function call with the 
		// actual code of the function
		// We are doing that because calling GetEntity millions of times a normal function
		// call has "overhead" (the CPU has to jump to diff memory address and come back. 
		// inline eliminates that jump. Just for returning a variable inline is a mush

		inline Entity GetEntityA() const { return m_EntityA; }
		inline Entity GetEntityB() const { return m_EntityB; }
		inline bool IsStart() const { return m_IsStart; }
		inline bool IsSensor() const { return m_IsSensor; }

		std::string ToString() const override
		{
			return std::format("CollisionEvent: {} and {} [Start: {}, Sensor: {}]",
				m_EntityA.GetId(), m_EntityB.GetId(), m_IsStart, m_IsSensor);
		}

		EVENT_CLASS_TYPE(Collision)
	private:
		Entity m_EntityA, m_EntityB;
		bool m_IsStart;
		bool m_IsSensor;

	};
}