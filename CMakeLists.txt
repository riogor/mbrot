cmake_minimum_required(VERSION 3.10)
project(mbrot)

set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_library(glad STATIC 
    ext/glad/src/gl.c
)

target_include_directories(glad PUBLIC 
    ext/glad/include
)

target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})

set(SOURCES
    main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(ext/glfw)

target_include_directories(${PROJECT_NAME}
    PUBLIC ext/glfw/include
    PUBLIC ext/glad/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE glad
    PRIVATE glfw
)

target_link_libraries(${PROJECT_NAME}
    glad 
    glfw 
    OpenGL::OpenGL
    OpenGL::EGL
)

target_compile_options(${PROJECT_NAME} PUBLIC -O3 -Ofast -march=native -funroll-loops)
set(OpenGL_GL_PREFERENCE GLVND)
