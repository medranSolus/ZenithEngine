cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})

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
if(NOT (${ZE_NO_DATA} OR ${ZE_CI_JOB}))
    copy_runtime_data(ENGINE "Data" "" "" "")
endif()

# DirectX 12 debug library and AgilitySDK
if(${ZE_ENABLE_DX12})
    if(NOT ${ZE_BUILD_RELEASE})
        copy_runtime_data(ENGINE_PIX "${PIX_DIR}/bin/x64" "" ".dll" "")
        copy_runtime_data(ENGINE_AGILITY_DEBUG "${AGILITY_DIR}/bin/x64" "" "d3d12SDKLayers.dll" "AgilitySDK/")
        copy_runtime_data(ENGINE_AGILITY_SYMBOLS "${AGILITY_DIR}/bin/x64" "" ".pdb" "AgilitySDK/")
    endif()

    copy_runtime_data(ENGINE_AGILITY "${AGILITY_DIR}/bin/x64" "" "D3D12Core.dll" "AgilitySDK/")
    copy_runtime_data(ENGINE_DSTORE "${DSTORE_DIR}/bin/x64" "" "*.dll" "")
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
# Enable only current OS+RHI
current_config_compile_filter(SRC_LIST ${ENGINE_INC_DIR} ${SRC_DIR})
# Manually specify RHI agnostic headers
list(APPEND SRC_LIST
    "${ENGINE_INC_DIR}/RHI/ApiType.h"
    "${ENGINE_INC_DIR}/RHI/Backend.h")

file(GLOB IMGUI_SRC_LIST
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_INC_DIR}/*.h"
    "${IMGUI_DIR}/misc/cpp/*.cpp"
    "${IMGUI_INC_DIR}/misc/cpp/*.h"
    "${IMGUI_DIR}/misc/freetype/*.cpp"
    "${IMGUI_INC_DIR}/misc/freetype/*.h")
if(${ZE_PLATFORM_WINDOWS})
    file(GLOB IMGUI_OS_LIST
        "${IMGUI_DIR}/backends/*win32.cpp"
        "${IMGUI_DIR}/backends/*win32.h")
else()
	message(FATAL_ERROR "Building ImGui for unsupported platform!")
endif()
if(${ZE_ENABLE_DX11})
    file(GLOB IMGUI_DX11_LIST
        "${IMGUI_DIR}/backends/*dx11.cpp"
        "${IMGUI_DIR}/backends/*dx11.h")
endif()
if(${ZE_ENABLE_DX12})
    file(GLOB IMGUI_DX12_LIST
        "${IMGUI_DIR}/backends/*dx12.cpp"
        "${IMGUI_DIR}/backends/*dx12.h")
endif()
if(${ZE_ENABLE_VK})
    file(GLOB IMGUI_VK_LIST
        "${IMGUI_DIR}/backends/*vulkan.cpp"
        "${IMGUI_DIR}/backends/*vulkan.h")
endif()

list(APPEND IMGUI_SRC_LIST
    ${IMGUI_OS_LIST}
    ${IMGUI_DX11_LIST}
    ${IMGUI_DX12_LIST}
    ${IMGUI_VK_LIST}
    "${IMGUI_DIR}/misc/debuggers/imgui.natvis")

# Disable warnings for ImGui
if(${ZE_COMPILER_MSVC})
    set_source_files_properties(${IMGUI_SRC_LIST} PROPERTIES COMPILE_FLAGS "/wd5219 /wd5019 /wd5039 /wd4774 /wd4245")
endif()

# Target setup
add_library(${ENGINE_TARGET} STATIC ${SRC_LIST} ${IMGUI_SRC_LIST} ${ZE_API_HEADER})

# Link targets for DirectX
if(${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12})
    set(DX_TARGETS D3DCompiler dxgi dxguid uuid)

    if(${ZE_ENABLE_DX11})
        set(DX_TARGETS ${DX_TARGETS} d3d11)
    endif()
    if(${ZE_ENABLE_DX12})
        set(DX_TARGETS ${DX_TARGETS} d3d12)
    endif()
endif()

target_compile_features(${ENGINE_TARGET} PUBLIC ${ZE_CXX_STD})
target_include_directories(${ENGINE_TARGET} PUBLIC ${ZE_INC_DIRS})
target_precompile_headers(${ENGINE_TARGET} PUBLIC ${PCH})
target_link_libraries(${ENGINE_TARGET}
    PUBLIC ${DX_TARGETS} ${COMMON_TARGET} ${AGS_TARGET} ${ASSIMP_TARGET} ${DSTORE_TARGET} ${ENTT_TARGET} ${FFXSDK_TARGETS} ${FTYPE_TARGET} ${LIBPNG_TARGET} ${PIX_TARGET} ${VOLK_TARGET} ${ZLIB_TARGET})
add_dependencies(${ENGINE_TARGET} ${ENGINE_COPY_TARGET} ${SHADER_TARGET} ${ENGINE_AGILITY_COPY_TARGET} ${ENGINE_AGILITY_DEBUG_COPY_TARGET} ${ENGINE_AGILITY_SYMBOLS_COPY_TARGET} ${ENGINE_AGS_COPY_TARGET} ${ENGINE_DSTORE_COPY_TARGET} ${ENGINE_PIX_COPY_TARGET})

# Icon definition
if(${ZE_PLATFORM_WINDOWS})
    target_compile_definitions(${ENGINE_TARGET} PUBLIC -DZE_APPICON=${ZE_ICON_ID})
endif()