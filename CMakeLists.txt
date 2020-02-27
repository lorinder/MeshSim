cmake_minimum_required(VERSION 3.10)

project(MeshSim
	DESCRIPTION "Mesh network simulator"
	LANGUAGES C CXX)

# Use C11 and CXX11
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Enable compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES GNU OR CMAKE_C_COMPILER_ID MATCHES CLANG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# Add source to module path so the Findns3.cmake module will be found
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(ns3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS
             filesystem)
add_subdirectory(sim)
add_subdirectory(ns3_apps)
