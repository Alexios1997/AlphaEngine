
#include <glad/gl.h> 
#include "AppLayer.h"
#include "EngineFramework/Renderer/IRenderer.h"
#include "EngineFramework/Logger.h"
#include "EngineFramework/Components/TransformComponent.h"
#include "EngineFramework/Components/RendererComponent.h"
#include "EngineFramework/Components/CameraComponent.h"
#include "EngineFramework/Components/RigidBodyComponent.h"
#include "EngineFramework/Systems/CameraSystem.h"
#include "EngineFramework/Systems/RenderSystem.h"
#include "EngineFramework/Systems/PlayerControllerSystem.h"
#include "EngineFramework/Systems/MovementSystem.h"
#include "EngineFramework/Systems/PhysicsSystem.h"
#include "EngineFramework/Utility.h"
#include <iostream>
#include <print>
#include "EngineFramework/ServiceLocator.h"
#include "EngineFramework/FileSystem.h"
#include "EngineFrameWork/Mesh.h"
#include <random>



namespace AlphaEngine
{

	// TESTING ! 
	static std::vector<Entity> monkeyEntities; 
	static float destructionTimer = 0.0f;
	static float respawnTimer = 0.0f;

	AppLayer::AppLayer() : Layer("AppLayer"), monkeyA{ 0 }, monkeyB{ 1 }
	{
	}

	AppLayer::~AppLayer() {}

	// Initialize OpenGL buffers (VAO, VBO, Shaders) here
	void AppLayer::OnAttach()
	{

		


		Layer::OnAttach();

		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();

		// Add Systems that are needed!



		ecsOrchestrator.AddSystem<RenderSystem>();
		ecsOrchestrator.AddSystem<CameraSystem>();
		ecsOrchestrator.AddSystem<PlayerControllerSystem>();
		ecsOrchestrator.AddSystem<MovementSystem>();
		ecsOrchestrator.AddSystem<PhysicsSystem>();



		std::cout << "[AppLayer] Attaching:\n";


		// <--------------------- Load Assets Async ---------------------------->
		AssetHandler monkeyModelHandle = ServiceLocator::Get<AssetManager>().LoadMesh("AlphaGame/Assets/Models/monkey.glb", false);
		AssetHandler basicShaderHandle = ServiceLocator::Get<AssetManager>().LoadShader("AlphaGame/Shaders/Basic_Instancing.glsl");
		AssetHandler basicTextureHandle = ServiceLocator::Get<AssetManager>().LoadTexture("AlphaGame/Assets/Textures/coloredChecker.png");
		AssetHandler fresnelShaderHandle = ServiceLocator::Get<AssetManager>().LoadShader("AlphaGame/Shaders/Fresnel.glsl");
		AssetHandler sphereModelHandle = ServiceLocator::Get<AssetManager>().LoadMesh("AlphaGame/Assets/Models/shpereBall.glb", false);
		AssetHandler floorModelHandle = ServiceLocator::Get<AssetManager>().LoadMesh("AlphaGame/Assets/Models/floor.glb", false);

		// Skybox Textures - Shaders
		std::vector<std::string> currentCubeMapFaces =
		{
		"AlphaGame/Assets/Textures/Skybox_px.png",
		"AlphaGame/Assets/Textures/Skybox_nx.png",
		"AlphaGame/Assets/Textures/Skybox_py.png",
		"AlphaGame/Assets/Textures/Skybox_ny.png",
		"AlphaGame/Assets/Textures/Skybox_pz.png",
		"AlphaGame/Assets/Textures/Skybox_nz.png"
		};

		AssetHandler cubeBoxHandle = ServiceLocator::Get<AssetManager>().LoadMesh("AlphaGame/Assets/Models/CubeBox.glb", false);
		AssetHandler skyCubeMapHandle = ServiceLocator::Get<AssetManager>().LoadCubeMap(currentCubeMapFaces);
		AssetHandler skyboxShaderHandle = ServiceLocator::Get<AssetManager>().LoadShader("AlphaGame/Shaders/Skybox.glsl");


		// To prevet looking like inside-out
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);



