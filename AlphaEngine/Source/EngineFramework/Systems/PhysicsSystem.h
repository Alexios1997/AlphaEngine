#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h> 
#include <Jolt/Physics/Collision/Shape/BoxShape.h> 
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/GroupFilter.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterMask.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>

#include "EngineFramework/Mesh.h"
#include "EngineFramework/PhysicsLayers.h"
#include "EngineFramework/ECS/ECS.h"
#include "EngineFrameWork/Components/RigidBodyComponent.h"
#include "EngineFrameWork/Components/RendererComponent.h"
#include "EngineFrameWork/Components/TransformComponent.h"
#include "EngineFramework/MainContactListener.h"
#include <glm/gtc/quaternion.hpp>
#include <iostream>

namespace AlphaEngine
{

	struct PendingMeshRequest {
		Entity entity;
		AssetHandler handle;
		glm::vec3 pos;
		glm::vec3 scale;
	};

	class PhysicsSystem : public System
	{
	private:
		std::vector<PendingMeshRequest> m_PendingMeshRequests;

	public:

		float accumulator = 0.0f;
		const float fixedDeltaTime = 1.0f / 60.0f;

		JPH::PhysicsSystem* jolt_PhysicsSystem;
		JPH::TempAllocatorImpl* jolt_TempAllocator;
		JPH::JobSystemThreadPool* jolt_JobSystem;
		MainContactListener* jolt_MainContactListener;

		// Interfaces instances
		PhysBPLayerInterfaceImpl m_BPLayerInterface;
		PhysObjectLayerPairFilterImpl m_ObjectLayerPairFilter;
		ObjectVsBroadPhaseLayerFilterImpl m_ObjectVsBPFilter;
		

		PhysicsSystem() 
		{
			RequireComponent<RigidBodyComponent>();
			RequireComponent<TransformComponent>();

			// Jolt requires a temp allocator for per-frame memory (mostly for optimization)
			jolt_TempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);

			// Multi-threading: Jolt uses all available cores
			jolt_JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);			
			
			// Initializing our Physics Settings
			JPH::PhysicsSettings settings;
			settings.mBaumgarte = 0.1f;
			settings.mPenetrationSlop = 0.005f;
			settings.mMinVelocityForRestitution = 1.0f;
			settings.mPointVelocitySleepThreshold = 0.05f;
			settings.mTimeBeforeSleep = 0.5f;
			settings.mAllowSleeping = true;

			jolt_PhysicsSystem = new JPH::PhysicsSystem();
			jolt_PhysicsSystem->Init(1024, 0, 1024, 1024, m_BPLayerInterface, m_ObjectVsBPFilter, m_ObjectLayerPairFilter);
			jolt_PhysicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
			jolt_PhysicsSystem->SetPhysicsSettings(settings);

