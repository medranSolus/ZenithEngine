cmake_minimum_required(VERSION 3.19)

############# SHADER RULES ##############

set(SHADER_TARGET "Shaders")
set(SHADER_DIR "${ENGINE_DIR}/Shader")
set(FXC "${EXTERNAL_DIR}/fxc.exe")
set(CSO_DIR "${ZE_BIN_DIR}/Shaders")

# Vertex shader files
set(VS_DIR "${SHADER_DIR}/VertexShader")
file(GLOB_RECURSE VS_SRC_LIST "${VS_DIR}/*.hlsl")
file(GLOB_RECURSE VS_INC_LIST "${VS_DIR}/*.hlsli")

# Geometry shader files
set(GS_DIR "${SHADER_DIR}/GeometryShader")
file(GLOB_RECURSE GS_SRC_LIST "${GS_DIR}/*.hlsl")
file(GLOB_RECURSE GS_INC_LIST "${GS_DIR}/*.hlsli")

# Pixel shader files
set(PS_DIR "${SHADER_DIR}/PixelShader")
file(GLOB_RECURSE PS_SRC_LIST "${PS_DIR}/*.hlsl")
file(GLOB_RECURSE PS_INC_LIST "${PS_DIR}/*.hlsli")
 
# Shader compilation macro
#   SHADER_TYPE = prefix type of shader (VS, GS, PS)
macro(add_shader_commands SHADER_TYPE)
    string(TOLOWER "${SHADER_TYPE}_5_0" ${SHADER_TYPE}_TYPE_FLAG)
    foreach(SD IN LISTS ${SHADER_TYPE}_SRC_LIST)
        get_filename_component(SD_NAME ${SD} NAME_WE)
        set(SD_OUT "${CSO_DIR}/${SD_NAME}.cso")
        list(APPEND SHADERS ${SD_OUT})
        add_custom_command(OUTPUT ${SD_OUT}
            COMMAND "${FXC}" ${SHADER_FLAGS} /T "${${SHADER_TYPE}_TYPE_FLAG}" /I "${${SHADER_TYPE}_DIR}" /Fo "${SD_OUT}" "${SD}"
            MAIN_DEPENDENCY "${SD}"
            DEPENDS "${${SHADER_TYPE}_INC_LIST}")
    endforeach()
endmacro()

# Shader compilation
add_shader_commands(VS)
add_shader_commands(GS)
add_shader_commands(PS)
add_custom_target(${SHADER_TARGET} ALL DEPENDS ${SHADERS} VERBATIM)


########## RESOURCE EMBEDDING ###########

set(RES_BIN_DIR "${BUILD_DIR}/Resource")
set(RES_HDR "${RES_BIN_DIR}/resource.h")
set(RES_FILES "${ZE_RES_DIR}/resource.rc" "${RES_HDR}")
file(WRITE "${RES_HDR}"
        "#pragma once\n"
        "#define ZE_ICON_PATH \"${ZE_RES_DIR}/icon.ico\"\n")


############# ENGINE RULES ##############

# Post build data copy
copy_runtime_data(ENGINE "Data/Runtime")

# Target files
set(SRC_DIR "${ENGINE_DIR}/Source")
set(PCH "${ENGINE_INC_DIR}/pch.h")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${ENGINE_INC_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_INC_DIR}/*.h")

# Target setup
add_library(${ENGINE_TARGET} STATIC ${SRC_LIST} ${RES_FILES})

target_compile_features(${ENGINE_TARGET} PUBLIC ${ZE_CXX_STD})
target_include_directories(${ENGINE_TARGET}
    PUBLIC ${ENGINE_INC_DIR} ${RES_BIN_DIR} ${ASSIMP_INC_DIR} ${IMGUI_INC_DIR} ${FTYPE_INC_DIR})
target_precompile_headers(${ENGINE_TARGET} PUBLIC ${PCH})
target_link_libraries(${ENGINE_TARGET}
    PUBLIC D3DCompiler dxguid d3d11 ${COMMON_TARGET} ${ZLIB_TARGET} ${ASSIMP_TARGET} ${FTYPE_TARGET})
add_dependencies(${ENGINE_TARGET} ${SHADER_TARGET} ${ENGINE_COPY_TARGET})