		// In physics we half extent everything for perfomance If we dont then we use min/max -> 2 operations
		// while half extent (abs(x-center) < halfExtent -> 1 operation
		// Position it at y = -1.0 so the "top" of the 2m thick box is at y = 0.0
		glm::vec3 floorPos(0.f, -10.f, -10.f);
		glm::vec3 floorHalfExtents(100.f, 1.f, 100.f); // 100m x 2m x 100m total size

		float sphereRadius = 0.2f;

		glm::vec3 spherePosA(0.2f, 10.f, -3.f);
		
		glm::vec3 spherePosB(0.f, 15.f, -3.f);

		// TODO : IT LOOKS weird like the floor is above or sth
		// Could be the perspective thing or w/e
		glm::vec3 spherePosC(2.f, 10.f, -3.f);

		glm::vec3 spherePosD(-2.f, 15.f, -3.f);


		// Stress test

		
		float spacing = 0.5f;
		int count = 20;

		for (int i = 0; i < count; ++i) {
			Entity ball = ecsOrchestrator.CreateEntity();

			
			float offsetX = (i % 5) * 0.1f;
			float offsetZ = (i / 10) * 0.1f;
			glm::vec3 pos(offsetX, 5.0f + (i * 1.5f), -3.0f + offsetZ);

			
			JPH::BodyID bodyID = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateSphereBody(ball, pos, sphereRadius, false);

			
			ecsOrchestrator.AddComponent<TransformComponent>(ball, TransformComponent(pos, glm::vec3(sphereRadius)));
			ecsOrchestrator.AddComponent<RenderComponent>(ball,
				RenderComponent(sphereModelHandle, basicShaderHandle, basicTextureHandle));
			ecsOrchestrator.AddComponent<RigidBodyComponent>(ball, RigidBodyComponent{ bodyID });
		}
		

		// Creating Entities just for testing

		monkeyA = ecsOrchestrator.CreateEntity();
		monkeyB = ecsOrchestrator.CreateEntity();
		sphereA = ecsOrchestrator.CreateEntity();
		//sphereB = ecsOrchestrator.CreateEntity();
		//sphereC = ecsOrchestrator.CreateEntity();
		//sphereD = ecsOrchestrator.CreateEntity();
		floor = ecsOrchestrator.CreateEntity();
		

		auto cameraEntity = ecsOrchestrator.CreateEntity();
		auto skyboxEntity = ecsOrchestrator.CreateEntity();


		// <-------- Testing purposes --------->



		// MiniGolfModel
		
		MiniGolfModel = ecsOrchestrator.CreateEntity();
		AssetHandler golfModelHandle = ServiceLocator::Get<AssetManager>().LoadMesh("AlphaGame/Assets/Models/TestingGolfComplexModel.glb", true);
		
		// Where should i Put it
		glm::vec3 golfModelPos(0.f, -6.f, -1.f);
		glm::vec3 golfModelScale(5.0f, 5.0f, 5.0f);

		ecsOrchestrator.GetSystem<PhysicsSystem>().RequestMeshBody(
			MiniGolfModel,
			golfModelHandle,
			golfModelPos,
			golfModelScale
		);

		ecsOrchestrator.AddComponent<TransformComponent>(MiniGolfModel,
			TransformComponent(golfModelPos, golfModelScale));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(MiniGolfModel,
			RigidBodyComponent(JPH::BodyID()));

		ecsOrchestrator.AddComponent<RenderComponent>(MiniGolfModel,
			RenderComponent(golfModelHandle, basicShaderHandle, basicTextureHandle));

		// Goal Zone

		/*
		goalZone = ecsOrchestrator.CreateEntity();
		glm::vec3 zonePos(0.0f, -8.0f, -10.f);
		// why half extents?
		glm::vec3 zoneHalfExtents(100.f, 1.0f, 100.f);
		JPH::BodyID sensorID = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateSensorBox(goalZone, zonePos, zoneHalfExtents);

		ecsOrchestrator.AddComponent<TransformComponent>(floor,
			TransformComponent(zonePos, glm::vec3(1.f)));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(floor,
			RigidBodyComponent(sensorID));
		
		*/

		// Spheres

