#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <unordered_set>
#include <set>
#include "EngineFramework/Logger.h"
#include "EngineFramework/ServiceLocator.h"
#include <memory>
#include <cassert>

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
	public:
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
		// Using the max value as Null
		static constexpr uint16_t NullID = 0xFFFF;

		Entity() : m_id(NullID) {};
		Entity(uint16_t id) : m_id(id) {};
		Entity(const Entity& entity) = default;

		uint16_t GetId() const;
		bool IsValid() const { return m_id != NullID; }

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
		const std::vector<Entity>& GetSystemEntities() const;
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
		//void Add(T object) { m_Data.push_back(object); }
		void Add(T&& object) { m_Data.push_back(std::move(object)); }
		void Set(uint16_t index, T object) { m_Data[index] = object; }
		T& Get(uint16_t index) { assert(index < m_Data.size()); return m_Data[index]; }
		const T& operator[](const uint16_t index) const { return m_Data[index]; }

	};



	// Registry -> Manages creation and destruction of entities, add systems and components
	class ECSOrchestrator : public IService
	{
	private:
		uint16_t m_NumEntities = 0;

		// vector of component pools, each pool contains all the 
		// data for a certain component type
		// vector index = component type id
		// pool index = entity id
		std::vector<std::unique_ptr<IComponentPool>> m_ComponentPools;

		// Vector of component signatures.
		// The signature lets us know which components are turned "on" for an entity
		// [vector index = entity id]
		std::vector<Signature> m_EntityComponentSignature;

		// Map of active systems [index = system typeid]
		// TODO: --------------------> We could potentially use just a vector
		// where the index is the System ID maybe , Little faster
		std::unordered_map<std::type_index, std::unique_ptr<System>> m_Systems;

		// Set of entities that are flagged to be added or removed in the next
		// registry Update()
		std::set<Entity> m_EntitiesToBeAdded;
		std::set<Entity> m_EntitiesToBeDestroyed;

		// Primary Camera ---------------> This creates Couple with!
		Entity m_PrimaryCamera;

	public:
		ECSOrchestrator() { Logger::Log("Created The Orchestrator"); };
		virtual ~ECSOrchestrator() { Logger::Log("Destroyed The Orchestrator"); };
		virtual void InitService() override { Logger::Log("Initializing Service named : ECS Orchestrator"); };

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
				m_ComponentPools.resize(componentId + 1);
			}

			// If no pools at thta index
			if (!m_ComponentPools[componentId])
			{			
				m_ComponentPools[componentId] = std::make_unique<ComponentPool<T>>();
			}

			// Fetch the pool either way,
			// When just using the pool we can use a raw pointer
			auto* currentCompPool = static_cast<ComponentPool<T>*>(m_ComponentPools[componentId].get());

			// If entity Id does not exist in that pool
			if (entityId >= currentCompPool->GetSize())
			{
				currentCompPool->Resize(m_NumEntities);
			}

			T newComponent(std::forward<TArgs>(args)...);

			currentCompPool->Set(entityId, newComponent);
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
		void HasComponent(Entity entity) const
		{
			const auto componentId = Component<T>::GetId();
			const auto entityId = entity.GetId();
			return m_EntityComponentSignature[entityId].test(componentId);
		}

		template<typename T>
		T& GetComponent(Entity entity) const
		{
			const auto componentId = Component<T>::GetId();
			const auto entityId = entity.GetId();
			auto* componentPool = static_cast<ComponentPool<T>*>(m_ComponentPools[componentId].get());
			return componentPool->Get(entityId);
		}

		// System Management
		template <typename T, typename ...TArgs>
		void AddSystem(TArgs&& ...args)
		{
			// Creating the new System and forwarding the arguments correctly
			// and then move it to our System data structure by making a pair
			std::unique_ptr<T> newSystem = std::make_unique<T>(std::forward<TArgs>(args)...);
			m_Systems.insert(std::make_pair( std::type_index(typeid(T)), std::move(newSystem)));
		}

		template <typename T>
		void RemoveSystem()
		{
			m_Systems.erase(std::type_index(typeid(T)));
		}

		template <typename T>
		bool HasSystem() const
		{
			return m_Systems.find(std::type_index(typeid(T))) != m_Systems.end();
		}

		template <typename T>
		T& GetSystem() const
		{
			auto it = m_Systems.find(std::type_index(typeid(T)));
			assert(it != m_Systems.end() && "System does not exist!");
			return *static_cast<T*>(it->second.get());
		}


		// Checks the component signature of an entity and add
		// the entity to the systems that are interested in it
		void AddEntityToSystems(Entity entity);

		// Generic Help functions ---------------> Couples though...
		void SetPrimaryCamera(Entity entity);
		Entity GetPrimaryCamera() const;

	};
}