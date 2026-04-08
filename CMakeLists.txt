cmake_minimum_required(VERSION 3.14)
project(Windy C CXX)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(imgui_exm
    ./src/imgui_glfw_vulkan_exm.cpp

    # imgui
    ./src/imgui/imconfig.h
    ./src/imgui/imgui.h
    ./src/imgui/imgui_internal.h
    ./src/imgui/imgui.cpp
    ./src/imgui/imgui_draw.cpp
    ./src/imgui/imgui_demo.cpp
    ./src/imgui/imgui_tables.cpp
    ./src/imgui/imgui_widgets.cpp

    # backends
    ./src/imgui_impl_glfw.cpp
    ./src/imgui_impl_vulkan.cpp
)

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)

include(FetchContent)
FetchContent_Declare(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

target_include_directories(imgui_exm PRIVATE
    ./src/backends
)

target_link_libraries(imgui_exm PRIVATE
    glfw
    glm::glm
    Vulkan::Vulkan
    PUBLIC TracyClient
)