		JPH::BodyID sphereBodyIDA = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateSphereBody(sphereA, spherePosA, sphereRadius, false);
		//JPH::BodyID floorBodyID = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateBoxBody(floor,floorPos, floorHalfExtents, true);
		//JPH::BodyID sphereBodyIDB = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateSphereBody(sphereB, spherePosB, sphereRadius, false);
		//JPH::BodyID sphereBodyIDC = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateSphereBody(sphereC, spherePosC, sphereRadius, false);
		//JPH::BodyID sphereBodyIDD = ecsOrchestrator.GetSystem<PhysicsSystem>().CreateSphereBody(sphereD, spherePosD, sphereRadius, false);


		/*
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(-2.0f, 2.0f);
		glm::vec3 centerPoint = glm::vec3(-0.5f, 0.0f, 0.0f);

		for (int i = 0; i < 979; i++)
		{
			Entity e = ecsOrchestrator.CreateEntity();

			glm::vec3 randomOffset = glm::vec3(dis(gen), dis(gen), dis(gen));
			glm::vec3 finalPos = centerPoint + randomOffset;

			// 3. Add components with the clustered position
			ecsOrchestrator.AddComponent<TransformComponent>(e,
				TransformComponent(finalPos, glm::vec3(0.1f)));

			ecsOrchestrator.AddComponent<RenderComponent>(e,
				RenderComponent(monkeyModelHandle, basicShaderHandle, monkeyTextureHandle));

			ecsOrchestrator.AddComponent<VelocityComponent>(e, VelocityComponent());

			monkeyEntities.push_back(e);
		}
		*/


		// Skybox Entity
		RenderComponent skyRender(cubeBoxHandle, skyboxShaderHandle, skyCubeMapHandle);
		skyRender.isSkybox = true;
		skyRender.layerID = 0;
		
		ecsOrchestrator.AddComponent<RenderComponent>(skyboxEntity, skyRender);

		ecsOrchestrator.AddComponent<TransformComponent>(skyboxEntity,
			TransformComponent(glm::vec3(0.3f), glm::vec3(0.5f)));

		// First Entity Monkey
		/*
		ecsOrchestrator.AddComponent<TransformComponent>(monkeyA,
			TransformComponent(glm::vec3(-1.8f, 0.0f, 0.0f), glm::vec3(0.5f)));

		ecsOrchestrator.AddComponent<RenderComponent>(monkeyA,
			RenderComponent(monkeyModelHandle, basicShaderHandle, basicTextureHandle));

		ecsOrchestrator.AddComponent<PlayerControllerComponent>(monkeyA,
			PlayerControllerComponent(5.f, 2.f));

		ecsOrchestrator.AddComponent<VelocityComponent>(monkeyA,
			VelocityComponent());
		

		// Second Monkey

		ecsOrchestrator.AddComponent<TransformComponent>(monkeyB,
			TransformComponent(glm::vec3(1.8f, 0.0f, 0.0f), glm::vec3(0.5f)));

		ecsOrchestrator.AddComponent<RenderComponent>(monkeyB,
			RenderComponent(monkeyModelHandle, basicShaderHandle, basicTextureHandle));
*/
		// Sphere A

		ecsOrchestrator.AddComponent<TransformComponent>(sphereA,
			TransformComponent(spherePosA, glm::vec3(sphereRadius)));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(sphereA,
			RigidBodyComponent(sphereBodyIDA));

		ecsOrchestrator.AddComponent<RenderComponent>(sphereA, 
			RenderComponent(sphereModelHandle, basicShaderHandle, basicTextureHandle));

		// Sphere B
		/*
		ecsOrchestrator.AddComponent<TransformComponent>(sphereB,
			TransformComponent(spherePosB, glm::vec3(sphereRadius)));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(sphereB,
			RigidBodyComponent(sphereBodyIDB));

		ecsOrchestrator.AddComponent<RenderComponent>(sphereB,
			RenderComponent(sphereModelHandle, basicShaderHandle, basicTextureHandle));

		// Sphere C

		ecsOrchestrator.AddComponent<TransformComponent>(sphereC,
			TransformComponent(spherePosC, glm::vec3(sphereRadius)));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(sphereC,
			RigidBodyComponent(sphereBodyIDC));

		ecsOrchestrator.AddComponent<RenderComponent>(sphereC,
			RenderComponent(sphereModelHandle, basicShaderHandle, basicTextureHandle));

		// Sphere D

		ecsOrchestrator.AddComponent<TransformComponent>(sphereD,
			TransformComponent(spherePosD, glm::vec3(sphereRadius)));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(sphereD,
			RigidBodyComponent(sphereBodyIDD));

		ecsOrchestrator.AddComponent<RenderComponent>(sphereD,
			RenderComponent(sphereModelHandle, basicShaderHandle, basicTextureHandle));
			*/
		// Floor
		/*
		ecsOrchestrator.AddComponent<TransformComponent>(floor,
			TransformComponent(floorPos, glm::vec3(1.f, 1.f,2.f)));

		ecsOrchestrator.AddComponent<RigidBodyComponent>(floor,
			RigidBodyComponent(floorBodyID));

		ecsOrchestrator.AddComponent<RenderComponent>(floor);
		*/
		// Camera Entity

