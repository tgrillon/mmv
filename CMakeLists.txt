cmake_minimum_required(VERSION 3.15...3.26)
project(mmv)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

# Fetching gkit library 
add_subdirectory(vendor/gkit)
# Fetching imgui library 
add_subdirectory(vendor/imgui)
# Fetching exprtk library 
add_subdirectory(vendor/exprtk)
# Fetching znoise library 
add_subdirectory(vendor/znoise)

add_subdirectory(src)     
