# Add source files here
add_executable(${ALPHA_GAME_TARGET_NAME} 
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders
	${CMAKE_CURRENT_SOURCE_DIR}/Source/GameFramework/AppLayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/GameFramework/AppLayer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/GameFramework/Main.cpp
)


# Connect the libraries to the game
target_link_libraries(${ALPHA_GAME_TARGET_NAME} PRIVATE ${ALPHA_ENGINE_TARGET_NAME})
