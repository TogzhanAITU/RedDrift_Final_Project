cmake_minimum_required(VERSION 3.10)
project(PuzzlePlatformer)

set(CMAKE_CXX_STANDARD 17)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(PuzzlePlatformer
    src/main.cpp
    src/Player.cpp
    src/Level.cpp
)

target_include_directories(PuzzlePlatformer PRIVATE src)

# Pass absolute source directory path to preprocessor
target_compile_definitions(PuzzlePlatformer PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(PuzzlePlatformer PRIVATE glfw GLEW::GLEW glm::glm OpenGL::GL)

file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY levels DESTINATION ${CMAKE_CURRENT_BINARY_DIR})