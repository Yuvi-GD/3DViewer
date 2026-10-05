# CMakeList.txt : CMake project for 3DViewer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("3DViewer")

# Add source to this project's executable.
add_executable (3DViewer "3DViewer.cpp" "3DViewer.h" "model.cpp" "model.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
	set(CMAKE_C_STANDARD 99)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	set_property(TARGET 3DViewer PROPERTY CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# ============================================================
# Paths
# ============================================================
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
set(SOKOL_DIR "${THIRD_PARTY_DIR}/sokol")
set(SHDC_EXECUTABLE "${THIRD_PARTY_DIR}/sokol-tools/sokol-shdc.exe")
add_subdirectory(${THIRD_PARTY_DIR}/cglm)

# Link cglm to your main executable globally
target_link_libraries(3DViewer PRIVATE cglm)

# ============================================================
# Shader Compilation via sokol-shdc
# ============================================================

# ensure output dir exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

set(SHADER_INPUT ${CMAKE_SOURCE_DIR}/shaders/triangle.glsl)
set(SHADER_OUTPUT ${CMAKE_SOURCE_DIR}/shaders/triangle_shader.h)

add_custom_command(
    OUTPUT ${SHADER_OUTPUT}
    COMMAND "${SHDC_EXECUTABLE}"
            --input ${SHADER_INPUT}
            --output ${SHADER_OUTPUT}
            --format sokol
            --slang glsl430:hlsl5:metal_macos
    DEPENDS ${SHADER_INPUT}
    COMMENT "Compiling triangle.glsl into triangle_shader.h"
)

add_custom_target(compile_shaders DEPENDS ${SHADER_OUTPUT})
add_dependencies(3DViewer compile_shaders)

target_include_directories(3DViewer PRIVATE ${CMAKE_SOURCE_DIR}/shaders)
target_sources(3DViewer PRIVATE ${SHADER_OUTPUT})


if (APPLE)
    # Force the C++ file to compile as Objective-C++ for Metal support
    set_source_files_properties("3DViewer.cpp" PROPERTIES COMPILE_FLAGS "-x objective-c++")
    
    # Link native macOS frameworks
    target_link_libraries(3DViewer PRIVATE 
        "-framework AppKit" 
        "-framework QuartzCore" 
        "-framework Metal" 
        "-framework MetalKit"
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Link required X11 and OpenGL libraries for Linux
    target_link_libraries(3DViewer PRIVATE X11 Xi Xcursor GL dl pthread m)
endif()