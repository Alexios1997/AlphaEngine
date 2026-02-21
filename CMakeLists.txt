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

	# Fetch stb_image 
	FetchContent_Declare(stb
		GIT_REPOSITORY https://github.com/nothings/stb.git
		GIT_SHALLOW TRUE
	)

	# Fetch Assimp
	FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.4 
    GIT_SHALLOW TRUE
	)

	# Fetch Assimp
	FetchContent_Declare(Jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG v5.5.0 
	GIT_SHALLOW TRUE
	SOURCE_SUBDIR  "Build"
	)


	# Disable unnecessary GLFW internal builds to speed up your compile
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
	set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

	# Optimization: Disable everything except the GLTF/GLB importer
	set(ASSIMP_BUILD_ASSIMP_TOOLS	OFF CACHE BOOL "" FORCE)
	set(ASSIMP_BUILD_TESTS			OFF CACHE BOOL "" FORCE)
	set(ASSIMP_INSTALL				OFF CACHE BOOL "" FORCE)
	set(ASSIMP_BUILD_SAMPLES		OFF CACHE BOOL "" FORCE)
	# Required for .glb compression
	set(ASSIMP_BUILD_ZLIB			ON CACHE BOOL "" FORCE)

	# Jolt Configuration Options
	# This is the magic variable Jolt looks for to decide between /MT and /MD
	set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)

	# Jolt internal switches
	set(INTERACTIVE_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(SAMPLES OFF CACHE BOOL "" FORCE)
	set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
	set(GENERATE_DEBUG_SYMBOLS ON CACHE BOOL "" FORCE)

	# Tell Jolt not to override your global compiler flags
	set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)

	# Instruction sets - Match these to your CPU (SSE2 is safest)
	set(USE_SSE4_1 OFF CACHE BOOL "" FORCE)
	set(USE_SSE4_2 OFF CACHE BOOL "" FORCE)
	set(USE_AVX OFF CACHE BOOL "" FORCE)
	set(USE_AVX2 OFF CACHE BOOL "" FORCE)


# <--Find System OpenGL-->
find_package(OpenGL REQUIRED)

# <--Make them available-->
FetchContent_MakeAvailable(glfw glm stb)
FetchContent_MakeAvailable(assimp)
FetchContent_MakeAvailable(Jolt)

# <--Subdirectories-->
add_subdirectory(AlphaEngine)
add_subdirectory(AlphaGame)

# Set The startup Project to be your Game
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${ALPHA_GAME_TARGET_NAME})

# Definition needed
target_compile_definitions(${ALPHA_ENGINE_TARGET_NAME} PUBLIC 
    ALPHA_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/"
)

# Jolt REQUIRES these defines to compile and link correctly
target_compile_definitions(${ALPHA_ENGINE_TARGET_NAME} PUBLIC 
    JPH_FLOATING_POINT_EXCEPTIONS_OFF
    JPH_USE_SSE2
	$<$<CONFIG:Debug>:JPH_DEBUG_RENDERER>
    $<$<CONFIG:Release>:JPH_DEBUG_RENDERER>
)

target_include_directories(${ALPHA_ENGINE_TARGET_NAME} PRIVATE ${stb_SOURCE_DIR})

# Link Assimp to my engine
target_link_libraries(${ALPHA_ENGINE_TARGET_NAME} PRIVATE assimp)

# Include it
target_include_directories(${ALPHA_ENGINE_TARGET_NAME} PRIVATE ${assimp_SOURCE_DIR}/include)

# Include it
target_include_directories(${ALPHA_ENGINE_TARGET_NAME} PUBLIC ${Jolt_SOURCE_DIR})

# Link Jolt to my engine
target_link_libraries(${ALPHA_ENGINE_TARGET_NAME} PUBLIC Jolt)



