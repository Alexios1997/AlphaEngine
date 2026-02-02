# Add source files here
add_library(${ALPHA_ENGINE_TARGET_NAME} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Application.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Event.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/InputEvents.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/WindowEvents.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Layer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Window.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Window.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Logger.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Logger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Renderer/Renderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Renderer/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/ECS/ECS.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/ECS/ECS.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Components/TransformComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineFramework/Systems/MovementSystem.h
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