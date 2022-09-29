cmake_minimum_required(VERSION 3.20)
include(ShaderBuild)
include(PostBuild)

############# SHADER RULES ##############

set(SHADER_TARGET "Shaders")
setup_shader(${ZE_BIN_DIR} ${ENGINE_DIR} ${SHADER_FLAGS})

# Shader compilation
add_shader_type(VS)
add_shader_type(GS)
add_shader_type(PS)
add_shader_type(CS)
add_shader_target(${SHADER_TARGET})


############# ENGINE RULES ##############

# Post build data copy
if(NOT (ZE_NO_DATA OR ZE_CI_JOB))
    copy_runtime_data(ENGINE "Data" "" "" "")
endif()

# DirectX 12 debug library and AgilitySDK
if(WIN32)
    if(MODE_DEBUG OR MODE_RELINFO)
        copy_runtime_data(ENGINE_PIX "${PIX_DIR}/bin/x64" "" ".dll" "")
        copy_runtime_data(ENGINE_AGILITY_DEBUG "${AGILITY_DIR}/bin/x64" "" "d3d12SDKLayers.dll" "D3D12/")
        copy_runtime_data(ENGINE_AGILITY_SYMBOLS "${AGILITY_DIR}/bin/x64" "" ".pdb" "D3D12/")
    endif()
    copy_runtime_data(ENGINE_AGILITY "${AGILITY_DIR}/bin/x64" "" "D3D12Core.dll" "D3D12/")
endif()

# Hardware specific lib
copy_runtime_data(ENGINE_AGS "${AGS_DIR}/ags_lib/lib" "${AGS_LIB}" ".dll" "")

# Target files
set(SRC_DIR "${ENGINE_DIR}/Source")
set(PCH "${ENGINE_INC_DIR}/pch.h")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${ENGINE_INC_DIR}/*.h"
    "${ZE_BUILD_INC_DIR}/*.h"
    "${AGILITY_INC_DIR}/*.h"
    "${EXT_INC_DIR}/*.h")

file(GLOB IMGUI_SRC_LIST
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_INC_DIR}/*.h"
    "${IMGUI_DIR}/misc/cpp/*.cpp"
    "${IMGUI_INC_DIR}/misc/cpp/*.h"
    "${IMGUI_DIR}/misc/freetype/*.cpp"
    "${IMGUI_INC_DIR}/misc/freetype/*.h")
if(WIN32)
    file(GLOB IMGUI_BACKEND_LIST
        "${IMGUI_DIR}/backends/*win32.cpp"
        "${IMGUI_DIR}/backends/*win32.h"
        "${IMGUI_DIR}/backends/*dx11.cpp"
        "${IMGUI_DIR}/backends/*dx11.h"
        "${IMGUI_DIR}/backends/*dx12.cpp"
        "${IMGUI_DIR}/backends/*dx12.h")
    set(IMGUI_SRC_LIST "${IMGUI_SRC_LIST}"
        "${IMGUI_BACKEND_LIST}")
else()
	message(FATAL_ERROR "Building ImGui for unsupported platform!")
endif()
if(MSVC)
    # Disable high warnings for ImGui
    if(CMAKE_CXX_FLAGS MATCHES "/Wall")
        string(REPLACE "/Wall" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set_source_files_properties(${IMGUI_SRC_LIST} PROPERTIES COMPILE_FLAGS /W4)
        set_source_files_properties(${SRC_LIST} PROPERTIES COMPILE_FLAGS /Wall)
        set_source_files_properties(${PCH} PROPERTIES COMPILE_FLAGS /Wall)
    endif()

    set(IMGUI_SRC_LIST "${IMGUI_SRC_LIST}"
        "${IMGUI_DIR}/misc/debuggers/imgui.natvis")
endif()

# Target setup
add_library(${ENGINE_TARGET} STATIC ${SRC_LIST} ${IMGUI_SRC_LIST} ${ZE_API_HEADER})

target_compile_features(${ENGINE_TARGET} PUBLIC ${ZE_CXX_STD})
target_include_directories(${ENGINE_TARGET} PUBLIC ${ZE_INC_DIRS})
target_precompile_headers(${ENGINE_TARGET} PUBLIC ${PCH})
target_link_libraries(${ENGINE_TARGET}
    PUBLIC D3DCompiler dxgi dxguid d3d11 d3d12 ${COMMON_TARGET} ${ZLIB_TARGET} ${ASSIMP_TARGET} ${FTYPE_TARGET} ${PIX_TARGET} ${ENTT_TARGET} ${AGS_TARGET})
add_dependencies(${ENGINE_TARGET} ${SHADER_TARGET} ${ENGINE_COPY_TARGET} ${ENGINE_PIX_COPY_TARGET} ${ENGINE_AGILITY_DEBUG_COPY_TARGET} ${ENGINE_AGILITY_SYMBOLS_COPY_TARGET} ${ENGINE_AGILITY_COPY_TARGET} ${ENGINE_AGS_COPY_TARGET})

# Icon definition
if(WIN32)
    target_compile_definitions(${ENGINE_TARGET} PUBLIC -DZE_APPICON=${ZE_ICON_ID})
endif()