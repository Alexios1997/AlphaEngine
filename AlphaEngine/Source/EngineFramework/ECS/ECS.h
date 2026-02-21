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
#include <deque>
#include <cstdint>

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

		// the index of the vector is the Entity ID
		// The value at that index is the Position in the m_ENtities array
		std::vector<int> m_EntityToIndex;

	public:
		System() { m_Entities.reserve(10000);  m_EntityToIndex.resize(10000, -1); };
		~System() = default;

		void AddEntityToSystem(Entity entity);
		void RemoveEntityFromSystem(Entity entity);
		bool HasEntity(Entity entity) const;
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
		virtual ~IComponentPool() = default;
		virtual void RemoveEntityFromPool(uint16_t entityId) = 0;
	};

	// Pool -> just a vector (contiguous data) of objects of type T
	template <typename T>
	class ComponentPool : public IComponentPool
	{
	private:

		// Problem: 
		// Index |   0			1			....			500				....			9999
		// Data  |  [Health] [Health]		empty		 [Health]		   [empty]		  [Health]
		// We do Dense - Sparse arrays because When our HealthSystem wants to update everyone, 
		// the CPU has to "jump" from address 0 to address 500, then to address 999. This is a Cache Miss
		// With Sparse set way:
		//
		// <------ The dense array (actualy comp data lives here) -------->
		// Dense Index |	   0			1			2			
		// Data		   |  [Health A] 	[Health B] 	[Health C]
		//
		// 
		// <------ The Sparse array (where in the dense array to find that entity's data) -------->
		// Entity ID	 |	   0			....			500				....			999			
		// Value		 |     0			-1				1				-1				2

		// The "Dense" array: Tightly packed component data for CPU cache speed
		std::vector<T> m_Data;

		// Maps Dense Index -> Entity ID (Needed for Swap-and-Pop)
		std::vector<uint16_t> m_DenseToEntity;

		// The "Sparse" array: Maps Entity ID -> Index in m_Data
		// We use int because we need -1 to indicate "no component"
		std::vector<int> m_EntityToIndex;

	public:
		ComponentPool(uint16_t size = 1000) { m_Data.reserve(size); m_DenseToEntity.reserve(size); }
		virtual ~ComponentPool() = default;

		bool isEmpty() const { return m_Data.empty(); }
		uint16_t GetSize() const { return m_Data.size(); }
		void Clear() { m_Data.clear(); }

		void AddComp(uint16_t entityId, T Object)
		{
			// Grow Sparse array if needed
			if (entityId >= m_EntityToIndex.size()) {
				m_EntityToIndex.resize(entityId + 1, -1);
			}

			// Map entity to the end of the dense array
			m_EntityToIndex[entityId] = static_cast<int>(m_Data.size());

			// Push data to the end
			m_Data.push_back(std::move(Object));
			m_DenseToEntity.push_back(entityId);
		}

		void RemoveComp(uint16_t entityId)
		{
			int indexToRemove = m_EntityToIndex[entityId];
			int lastIndex = static_cast<int>(m_Data.size()) - 1;

			// Move last element to the hole
			m_Data[indexToRemove] = std::move(m_Data[lastIndex]);

			// Update mapings
			uint16_t entityOfLastElement = m_DenseToEntity[lastIndex];
			m_EntityToIndex[entityOfLastElement] = indexToRemove;
			m_DenseToEntity[indexToRemove] = entityOfLastElement;

			// Cleanup
			m_Data.pop_back();
			m_DenseToEntity.pop_back();
			m_EntityToIndex[entityId] = -1;
		}

		void RemoveEntityFromPool(uint16_t entityId) override {
			// Check if the entity is within range of our sparse array
			// Check if the value is not -1 (meaning it actually has a component)
			if (entityId < m_EntityToIndex.size() && m_EntityToIndex[entityId] != -1) {
				RemoveComp(entityId); // This calls your Swap-and-Pop logic
			}
		}
		T& Get(uint16_t entityId) {
			assert(entityId < m_EntityToIndex.size() && m_EntityToIndex[entityId] != -1);


			int index = m_EntityToIndex[entityId];


			return m_Data[index];
		}

		uint16_t GetCount() const {
			return static_cast<uint16_t>(m_Data.size());
		}

		std::vector<T>& GetAllData() {
			return m_Data;
		}
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
		// std::set is a tree, which means that every time we call create entity,
		// the cpu has to allocate a new node and rebalance the tree.
		// It would be better to use std::vector
		std::vector<Entity> m_EntitiesToBeAdded;
		std::vector<Entity> m_EntitiesToBeDestroyed;
		// Using this data structure so we add here the entities that need to 
		// be refreshed and not Refresing every time But only once when we have all entities (Deffered
		std::vector<Entity> m_EntitiesToRefresh;

		// List of free entity ids that were prev removed
		std::vector<uint16_t> m_FreeIDs;

		// Primary Camera ---------------> This creates Couple with!
		Entity m_PrimaryCamera;

	public:
		ECSOrchestrator() { Logger::Log("Created The Orchestrator"); 
		m_EntitiesToBeAdded.reserve(1000);
		m_EntitiesToBeDestroyed.reserve(1000);
		m_EntitiesToRefresh.reserve(1000);
		m_EntityComponentSignature.reserve(10000);
		};

		virtual ~ECSOrchestrator() { Logger::Log("Destroyed The Orchestrator"); };
		virtual void InitService() override { Logger::Log("Initializing Service named : ECS Orchestrator"); };

		// Update processes the entities that are waiting to be added/destroyed
		void UpdateEntitiesLifeTime();

		// Entity Management
		Entity CreateEntity();
		void DestroyEntity(Entity entity);

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


			T newComponent(std::forward<TArgs>(args)...);

			currentCompPool->AddComp(entityId, std::move(newComponent));

			m_EntityComponentSignature[entityId].set(componentId);
			m_EntitiesToRefresh.push_back(entity);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			const auto componentId = Component<T>::GetId();
			const auto entityId = entity.GetId();

			if (componentId < m_ComponentPools.size() && m_ComponentPools[componentId]) {
				auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[componentId].get());
				pool->RemoveComp(entityId);
			}

			m_EntityComponentSignature[entityId].set(componentId, false);
			m_EntitiesToRefresh.push_back(entity);
		}

		template<typename T>
		bool HasComponent(Entity entity) const
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
			m_Systems.insert(std::make_pair(std::type_index(typeid(T)), std::move(newSystem)));
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
		// Also Remove them
		void AddEntityToSystems(Entity entity);
		void RemoveEntityFromSystems(Entity entity);
		void RefreshEntity(Entity entity);

		// Generic Help functions ---------------> Couples though...
		void SetPrimaryCamera(Entity entity);
		Entity GetPrimaryCamera() const;

	};
}