# Generic Project Variables
cmake_minimum_required(VERSION 4.2.1)

project(Alpha
	VERSION 1.0.0
	LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD			20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

# Variables
set(ALPHA_GAME_TARGET_NAME AlphaGame)
set(ALPHA_ENGINE_TARGET_NAME AlphaEngine)

# Add Static libraries as external
add_library(glad STATIC 
    external/glad/src/gl.c
)
target_include_directories(glad PUBLIC 
    external/glad/include
)


# <--Dependency Management-->
include(FetchContent)

	# Fetch GLFW (Windowing & Input)
	FetchContent_Declare(glfw
		GIT_REPOSITORY https://github.com/glfw/glfw
		GIT_TAG 3.4
		GIT_SHALLOW TRUE
	)

	# Fetch GLM (Math) 
	FetchContent_Declare(glm
		GIT_REPOSITORY https://github.com/g-truc/glm
		GIT_TAG 1.0.3
		GIT_SHALLOW TRUE
	)

	# Disable unnecessary GLFW internal builds to speed up your compile
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
	set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

# <--Find System OpenGL-->
find_package(OpenGL REQUIRED)

# <--Make them available-->
FetchContent_MakeAvailable(glfw glm)

# <--Subdirectories-->
add_subdirectory(AlphaEngine)
add_subdirectory(AlphaGame)

# Set The startup Project to be your Game
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${ALPHA_GAME_TARGET_NAME})

