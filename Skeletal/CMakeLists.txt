cmake_minimum_required(VERSION 3.14)
project(Skeletal)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADER_FILES
        ${CMAKE_SOURCE_DIR}/src/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE IMGUI_SRC
        ${CMAKE_SOURCE_DIR}/External/imgui/*.h
        ${CMAKE_SOURCE_DIR}/External/imgui/*.cpp
        ${CMAKE_SOURCE_DIR}/External/stb_image/*.h
        )

file(COPY Resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${IMGUI_SRC_FILES})

list(APPEND CMAKE_PREFIX_PATH /usr/local/Cellar)

link_directories(/usr/local/Cellar/glfw/3.3/lib) # I know hard coding paths sux... but find_package doesnt work correctly on my machine
include_directories(/usr/local/Cellar/glfw/3.3/include)
target_link_libraries (${PROJECT_NAME} /usr/local/Cellar/glfw/3.3/lib/libglfw.3.3.dylib)

include_directories(/usr/local/Cellar/glm/0.9.9.0/include)

link_directories(/usr/local/Cellar/assimp/5.0.1/lib)
include_directories(/usr/local/Cellar/assimp/5.0.1/include)
target_link_libraries (${PROJECT_NAME} /usr/local/Cellar/assimp/5.0.1/lib/libassimp.5.0.0.dylib)

#GLAD 4, loaders for openGL 4.1 CORE
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/External/glad4_1")
add_library("glad" "${GLAD_DIR}/src/glad.c")
include_directories(${GLAD_DIR}/include)

target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/External/glad4_1/libglad.a)
include_directories(${CMAKE_SOURCE_DIR}/External/stb_image
        ${CMAKE_SOURCE_DIR}/External/imgui
        ${CMAKE_SOURCE_DIR}/External/glad4_1/include
        )