# Generic Project Sets
cmake_minimum_required(VERSION 4.2.1)

project(Alpha
	VERSION 1.0.0
	LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD			17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

# Creates a variable.
set(ALPHA_GAME_TARGET_NAME AlphaGame)

add_subdirectory(Alpha)


