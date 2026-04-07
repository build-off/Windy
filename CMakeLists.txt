cmake_required_minimum(3.14)
project(Windy C CXX)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")

add_executable(imgui_exm
    ./src/imgui_glfw_vulkan_exm.cpp

    ./src/imgui/imconfig.h
    ./src/imgui/imgui.h
    ./src/imgui_impl_glfw.cpp
    ./src/imgui_impl_vulkan.cpp
)

target_include_directories(imgui_exm PRIVATE
    ./src/backends
)

target_link_libraries(imgui_exm PRIVATE
    glfw3
    Vulkan::Vulkan
)





