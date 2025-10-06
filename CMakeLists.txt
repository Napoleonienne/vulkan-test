cmake_minimum_required(VERSION 3.15)
project(VulkanTestProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Vulkan ---
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

# --- GLFW ---
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)


if(NOT GLFW_FOUND)
    message(FATAL_ERROR "GLFW not found. Installez-le et réessayez.")
endif()

# --- Includes ---
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GLFW_INCLUDE_DIRS}
)

# --- Sources ---
file(GLOB_RECURSE SOURCES
    src/*.cpp
    include/*.h
)

# --- Executable ---
add_executable(vulkan_test ${SOURCES})

# --- Libraries ---
target_link_libraries(vulkan_test
    ${GLFW_LIBRARIES}
    Vulkan::Vulkan
    GL                  # OpenGL (utile pour tests interop)
    X11 Xrandr Xi Xxf86vm Xcursor pthread dl m
)

# --- glslc (compilation shaders Vulkan) ---
find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc non trouvé. Installez le SDK Vulkan.")
endif()

# --- Shaders ---
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders_compiled")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

file(GLOB SHADERS CONFIGURE_DEPENDS
    ${SHADER_DIR}/*.vert
    ${SHADER_DIR}/*.frag
    ${SHADER_DIR}/*.comp
)

set(SPIRV_SHADERS "")
foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV_OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E echo "[GLSLC] Compilation : ${SHADER_NAME}"
        COMMAND ${GLSLC} ${SHADER} -o ${SPIRV_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compilation shader ${SHADER_NAME}"
    )

    list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})
add_dependencies(vulkan_test Shaders)
