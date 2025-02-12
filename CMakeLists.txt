cmake_minimum_required(VERSION 3.22)
project(MoviesApp)

set(CMAKE_CXX_STANDARD 23)

# Set path to ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
file(GLOB IMGUI_SOURCES
        ${IMGUI_DIR}/*.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )

# Set paths for GLFW
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/glfw-3.4/include")
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/external/glfw-3.4/build/src/libglfw3.a")

# Find OpenGL library
find_package(OpenGL REQUIRED)

# Include directories for ImGui and GLFW
include_directories(
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${GLFW_INCLUDE_DIR}
)

# FetchContent to include httplib and nlohmann/json
include(FetchContent)

FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.12.5
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Define the executable and source files
add_executable(MoviesApp main.cpp ${IMGUI_SOURCES} OMDbApi.cpp OMDbApi.h GuiManager.cpp GuiManager.h Movie.h ImageLoader.cpp ImageLoader.h)

# Link libraries
target_link_libraries(MoviesApp ${GLFW_LIBRARY} OpenGL::GL httplib nlohmann_json::nlohmann_json)
