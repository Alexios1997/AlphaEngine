#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <unordered_set>
#include <set>
#include "EngineFramework/Logger.h"
#include <memory>

namespace AlphaEngine
{
	const uint16_t MAX_COMPONENTS = 32;
	// Signature: We use a bitset (1s and 0s) to keep track of which
	// components an entity has, and also helps keep track of which
	// entities a system is interested in.
	typedef std::bitset<MAX_COMPONENTS> Signature;

	struct IComponent
	{
	protected:
		static uint16_t nextId;
	};

	// Used to assign a unique id to a component type
	template <typename T>
	class Component : public IComponent
	{
		static int GetId()
		{
			static auto id = nextId++;
			return id;
		}
	};

	class Entity
	{
	private:
		uint16_t m_id;
	public:
		Entity(uint16_t id) : m_id(id) {};
		uint16_t GetId() const;

		Entity& operator = (const Entity& other) = default;
		bool operator == (const Entity& other) const { return m_id == other.m_id; }
		bool operator != (const Entity& other) const { return m_id == other.m_id; }
		bool operator > (const Entity& other) const { return m_id > other.m_id; }
		bool operator < (const Entity& other) const { return m_id < other.m_id; }

	};

	// The System processes entities that contain a specific Signature
	class System
	{
	private:
		Signature m_ComponentSignature;
		std::vector<Entity> m_Entities;

	public:
		System() = default;
		~System() = default;

		void AddEntityToSystem(Entity entity);
		void RemoveEntityFromSystem(Entity entity);
		const std::vector<Entity>& GetSystemEntities();
		const Signature& GetComponentSignature();

		template <typename T>
		void RequireComponent()
		{
			const auto componentId = Component<T>::GetId();
			m_ComponentSignature.set(componentId);
		}
	};



	class IComponentPool
	{
	public:
		virtual ~IComponentPool() {}
	};

	// Pool -> just a vector (contiguous data) of objects of type T
	template <typename T>
	class ComponentPool : public IComponentPool
	{
	private:
		std::vector<T> m_Data;
	public:
		ComponentPool(uint16_t size = 100) { m_Data.reserve(size); }
		virtual ~ComponentPool() = default;

		bool isEmpty() const { return m_Data.empty(); }
		uint16_t GetSize() const { return m_Data.size(); }
		void Resize(int n) { m_Data.resize(n); }
		void Clear() { m_Data.clear(); }
		void Add(T object) { m_Data.push_back(object); }
		void Set(uint16_t index, T object) { m_Data[index] = object; }
		T& Get(uint16_t index) { return static_cast<T&>(m_Data[index]); }
		const T& operator[](const uint16_t index) const { return m_Data[index]; }

	};



	// Registry -> Manages creation and destruction of entities, add systems and components
	class ECSOrchestrator
	{
	private:
		uint16_t m_NumEntities = 0;

		// vector of component pools, each pool contains all the 
		// data for a certain component type
		// vector index = component type id
		// pool index = entity id
		std::vector<std::shared_ptr<IComponentPool>> m_ComponentPools;

		// Vector of component signatures.
		// The signature lets us know which components are turned "on" for an entity
		// [vector index = entity id]
		std::vector<Signature> m_EntityComponentSignature;

		// Map of active systems [index = system typeid]
		std::unordered_map<std::type_index, std::shared_ptr<System>> m_Systems;

		// Set of entities that are flagged to be added or removed in the next
		// registry Update()
		std::set<Entity> m_EntitiesToBeAdded;
		std::set<Entity> m_EntitiesToBeDestroyed;

	public:
		ECSOrchestrator() { Logger::Log("Created The Orchestrator"); };
		~ECSOrchestrator() { Logger::Log("Destroyed The Orchestrator"); };

		// Update processes the entities that are waiting to be added/destroyed
		void UpdateEntitiesLifeTime();

		// Entity Management
		Entity CreateEntity();

		// Component Management
		template <typename T, typename ...TArgs>
		void AddComponent(Entity entity, TArgs&& ...args)
		{
			const auto componentId = Component<T>::GetId();
			const auto entityId = entity.GetId();

			// If not inside the vector
			if (componentId >= m_ComponentPools.size())
			{
				m_ComponentPools.resize(componentId + 1, nullptr);
			}

			// If no pools at thta index
			if (!m_ComponentPools[componentId])
			{
				std::shared_ptr<ComponentPool<T>> newComponentPool = std::make_shared<ComponentPool<T>>();
				m_ComponentPools[componentId] = newComponentPool;
			}

			// Fetch the pool either way
			std::shared_ptr<ComponentPool<T>> currentComponentPool = std::static_pointer_cast<ComponentPool<T>>(m_ComponentPools[componentId]);

			// If entity Id does not exist in that pool
			if (entityId >= currentComponentPool->GetSize())
			{
				currentComponentPool->Resize(m_NumEntities);
			}

			T newComponent(std::forward<TArgs>(args)...);

			currentComponentPool->Set(entityId, newComponent);
			m_EntityComponentSignature[entityId].set(componentId);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			const auto componentId = Component<T>::GetId();
			const auto entityId = entity.GetId();
			m_EntityComponentSignature[entityId].set(componentId, false);
		}

		template<typename T>
		void HasComponent(Entity entity)
		{
			const auto componentId = Component<T>::GetId();
			const auto entityId = entity.GetId();
			return m_EntityComponentSignature[entityId].test(componentId);
		}


		// System Management
		template <typename T, typename ...TArgs>
		void AddSystem(TArgs&& ...args)
		{
			std::shared_ptr<T> newSystem(std::make_shared<T>(std::forward<TArgs>(args)...));
			m_Systems.insert(std::make_pair( std::type_index(typeid(T)), newSystem));
		}

		template <typename T>
		void RemoveSystem()
		{
			auto system = m_Systems.find(std::type_index(typeid(T)));
			m_Systems.erase(system);
		}

		template <typename T>
		const bool HasSystem()
		{
			return m_Systems.find(std::type_index(typeid(T))) != m_Systems.end();
		}

		template <typename T>
		const T& GetSystem()
		{
			auto system = m_Systems.find(std::type_index(typeid(T)));
			//return *(std::static_pointer_cast<T>(system->second));
		}

		// Checks the component signature of an entity and add
		// the entity to the systems that are interested in it
		void AddEntityToSystems(Entity entity);

	};
}