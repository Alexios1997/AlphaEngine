#include "EngineFramework/ECS/ECS.h"
#include "EngineFramework/Logger.h"
#include "EngineFramework/Systems/CameraSystem.h"
#include <algorithm>

namespace AlphaEngine
{
	uint16_t IComponent::nextId = 0;

	uint16_t Entity::GetId() const
	{
		return m_id;
	}

	void System::AddEntityToSystem(Entity entity)
	{
		uint16_t id = entity.GetId();
		if (id >= m_EntityToIndex.size()) m_EntityToIndex.resize(id + 500, -1);

		// Map the ID to the current end of the list
		m_EntityToIndex[id] = static_cast<int>(m_Entities.size());
		m_Entities.push_back(entity);
	}

	// remove if It is an O(n) operation. If we have 5000 entities in a system,
	// the Cpu has to look at every single one to find the match.
	// Then it has to shift all subsequen entities to fill the gap...
	// We need to use Swap and Pop -> Use a helper map to know exactly where the entity is O(1) for lookup
	void System::RemoveEntityFromSystem(Entity entity)
	{
		uint16_t id = entity.GetId();
		int indexToRemove = m_EntityToIndex[id];
		if (indexToRemove == -1) return;

		Entity lastEntity = m_Entities.back();
		m_Entities[indexToRemove] = lastEntity;
		// Instant update

		m_EntityToIndex[lastEntity.GetId()] = indexToRemove; 

		m_Entities.pop_back();
		m_EntityToIndex[id] = -1;
	}

	bool System::HasEntity(Entity entity) const
	{
		uint16_t id = entity.GetId();

		if (id >= m_EntityToIndex.size()) {
			return false;
		}

		// Check if the value is NOT -1 (Since we use -1 for "not in system logic")
		return m_EntityToIndex[id] != -1;
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

		if (m_FreeIDs.empty())
		{
			// If there are no free ids waiting to be used
			entityId = m_NumEntities++;

			// Make sure the entityComponentSignatures vector can accomadate the new entity
			if (entityId >= m_EntityComponentSignature.size())
			{
				m_EntityComponentSignature.resize(entityId + 1);
			}
		}
		else 
		{
			// reuse from freeids now
			entityId = m_FreeIDs.back();
			m_FreeIDs.pop_back();

		}
	

		Entity entity(entityId);
		m_EntitiesToBeAdded.push_back(entity);

		//AlphaEngine::Logger::Log("Entity Created with id = " + std::to_string(entityId));

		return entity;
	}

	void ECSOrchestrator::DestroyEntity(Entity entity)
	{
		m_EntitiesToBeDestroyed.push_back(entity);
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

	void ECSOrchestrator::RemoveEntityFromSystems(Entity entity)
	{
		for (auto& system : m_Systems)
		{
			system.second->RemoveEntityFromSystem(entity);
		}
	}

	// We do that Because we need to know in runtime and not only once the entity is created,
	// if this entity signature just changed. So refresh it if not already in the system.
	void ECSOrchestrator::RefreshEntity(Entity entity)
	{
		const auto entityId = entity.GetId();
		const auto& entityCompSignature = m_EntityComponentSignature[entityId];

		for (auto& pair : m_Systems) {
			auto& system = pair.second;

			const auto& systemSignature = system->GetComponentSignature();

			bool isInterested = (entityCompSignature & systemSignature) == systemSignature;

			// Already have the entity?
			bool alreadyInSystem = system->HasEntity(entity);

			if (isInterested && !alreadyInSystem) {
				system->AddEntityToSystem(entity);
			}
			else if (!isInterested && alreadyInSystem) {
				system->RemoveEntityFromSystem(entity);
			}
		}

	}

	void ECSOrchestrator::SetPrimaryCamera(Entity entity)
	{
		if (!entity.IsValid()) return;


		// Signature of The Component itself
		Signature cameraRequiredSignature;
		cameraRequiredSignature.set(Component<CameraComponent>::GetId());

		// The Signature of the Entity
		const auto entityId = entity.GetId();
		const auto& entityCompSignature = m_EntityComponentSignature[entityId];

		//Compare: Use bitwise AND to see if the camera bit is "on"
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

		for (auto entity : m_EntitiesToBeDestroyed)
		{
			uint16_t id = entity.GetId();

			RemoveEntityFromSystems(entity);

			for (auto& pool : m_ComponentPools) {
				if (pool) {
					pool->RemoveEntityFromPool(id);
				}
			}
			m_EntityComponentSignature[entity.GetId()].reset();
			m_FreeIDs.push_back(entity.GetId());
		}
		m_EntitiesToBeDestroyed.clear();

		// Process Refreshes once for all changes
		// We sort and unique the vector to avoid refreshing the same entity 5 times 
		// if you added 5 components to it in one frame.
		std::sort(m_EntitiesToRefresh.begin(), m_EntitiesToRefresh.end());
		m_EntitiesToRefresh.erase(std::unique(m_EntitiesToRefresh.begin(), m_EntitiesToRefresh.end()), m_EntitiesToRefresh.end());

		for (auto& entity : m_EntitiesToRefresh) {
			// Only refresh if the entity wasn't just destroyed!
			if (m_EntityComponentSignature[entity.GetId()].any()) {
				RefreshEntity(entity);
			}
		}
		m_EntitiesToRefresh.clear();
	}
}


