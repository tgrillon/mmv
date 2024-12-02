set(ZNOISE_SOURCE_DIR "./Source")
set(ZNOISE_INCLUDE_DIR "./Include")

add_library(znoise STATIC   ${ZNOISE_SOURCE_DIR}/FBM.cpp
                            ${ZNOISE_SOURCE_DIR}/HybridMultiFractal.cpp
                            ${ZNOISE_SOURCE_DIR}/MixerBase.cpp
                            ${ZNOISE_SOURCE_DIR}/NoiseBase.cpp
                            ${ZNOISE_SOURCE_DIR}/NoiseTools.cpp
                            ${ZNOISE_SOURCE_DIR}/Perlin.cpp
                            ${ZNOISE_SOURCE_DIR}/Simplex.cpp
                            ${ZNOISE_SOURCE_DIR}/Worley.cpp

                            ${ZNOISE_INCLUDE_DIR}/Enums.hpp
                            ${ZNOISE_INCLUDE_DIR}/FBM.hpp
                            ${ZNOISE_INCLUDE_DIR}/HybridMultiFractal.hpp
                            ${ZNOISE_INCLUDE_DIR}/MixerBase.hpp
                            ${ZNOISE_INCLUDE_DIR}/NoiseBase.hpp
                            ${ZNOISE_INCLUDE_DIR}/NoiseTools.hpp
                            ${ZNOISE_INCLUDE_DIR}/Perlin.hpp
                            ${ZNOISE_INCLUDE_DIR}/Simplex.hpp
                            ${ZNOISE_INCLUDE_DIR}/Worley.hpp)

target_include_directories(znoise PUBLIC ${ZNOISE_INCLUDE_DIR})
