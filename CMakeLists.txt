cmake_minimum_required(VERSION 3.10)

# Project name
project(ReBit)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui directory
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)

# ImGui directory
set(TINYFILEDIALOGS_DIR ${RESOURCES_DIR}/tinyfiledialogs)

# ImGui directory
set(IMGUI_DIR ${RESOURCES_DIR}/imgui)

# GLFW directory
set(GLFW_DIR ${RESOURCES_DIR}/glfw)

# Define sources
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/source_files/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source_files/*.h"
)

list(APPEND SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${TINYFILEDIALOGS_DIR}/tinyfiledialogs.c
)

# Configuration for GLFW
set(GLFW_INCLUDE_DIR ${GLFW_DIR}/include)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GLFW_LIB_DIR ${GLFW_DIR}/lib-vc2010-64)
else()
    set(GLFW_LIB_DIR ${GLFW_DIR}/lib-vc2010-32)
endif()

# Link GLFW library
set(PLATFORM_LIBS
    ${GLFW_LIB_DIR}/glfw3.lib
    gdi32
    opengl32
    imm32
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${TINYFILEDIALOGS_DIR}
    ${GLFW_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/source_files
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_LIBS})

# Remove ALL_BUILD and ZERO_CHECK targets from the generated Visual Studio project
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

