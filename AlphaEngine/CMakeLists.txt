# Add source files here
add_library(${ALPHA_ENGINE_TARGET_NAME} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Application.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/AssetManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/AssetManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Event.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/InputEvents.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/WindowEvents.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Layer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Window.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Window.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Logger.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Logger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/EventBus.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Delegate.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/ServiceLocator.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Utility.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Input.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Input.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Mesh.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Shader.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Shader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/FileSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Renderer/IRenderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Renderer/OpenGLRenderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Renderer/OpenGLRenderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/ECS/ECS.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/ECS/ECS.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Components/TransformComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Components/RendererComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Components/CameraComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Components/PlayerControllerComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Components/VelocityComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Systems/MovementSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Systems/RenderSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Systems/CameraSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Systems/PlayerControllerSystem.h
)

target_include_directories(${ALPHA_ENGINE_TARGET_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Connect the libraries to the game
target_link_libraries(${ALPHA_ENGINE_TARGET_NAME} PUBLIC 
   glad        
   glfw       
   glm::glm   
   OpenGL::GL 
)