			// Create our Contantc Listner
			jolt_MainContactListener = new MainContactListener(jolt_PhysicsSystem);
			jolt_PhysicsSystem->SetContactListener(jolt_MainContactListener);

		}

		// Requesting Mesh handler to create a mesh shape
		void RequestMeshBody(Entity entity, AssetHandler handle, glm::vec3 pos, glm::vec3 scale)
		{
			m_PendingMeshRequests.push_back({ entity, handle, pos, scale });
		}


		// Create Mesh Body Shape
		JPH::BodyID CreateMeshBody(Entity entity, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, glm::vec3 pos, glm::vec3 scale)
		{

			if (vertices.empty()) {
				Logger::Err("[PhysicsSystem]: No Vertices found to create Mesh Body");
				return JPH::BodyID(); 
			}
			if (indices.empty()) {
				Logger::Err("[PhysicsSystem]: No Indices found to create Mesh Body"); 
				return JPH::BodyID();
			}

			// Getting the Body Interface
			JPH::BodyInterface& bodyInterface = jolt_PhysicsSystem->GetBodyInterface();

			// Reserving memory first to avoid reallocations
			// And then pushing our vertices (through ref obviously) to jolt List (they use their own maths)
			JPH::VertexList joltVertices;
			joltVertices.reserve(vertices.size());
			for (const auto& v : vertices) {
				joltVertices.push_back(JPH::Float3(v.Position.x, v.Position.y, v.Position.z));
			}

			// Here pushing our indices
			JPH::IndexedTriangleList joltTriangles;
			joltTriangles.reserve(indices.size() / 3);
			for (size_t i = 0; i + 2 < indices.size(); i += 3) {
				joltTriangles.push_back(JPH::IndexedTriangle(indices[i], indices[i + 1], indices[i + 2]));
			}

			// We create our Mesh Shame settings here
			// Sanitize -> It removfes triangles with 0 area that would otherwise crash the engine
			JPH::MeshShapeSettings shapeSettings(joltVertices, joltTriangles);
			shapeSettings.Sanitize();

			JPH::Shape::ShapeResult shapeResult = shapeSettings.Create();
			if (shapeResult.HasError()) {
				Logger::Err("[PhysicsSystem]: Problem with the shape settings");
				return JPH::BodyID();
			}

			// Gertting the shape result to ref our shape
			JPH::Ref<JPH::Shape> meshShape = shapeResult.Get();

			// Jolt automatically centers shapes. If our modlel origin is at the corner, Jolt will move it to the center
			// So by using OffsetCenter mass we trick Jolt into keeping the origin where the 3d artist placed it.
			JPH::OffsetCenterOfMassShapeSettings comSettings(JPH::Vec3::sZero(), meshShape);
			meshShape = comSettings.Create().Get();

			// Applying the given scale as well
			if (scale != glm::vec3(1.0f)) {
				JPH::ScaledShapeSettings scaledSettings(meshShape, JPH::Vec3(scale.x, scale.y, scale.z));
				meshShape = scaledSettings.Create().Get();
			}

			// Actual creation of the body that will be handled for physics
			JPH::BodyCreationSettings settings(
				meshShape,
				JPH::RVec3(pos.x, pos.y, pos.z),
				JPH::Quat::sIdentity(),
				JPH::EMotionType::Static,
				PhysObjectLayers::NON_MOVING
			);

			settings.mUserData = static_cast<uint64_t>(entity.GetId());
			settings.mFriction = 0.5f;

			return bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
		}

		// Creating a sphere Body
		JPH::BodyID CreateSphereBody(Entity entity, glm::vec3 pos, float radius, bool isStatic)
		{
			JPH::BodyInterface& bodyInterface = jolt_PhysicsSystem->GetBodyInterface();

			JPH::SphereShapeSettings settings(radius);
			JPH::ShapeRefC shape = settings.Create().Get();

			// Setting the motion and layer
			JPH::EMotionType currentMotionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
			JPH::ObjectLayer currentLayer = isStatic ? PhysObjectLayers::NON_MOVING : PhysObjectLayers::MOVING;

			JPH::BodyCreationSettings bodySettings(shape, JPH::RVec3(pos.x, pos.y, pos.z), JPH::Quat::sIdentity(), currentMotionType, currentLayer);
			bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
			bodySettings.mFriction = 0.5f;
			bodySettings.mLinearDamping = 0.2f; 
			bodySettings.mAngularDamping = 0.2f;
			
			bodySettings.mMassPropertiesOverride.mMass = 1.0f;
			bodySettings.mUserData = static_cast<uint64_t>(entity.GetId());

			if (!isStatic) bodySettings.mRestitution = 0.7f;

			return bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);
		}

		JPH::BodyID CreateBoxBody(Entity entity, glm::vec3 pos, glm::vec3 halfExtents, bool isStatic, float restitution = 0.0f)
		{
			JPH::BodyInterface& bodyInterface = jolt_PhysicsSystem->GetBodyInterface();

			// Jolt uses "Half Extents" (distance from center to edge)
			JPH::BoxShapeSettings settings(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
			JPH::ShapeRefC shape = settings.Create().Get();

			JPH::EMotionType motionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
			JPH::ObjectLayer layer = isStatic ? PhysObjectLayers::NON_MOVING : PhysObjectLayers::MOVING;

			JPH::RVec3 joltPos(pos.x, pos.y, pos.z);
			JPH::BodyCreationSettings bodySettings(shape, joltPos, JPH::Quat::sIdentity(), motionType, layer);
			bodySettings.mFriction = 0.2f;
			bodySettings.mRestitution = restitution;
			bodySettings.mUserData = static_cast<uint64_t>(entity.GetId());
			// Create and Add to world
			// Static objects don't need to be "Activated" because they never move
			JPH::EActivation activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

			return bodyInterface.CreateAndAddBody(bodySettings, activation);
		}

		// Box used for Sensoring things (Trigger Areas)
		JPH::BodyID CreateSensorBox(Entity entity, glm::vec3 pos, glm::vec3 halfExtents) 
		{

			JPH::BodyInterface& bodyInterface = jolt_PhysicsSystem->GetBodyInterface();

			JPH::BoxShapeSettings shapeSettings(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
			JPH::ShapeRefC shape = shapeSettings.Create().Get();

			// Set mIsSensor to true
			JPH::BodyCreationSettings settings(shape, JPH::RVec3(pos.x, pos.y, pos.z), JPH::Quat::sIdentity(), JPH::EMotionType::Static, PhysObjectLayers::NON_MOVING);
			settings.mIsSensor = true;
			settings.mUserData = static_cast<uint64_t>(entity.GetId());

			return bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
		}

		JPH::BodyID CastRay(glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance = 100.0f) 
		{
			// Prepare Ray
			JPH::RVec3 from(rayOrigin.x, rayOrigin.y, rayOrigin.z);
			JPH::Vec3 direction(rayDir.x * maxDistance, rayDir.y * maxDistance, rayDir.z * maxDistance);
			JPH::RRayCast ray(from, direction);

			// Prepare Settings
			JPH::RayCastSettings settings;
			JPH::ClosestHitCollisionCollector<JPH::CastRayCollector> collector;

			// Required filters
			JPH::BroadPhaseLayerFilter bp_filter;
			JPH::ObjectLayerFilter obj_filter;
			JPH::BodyFilter body_filter;
			JPH::ShapeFilter shape_filter;
			
			// Ignoring Filter
			JPH::BodyInterface& bi = jolt_PhysicsSystem->GetBodyInterface();
			IgnoreSensorsFilter sensorFilter(bi);

			// Jolt Call Cast Ray
			jolt_PhysicsSystem->GetNarrowPhaseQuery().CastRay(
				ray,
				settings,
				collector,
				bp_filter,
				obj_filter,
				sensorFilter,
				shape_filter
			);

			return collector.HadHit() ? collector.mHit.mBodyID : JPH::BodyID();
		}

		void RunSystem(ECSOrchestrator& ecs, float deltaTime) {

			//  Cap deltaTime so we wont have to catch up perfectly and create more lagging
			float frameTime = std::min(deltaTime, 0.25f);
			accumulator += frameTime;

			// Step the physics world
			while (accumulator >= fixedDeltaTime) {
				jolt_PhysicsSystem->Update(fixedDeltaTime, 1, jolt_TempAllocator, jolt_JobSystem);
				accumulator -= fixedDeltaTime;
			}


			auto& assetManager = ServiceLocator::Get<AssetManager>();

			// Check if requested meshes are loaded so we can create the collision mesh bodies
			for (auto it = m_PendingMeshRequests.begin(); it != m_PendingMeshRequests.end(); ) {
				
				if (assetManager.IsMeshLoaded(it->handle)) {

					// The data is ready! Build the Jolt body now.
					JPH::BodyID currentBodyID = CreateMeshBody(
						it->entity,
						assetManager.GetMeshAllVertices(it->handle),
						assetManager.GetMeshAllIndices(it->handle),
						it->pos,
						it->scale
					);

					if (!currentBodyID.IsInvalid()) {
						// Get the component we added earlier and update the ID
						if (ecs.HasComponent<RigidBodyComponent>(it->entity)) {
							auto& rb = ecs.GetComponent<RigidBodyComponent>(it->entity);
							rb.bodyID = currentBodyID;

							std::cout << "[Physics] Linked Jolt Body " << currentBodyID.GetIndex() << " to Entity " << it->entity.GetId() << std::endl;
						}
					}

					it = m_PendingMeshRequests.erase(it);
				}
				// Still loading
				else {
					++it; 
				}
			}


			// This checks the contacts happened and that are in the event queue of our contract listener
			// , thats why we publish the event as well
			{
				std::lock_guard<std::mutex> lock(jolt_MainContactListener->m_QueueMutex);
				JPH::BodyInterface& bi = jolt_PhysicsSystem->GetBodyInterface();

				for (auto& contact : jolt_MainContactListener->m_EventQueue) {
					// Safe lookup on Main Thread
					Entity e1 = static_cast<Entity>(bi.GetUserData(contact.id1));
					Entity e2 = static_cast<Entity>(bi.GetUserData(contact.id2));

					// If it's a 'Removed' event, we might need to check if the body still exists
					bool isSensor = contact.isSensor;
					if (!contact.isStart) {
						// Determine if it was a sensor before it vanished
						isSensor = bi.IsSensor(contact.id1) || bi.IsSensor(contact.id2);
					}

					CollisionEvent event(e1, e2, contact.isStart, isSensor);
					EventBus::Publish(event);
				}
				jolt_MainContactListener->m_EventQueue.clear();
			}

			// Sync Jolt results back to ECS Transforms
			JPH::BodyInterface& bodyInterface = jolt_PhysicsSystem->GetBodyInterface();

			for (auto const& entity : GetSystemEntities()) {
				auto& rb = ecs.GetComponent<RigidBodyComponent>(entity);
				auto& transform = ecs.GetComponent<TransformComponent>(entity);


				// Fetch the simulated position/rotation from Jolt
				JPH::RVec3 pos;
				JPH::Quat rot;
				bodyInterface.GetPositionAndRotation(rb.bodyID, pos, rot);

				// Copy to Transform so the RenderSystem can see it
				transform.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
				transform.rotation = glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
				JPH::Vec3 vel = bodyInterface.GetLinearVelocity(rb.bodyID);
				JPH::Vec3 angVel = bodyInterface.GetAngularVelocity(rb.bodyID);
			}
		}

	};
}
