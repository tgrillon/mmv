set(SOURCE_DIR "Source")
set(INCLUDE_DIR "Include")
set(TEST_DIR "Test")
add_executable(${PROJECT_NAME} main.cpp 
                               ${SOURCE_DIR}/Window.cpp
                               ${SOURCE_DIR}/App.cpp
                               ${SOURCE_DIR}/Camera.cpp
                               ${SOURCE_DIR}/CameraSystem.cpp
                               ${SOURCE_DIR}/Framebuffer.cpp
                               ${SOURCE_DIR}/Viewer.cpp
                               ${SOURCE_DIR}/HeightField.cpp
                               ${SOURCE_DIR}/PerlinNoise.cpp
                               ${SOURCE_DIR}/Timer.cpp
                               ${SOURCE_DIR}/vecext.cpp
                               ${SOURCE_DIR}/pch.cpp

                               ${INCLUDE_DIR}/Window.h
                               ${INCLUDE_DIR}/App.h
                               ${INCLUDE_DIR}/Camera.h
                               ${INCLUDE_DIR}/CameraSystem.h
                               ${INCLUDE_DIR}/Framebuffer.h
                               ${INCLUDE_DIR}/Viewer.h
                               ${INCLUDE_DIR}/HeightField.h
                               ${INCLUDE_DIR}/PerlinNoise.h
                               ${INCLUDE_DIR}/Timer.h
                               ${INCLUDE_DIR}/vecext.h
                               ${INCLUDE_DIR}/Utils.h
                               ${INCLUDE_DIR}/Memory.h
                               ${INCLUDE_DIR}/pch.h

                               ${TEST_DIR}/HeightFieldTest.cpp
                               )

target_link_libraries(${PROJECT_NAME} PRIVATE gkit
                                              imgui
                                              exprtk
                                              )
                                              
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR}) 
target_precompile_headers(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR/pch.h})

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Path to the data directory.")
message(STATUS "Data directory set to: ${DATA_DIR}")
set(SHADER_DIR "${DATA_DIR}/shaders" CACHE PATH "Path to the shader directory.")
message(STATUS "Shader directory set to: ${SHADER_DIR}")
message(STATUS "Map directory set to: ${MAP_DIR}")

target_compile_definitions(${PROJECT_NAME} PUBLIC DATA_DIR="${DATA_DIR}"
                                           SHADER_DIR="${SHADER_DIR}"
                                           )           
