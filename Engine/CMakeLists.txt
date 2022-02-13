cmake_minimum_required(VERSION 3.20)
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

# Shader slots generation
set(SHADER_SLOT_COUNT 128)
math(EXPR LAST_SHADER_SLOT "${SHADER_SLOT_COUNT}-1")
set(SLOT_GEN "${ZE_BUILD_INC_DIR}/SlotsGen.h")
file(WRITE ${SLOT_GEN}
    "#pragma once\n"
    "#define ZE_GEN_SLOTS0(X)\n")
foreach(I RANGE 1 ${LAST_SHADER_SLOT})
    math(EXPR PREV_SLOT "${I}-1")
    file(APPEND ${SLOT_GEN} "#define ZE_GEN_SLOTS${I}(X) ZE_GEN_SLOTS${PREV_SLOT}(X) ##X##${PREV_SLOT} = ${PREV_SLOT},\n")
endforeach()
file(APPEND ${SLOT_GEN}
    "#define ZE_GEN_SLOTS(X) ZE_GEN_SLOTS${LAST_SHADER_SLOT}(X) Count = ${SHADER_SLOT_COUNT}")

# Post build data copy
if(NOT (ZE_NO_DATA OR ZE_CI_JOB))
    copy_runtime_data(ENGINE "Data" "" "")
endif()

# DirectX 12 debug library
if(WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    copy_runtime_data(ENGINE_PIX "${PIX_DIR}/bin/x64" "" ".dll")
endif()

# Target files
set(SRC_DIR "${ENGINE_DIR}/Source")
set(PCH "${ENGINE_INC_DIR}/pch.h")
set(STB "${EXT_INC_DIR}/stb_sprintf.h")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${ENGINE_INC_DIR}/*.h"
    "${ZE_BUILD_INC_DIR}/*.h")

file(GLOB IMGUI_SRC_LIST
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_INC_DIR}/*.h"
    "${IMGUI_DIR}/misc/cpp/*.cpp"
    "${IMGUI_INC_DIR}/misc/cpp/*.h"
    "${IMGUI_DIR}/misc/freetype/*.cpp"
    "${IMGUI_INC_DIR}/misc/freetype/*.h")
if(MSVC)
    set(IMGUI_SRC_LIST "${IMGUI_SRC_LIST}"
        "${IMGUI_DIR}/misc/debuggers/imgui.natvis")
endif()
if(WIN32)
    file(GLOB IMGUI_BACKEND_LIST
        "${IMGUI_DIR}/backends/*win32.cpp"
        "${IMGUI_DIR}/backends/*win32.h"
        "${IMGUI_DIR}/backends/*dx11.cpp"
        "${IMGUI_DIR}/backends/*dx11.h"
        "${IMGUI_DIR}/backends/*dx12.cpp"
        "${IMGUI_DIR}/backends/*dx12.h")
else()
	message(FATAL_ERROR "Building ImGui for unsupported platform!")
endif()

# Target setup
add_library(${ENGINE_TARGET} STATIC ${SRC_LIST} ${IMGUI_SRC_LIST} ${IMGUI_BACKEND_LIST} ${STB} ${ZE_API_HEADER})

target_compile_features(${ENGINE_TARGET} PUBLIC ${ZE_CXX_STD})
target_include_directories(${ENGINE_TARGET}
    PUBLIC ${ENGINE_INC_DIR} ${ZE_BUILD_INC_DIR} ${ZE_API_DIR} ${EXT_INC_DIR} ${ASSIMP_INC_DIR} ${IMGUI_INC_DIR} ${FTYPE_INC_DIR} ${PIX_INC_DIR})
target_precompile_headers(${ENGINE_TARGET} PUBLIC ${PCH})
target_link_libraries(${ENGINE_TARGET}
    PUBLIC D3DCompiler dxgi dxguid d3d11 d3d12 ${COMMON_TARGET} ${ZLIB_TARGET} ${ASSIMP_TARGET} ${FTYPE_TARGET} ${PIX_TARGET})
add_dependencies(${ENGINE_TARGET} ${SHADER_TARGET} ${ENGINE_COPY_TARGET} ${ENGINE_PIX_COPY_TARGET})

# Icon definition
if(WIN32)
    target_compile_definitions(${ENGINE_TARGET} PUBLIC -DZE_APPICON=${ZE_ICON_ID})
endif()