		ecsOrchestrator.AddComponent<TransformComponent>(cameraEntity,
			TransformComponent(glm::vec3(0.f, 0.f, 40.f), glm::vec3(1.f))
		);

		ecsOrchestrator.AddComponent<CameraComponent>(cameraEntity,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		ecsOrchestrator.SetPrimaryCamera(cameraEntity);



		AlphaEngine::EventBus::Subscribe([this](AlphaEngine::Event& e) {
			AlphaEngine::EventDispatcher dispatcher(e);
			dispatcher.Dispatch<AlphaEngine::CollisionEvent>([this](AlphaEngine::CollisionEvent& ev) {

				if (ev.IsSensor()) {

					uint32_t goalID = goalZone.GetId();

					if (ev.GetEntityA().GetId() == goalID || ev.GetEntityB().GetId() == goalID) {
						this->RespawnBall(ev.GetEntityA().GetId() == goalID ? ev.GetEntityB() : ev.GetEntityA());
					}
				}
				return true;
				});
			});


		//ecsOrchestrator.DestroyEntity(en2);

	}

	void AppLayer::OnDetach() {

		Layer::OnDetach();
		std::cout << "[AppLayer] Detaching";

	}

	// Update physics, player movement, etc.
	void AppLayer::OnUpdate(float deltaTime) {

		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();

		



		// Add current frame's time to our accumulator
		m_FPSAccumulator += deltaTime;
		m_FrameCounter++;

		// If 1 second has passed, display and reset
		if (m_FPSAccumulator >= 1.0f)
		{
			float avgDeltaTime = (m_FPSAccumulator / m_FrameCounter) * 1000.0f; // ms
			uint32_t fps = m_FrameCounter;

			// Print to console
			std::cout << "[Performance] FPS: " << fps
				<< " | Avg Delta: " << avgDeltaTime << "ms" << std::endl;

			// Reset for the next second
			m_FPSAccumulator = 0.0f;
			m_FrameCounter = 0;
		}
		
		
		auto& input = ServiceLocator::Get<Input>();

		ecsOrchestrator.GetSystem<CameraSystem>().RunSystem(ecsOrchestrator);
		ecsOrchestrator.GetSystem<PlayerControllerSystem>().RunSystem(ecsOrchestrator, input);
		//ecsOrchestrator.GetSystem<MovementSystem>().RunSystem(ecsOrchestrator, deltaTime);
		ecsOrchestrator.GetSystem<PhysicsSystem>().RunSystem(ecsOrchestrator, deltaTime);

	}

	// Draw objects/world
	void AppLayer::OnRender()
	{
		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();
		auto& currentRenderer = ServiceLocator::Get<IRenderer>();

		ecsOrchestrator.GetSystem<RenderSystem>().RunSystem(currentRenderer, ecsOrchestrator);

	}

	void AppLayer::OnEvent(AlphaEngine::Event& event)
	{
		AlphaEngine::EventDispatcher dispatcher(event);

		dispatcher.Dispatch<AlphaEngine::MouseButtonPressedEvent>([this](AlphaEngine::MouseButtonPressedEvent& e) {
			// If Left Mouse Button (usually 0 or GLFW_MOUSE_BUTTON_LEFT)
			if (e.GetMouseButton() == 0) {
				this->HandleMouseClick();
			}
			return true;
			});

		dispatcher.Dispatch<AlphaEngine::MouseMovedEvent>([this](AlphaEngine::MouseMovedEvent& e) { return OnMouseMoved(e); });
		dispatcher.Dispatch<AlphaEngine::WindowClosedEvent>([this](AlphaEngine::WindowClosedEvent& e) { return OnWindowClosed(e); });
		dispatcher.Dispatch<AlphaEngine::WindowResizeEvent>([this](AlphaEngine::WindowResizeEvent& e) { return OnResize(e); });
	}

