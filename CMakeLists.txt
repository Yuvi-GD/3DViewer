cmake_minimum_required(VERSION 3.12)
project("3DViewer" C CXX)

# ============================================================
# 1. Compiler Settings
# ============================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ============================================================
# 2. Paths & Dependencies
# ============================================================
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

# CGLM Math Library
add_subdirectory(${THIRD_PARTY_DIR}/cglm)

# ============================================================
# 3. Shader Compilation (Sokol-shdc)
# ============================================================
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

set(SHDC_EXECUTABLE "${THIRD_PARTY_DIR}/sokol-tools/sokol-shdc.exe")
set(SHADER_INPUT ${CMAKE_SOURCE_DIR}/shaders/object.glsl)
set(SHADER_OUTPUT ${CMAKE_SOURCE_DIR}/shaders/object_shader.h)

add_custom_command(
    OUTPUT ${SHADER_OUTPUT}
    COMMAND "${SHDC_EXECUTABLE}"
            --input ${SHADER_INPUT}
            --output ${SHADER_OUTPUT}
            --format sokol
            --slang glsl430:hlsl5:metal_macos
    DEPENDS ${SHADER_INPUT}
    COMMENT "Compiling object.glsl into object_shader.h"
)
add_custom_target(compile_shaders DEPENDS ${SHADER_OUTPUT})

# ============================================================
# 4. Main Executable
# ============================================================
add_executable(3DViewer 
    "src/3DViewer.cpp" 
    "src/model.cpp"
    ${SHADER_OUTPUT}
)
add_dependencies(3DViewer compile_shaders)

# Include directories
target_include_directories(3DViewer PRIVATE 
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_SOURCE_DIR}
    ${THIRD_PARTY_DIR}
)

# ============================================================
# 5. OS-Specific Linking & Assets
# ============================================================
target_link_libraries(3DViewer PRIVATE cglm)

if (APPLE)
    set_source_files_properties("3DViewer.cpp" PROPERTIES COMPILE_FLAGS "-x objective-c++")
    target_link_libraries(3DViewer PRIVATE 
        "-framework AppKit" 
        "-framework QuartzCore" 
        "-framework Metal" 
        "-framework MetalKit"
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(3DViewer PRIVATE X11 Xi Xcursor GL dl pthread m)
endif()

# Copy test models to the output directory so the executable can find them
add_custom_command(TARGET 3DViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/model.obj"
    $<TARGET_FILE_DIR:3DViewer>
)