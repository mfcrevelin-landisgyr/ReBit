cmake_minimum_required(VERSION 3.10)

project(ReBit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source code directory
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Source)

# Third party code directory
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/ThirdParty)



# Tiny File Dialogs directory
set(TINYFILEDIALOGS_DIR ${THIRDPARTY_DIR}/tinyfiledialogs)

# ImGui directory
set(IMGUI_DIR ${THIRDPARTY_DIR}/imgui)

# GLFW directory
set(GLFW_DIR ${THIRDPARTY_DIR}/glfw)



# Define sources
file(GLOB_RECURSE SOURCES 
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
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
    ${SOURCE_DIR}
    ${TINYFILEDIALOGS_DIR}
    ${GLFW_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_LIBS})

# Remove ALL_BUILD and ZERO_CHECK targets from the generated Visual Studio project
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

