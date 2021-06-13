cmake_minimum_required(VERSION 3.19)
include(ShaderBuild)
include(PostBuild)

############# SHADER RULES ##############

set(SHADER_TARGET "Shaders")
setup_shader(${ZE_BIN_DIR} ${ENGINE_DIR} "${EXTERNAL_DIR}/fxc.exe" ${SHADER_FLAGS})

# Shader compilation
add_shader_type(VS)
add_shader_type(GS)
add_shader_type(PS)
add_shader_type(CS)
add_shader_target(${SHADER_TARGET})


############# ENGINE RULES ##############

# Post build data copy
if(NOT ZE_NO_DATA OR NOT ZE_CI_JOB)
    copy_runtime_data(ENGINE "Data")
endif()

# Target files
set(SRC_DIR "${ENGINE_DIR}/Source")
set(PCH "${ENGINE_INC_DIR}/pch.h")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${ENGINE_INC_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_INC_DIR}/*.h")

# Target setup
add_library(${ENGINE_TARGET} STATIC ${SRC_LIST} ${ZE_API_HEADER})

target_compile_features(${ENGINE_TARGET} PUBLIC ${ZE_CXX_STD})
target_include_directories(${ENGINE_TARGET}
    PUBLIC ${ENGINE_INC_DIR} ${ZE_API_DIR} ${ASSIMP_INC_DIR} ${IMGUI_INC_DIR} ${FTYPE_INC_DIR})
target_precompile_headers(${ENGINE_TARGET} PUBLIC ${PCH})
target_link_libraries(${ENGINE_TARGET}
    PUBLIC D3DCompiler dxguid d3d11 ${COMMON_TARGET} ${ZLIB_TARGET} ${ASSIMP_TARGET} ${FTYPE_TARGET})
add_dependencies(${ENGINE_TARGET} ${SHADER_TARGET} ${ENGINE_COPY_TARGET})

# Icon definition
if(WIN32)
    target_compile_definitions(${ENGINE_TARGET} PUBLIC -DZE_APPICON=${ZE_ICON_ID})
endif()