	void AppLayer::RespawnBall(Entity entity)
	{
		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();

		auto& physicsSystem = ecsOrchestrator.GetSystem<PhysicsSystem>();
		auto& rb = ecsOrchestrator.GetComponent<RigidBodyComponent>(entity);
		JPH::BodyInterface& bodyInterface = physicsSystem.jolt_PhysicsSystem->GetBodyInterface();

		// Generate a random height/offset for variety
		float randomX = (std::rand() % 10 - 5) * 0.5f; // Random -2.5 to 2.5
		float randomY = 15.0f + (std::rand() % 10);   // Random 15.0 to 25.0
		JPH::RVec3 newPos(randomX, randomY, -3.0f);

		// Set position and roation to the teleported entity through using the body id of the given rigidbody's entity
		// + the body interface to access the funcs we want
		bodyInterface.SetPositionAndRotation(rb.bodyID, newPos, JPH::Quat::sIdentity(), JPH::EActivation::Activate);
		bodyInterface.SetLinearVelocity(rb.bodyID, JPH::Vec3::sZero());
		bodyInterface.SetAngularVelocity(rb.bodyID, JPH::Vec3::sZero());

	}



	bool AppLayer::OnMouseMoved(AlphaEngine::MouseMovedEvent& event)
	{
		m_MousePosition = { static_cast<float>(event.GetX()), static_cast<float>(event.GetY()) };

		// Returning false means that other layers can also respond if needed
		return false;
	}

	bool AppLayer::OnWindowClosed(AlphaEngine::WindowClosedEvent& event)
	{
		std::cout << "Window Closed!" << std::endl;

		// Returning false means that other layers can also respond if needed
		return false;
	}

	bool AppLayer::OnResize(AlphaEngine::WindowResizeEvent& event)
	{
		m_WindowWidth = static_cast<float>(event.GetWidth());
		m_WindowHeight = static_cast<float>(event.GetHeight());

		return false;
	}

	void AppLayer::HandleMouseClick()
	{
		auto& ecsOrchestrator = ServiceLocator::Get<ECSOrchestrator>();

		auto& camera = ecsOrchestrator.GetComponent<CameraComponent>(ecsOrchestrator.GetPrimaryCamera());
		auto& transform = ecsOrchestrator.GetComponent<TransformComponent>(ecsOrchestrator.GetPrimaryCamera());
		auto& physicsSystem = ecsOrchestrator.GetSystem<PhysicsSystem>();

		// Creating our Ray from getting the mouse position from screen (+window height + width) to world 
		Ray ray = AlphaEngine::CameraUtils::ScreenToWorldRay(m_MousePosition.x, m_MousePosition.y, m_WindowWidth, m_WindowHeight, camera, transform.position);

		// We cast that ray and get a body id If valid we see what we received 
		// We also check filters and such stuff
		JPH::BodyID hitID = physicsSystem.CastRay(ray.origin, ray.direction, 100.0f);
		if (!hitID.IsInvalid())
		{
			JPH::BodyInterface& bodyInterface = physicsSystem.jolt_PhysicsSystem->GetBodyInterface();

			// Filter: Only kick dynamic objects (Balls), not the Static floor!
			if (bodyInterface.GetMotionType(hitID) == JPH::EMotionType::Dynamic) {

				// Wake the body up (Essential for sleeping objects!)
				bodyInterface.ActivateBody(hitID);

				// IMpulse Logic
				JPH::Vec3 impulse = JPH::Vec3(0.0f, 5.0f, 0.0f) +
					JPH::Vec3(ray.direction.x, ray.direction.y, ray.direction.z) * 2.0f;

				bodyInterface.AddImpulse(hitID, impulse);

				// Get the Entity ID back from Jolt (The Link)
				Entity hitEntity = static_cast<Entity>(bodyInterface.GetUserData(hitID));
				std::cout << "Kicked Entity: " << hitEntity.GetId() << " (Jolt ID: " << hitID.GetIndex() << ")" << std::endl;
			}
		}
	}
}
