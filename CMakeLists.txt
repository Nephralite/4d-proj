cmake_minimum_required(VERSION 3.30)
project(projectionDemo)

set(CMAKE_CXX_STANDARD 20)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLEW
find_package(GLEW REQUIRED)

# Find FreeGLUT
find_package(PkgConfig REQUIRED)
#find_package(GLFW3 REQUIRED)

add_executable(projectionDemo main.cpp
)

# Link OpenGL, GLEW, and FreeGLUT to your executable
target_link_libraries(projectionDemo
    OpenGL::GL
    GLEW::GLEW
    glfw
)

# Include directories
target_include_directories(projectionDemo
    PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)
