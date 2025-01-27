cmake_minimum_required(VERSION 3.22)
project(MoviesApp)

set(CMAKE_CXX_STANDARD 23)

# הוספת נתיב imgui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
file(GLOB IMGUI_SOURCES
        ${IMGUI_DIR}/*.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )

# הגדר נתיבים ל-GLFW
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/glfw-3.4/include")
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/external/glfw-3.4/build/src/libglfw3.a")



# מציאת ספריית OpenGL
find_package(OpenGL REQUIRED)

# הכללת קבצי imgui ו-GLFW
include_directories(
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${GLFW_INCLUDE_DIR}
)

# הגדרת קובץ ההרצה והמקורות
add_executable(MoviesApp main.cpp ${IMGUI_SOURCES})

# קישור ספריות
target_link_libraries(MoviesApp ${GLFW_LIBRARY} OpenGL::GL)
