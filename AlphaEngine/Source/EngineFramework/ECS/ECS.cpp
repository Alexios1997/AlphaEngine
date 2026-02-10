#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Logger.h"
#include "EngineFramework/Systems/CameraSystem.h"

namespace AlphaEngine
{
	uint16_t IComponent::nextId = 0;

	uint16_t Entity::GetId() const
	{
		return m_id;
	}

	void System::AddEntityToSystem(Entity entity)
	{
		m_Entities.push_back(entity);
	}

	void System::RemoveEntityFromSystem(Entity entity)
	{
		m_Entities.erase(std::remove_if(
			m_Entities.begin(),
			m_Entities.end(),
			[&entity](Entity other)
			{
				return entity == other;
			}),
			m_Entities.end()
		);
	}

	const std::vector<Entity>& System::GetSystemEntities() const
	{
		//Logger::Log("System tracking entities count: " + std::to_string(m_Entities.size()) + " for a specific system: ");

		return m_Entities;
	}

	const Signature& System::GetComponentSignature()
	{
		return m_ComponentSignature;
	}

	Entity ECSOrchestrator::CreateEntity()
	{

		uint16_t entityId;
		entityId = m_NumEntities++;

		Entity entity(entityId);
		m_EntitiesToBeAdded.insert(entity);

		// Make sure the entityComponentSignatures vector can accomadate the new entity
		if (entityId >= m_EntityComponentSignature.size())
		{
			m_EntityComponentSignature.resize(entityId + 1);
		}

		AlphaEngine::Logger::Log("Entity Created with id = " + std::to_string(entityId));

		return entity;
	}

	void ECSOrchestrator::AddEntityToSystems(Entity entity)
	{
		const auto entityId = entity.GetId();
		const auto& entityCompSignature = m_EntityComponentSignature[entityId];

		for (auto& system : m_Systems)
		{
			const auto& systemCompSignature = system.second->GetComponentSignature();

			bool isInterested = (entityCompSignature & systemCompSignature) == systemCompSignature;

			if (isInterested)
			{

				system.second->AddEntityToSystem(entity);
			}
		}
	}

	void ECSOrchestrator::SetPrimaryCamera(Entity entity)
	{
		if (!entity.IsValid()) return;

		Logger::Log("Check: ");

		// Signature of The Component itself
		Signature cameraRequiredSignature;
		cameraRequiredSignature.set(Component<CameraComponent>::GetId());

		// The Signature of the Entity
		const auto entityId = entity.GetId();
		const auto& entityCompSignature = m_EntityComponentSignature[entityId];

		// 3. Compare: Use bitwise AND to see if the camera bit is "on"
		// This checks: "Does entityCompSignature have the cameraRequiredSignature bit set?"
		bool hasCamera = (entityCompSignature & cameraRequiredSignature) == cameraRequiredSignature;

		if (hasCamera) {
			m_PrimaryCamera = entity;
			Logger::Log("Primary camera set to Entity ID: " + std::to_string(entityId));
		}
		else {
			Logger::Err("Failed to set Primary Camera: Entity lacks CameraComponent bit.");
		}

	}

	Entity ECSOrchestrator::GetPrimaryCamera() const
	{
		return m_PrimaryCamera;
	}

	void ECSOrchestrator::UpdateEntitiesLifeTime()
	{
		for (auto entity : m_EntitiesToBeAdded)
		{
			AddEntityToSystems(entity);
		}
		m_EntitiesToBeAdded.clear();

		// todo: REMOVE ENTITIES that are waiting to be killed
	}
}


