cmake_minimum_required(VERSION 3.14)
project(Windy C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")

# allow clangd to find the dependencies, using the simlink created
# in the root of the project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(./cmake/jolt.cmake)
include(./cmake/include.cmake)

# Compiling shaders
# TODO: handle executable for other systems
set(SLANGC_EXECUTABLE vulkan_sdk/1.4.341.1/x86_64/bin/slangc)
function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)
    set(OUTPUT_FILE slang.spv)

    add_custom_command (
		    OUTPUT ${SHADERS_DIR}
		    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
		      -target spirv 
		      -profile spirv_1_4 
			    -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS}
			    -o ${OUTPUT_FILE}
			  WORKING_DIRECTORY ${SHADERS_DIR}
			  DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
			  COMMENT "Compiling Slang Shaders"
			  VERBATIM
    )
    add_custom_command (
	    OUTPUT ${SHADERS_DIR}/slang.spv
	    COMMAND ${SLANGC_EXECUTABLE}
    )
    add_custom_target (${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

set(SHADER_SLANG_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.slang
)
add_slang_shader_target(shaders SOURCES ${SHADER_SLANG_SOURCES})

add_executable(imgui_exm
    ./src/core/entry_point.cpp

    # imgui
    ./src/imgui/imconfig.h
    ./src/imgui/imgui.h
    ./src/imgui/imgui_internal.h
    ./src/imgui/imgui.cpp
    ./src/imgui/imgui_draw.cpp
    ./src/imgui/imgui_demo.cpp
    ./src/imgui/imgui_tables.cpp
    ./src/imgui/imgui_widgets.cpp

    # core
    ./src/core/log.h
    ./src/core/log.cpp
    ./src/core/base.h

    # backends
    ./src/imgui_impl_glfw.cpp
    ./src/imgui_impl_vulkan.cpp
)

add_dependencies(imgui_exm shaders)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(spdlog REQUIRED)

target_include_directories(imgui_exm PRIVATE
    ./src/backends
)

target_link_libraries(imgui_exm PRIVATE
    glfw
    glm::glm
    Vulkan::Vulkan
    Jolt
    spdlog::spdlog
    TracyClient
)







