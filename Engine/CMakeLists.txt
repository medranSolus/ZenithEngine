cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})

############# SHADER RULES ##############

set(SHADER_TARGET "Shaders")
setup_shader(${ZE_BIN_DIR} ${ENGINE_DIR} ${SHADER_FLAGS})

# Add permutations
include(Shader/Permutations.cmake)

# Shader compilation
add_shader_type(VS)
add_shader_type(GS)
add_shader_type(PS)
add_shader_type(CS)
add_shader_exclusive_rhi(CS "SSSR" "DX12")
add_shader_exclusive_rhi(CS "Denoiser" "DX12")
add_shader_target(${SHADER_TARGET})


############# ENGINE RULES ##############

# Post build data copy
if(NOT (${ZE_NO_DATA} OR ${ZE_CI_JOB}))
    copy_runtime_data(ENGINE "Data" "" "" "")
endif()

# Copy DirectX 12 libraries
if(${ZE_ENABLE_DX12})
    if(NOT ${ZE_BUILD_RELEASE})
        copy_runtime_data(ENGINE_PIX "${PIX_DIR}/bin/x64" "" ".dll" "")
        copy_runtime_data(ENGINE_AGILITY_DEBUG "${AGILITY_DIR}/bin/x64" "" "d3d12SDKLayers.dll" "AgilitySDK/")
        copy_runtime_data(ENGINE_AGILITY_SYMBOLS "${AGILITY_DIR}/bin/x64" "" ".pdb" "AgilitySDK/")
    endif()
    
    copy_runtime_data(ENGINE_AGILITY "${AGILITY_DIR}/bin/x64" "" "D3D12Core.dll" "AgilitySDK/")
    copy_runtime_data(ENGINE_DSTORE "${DSTORE_DIR}/bin/x64" "" ".dll" "")
    copy_runtime_data(ENGINE_FFXAPI_DX12_LOADER "${FFXAPI_DLL_DIR}" "" "amd_fidelityfx_loader_dx12.dll" "")
    copy_runtime_data(ENGINE_FFXAPI_DX12_UPSCALE "${FFXAPI_DLL_DIR}" "" "amd_fidelityfx_upscaler_dx12.dll" "")
endif()

# Hardware specific lib
if(${ZE_AGS_ENABLED})
    copy_runtime_data(ENGINE_AGS "${AGS_DIR}/ags_lib/lib" "${AGS_LIB}.dll" "" "")
endif()
if(${ZE_XAUDIO2_ENABLED})
    copy_runtime_data(ENGINE_XAUDIO2 "${XAUDIO2_DIR}/release/bin/x64" "${XAUDIO2_LIB}.dll" "" "")
endif()
if(${ZE_XESS_ENABLED})
    copy_runtime_data(ENGINE_XESS "${XESS_DIR}/bin" "${XESS_LIB}.dll" "" "")
endif()
if(${ZE_DLSS_ENABLED})
    if(${ZE_BUILD_RELEASE} OR ${ZE_BUILD_PROFILE})
        set(DLSS_LOCATION "rel")
    else()
        set(DLSS_LOCATION "dev")
    endif()
	if(${ZE_PLATFORM_WINDOWS})
        copy_runtime_data(ENGINE_DLSS "${DLSS_DIR}/lib/Windows_x86_64/${DLSS_LOCATION}" "nvngx_dlss.dll" "" "")
	elseif(${ZE_PLATFORM_LINUX})
        copy_runtime_data(ENGINE_DLSS "${DLSS_DIR}/lib/Linux_x86_64/${DLSS_LOCATION}" "libnvidia-ngx-dlss.so" "" "")
	endif()
endif()

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
    set(DX_TARGETS D3DCompiler dxgi dxguid uuid DWrite)

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
    PUBLIC ${DX_TARGETS} ${COMMON_TARGET} ${ENGINE_EXTERNAL_TARGETS})
add_dependencies(${ENGINE_TARGET} ${ENGINE_COPY_TARGET} ${SHADER_TARGET} ${ENGINE_AGILITY_COPY_TARGET} ${ENGINE_AGILITY_DEBUG_COPY_TARGET} ${ENGINE_AGILITY_SYMBOLS_COPY_TARGET} ${ENGINE_AGS_COPY_TARGET} ${ENGINE_DLSS_COPY_TARGET} ${ENGINE_DSTORE_COPY_TARGET} ${ENGINE_FFXAPI_DX12_LOADER_COPY_TARGET} ${ENGINE_FFXAPI_DX12_UPSCALE_COPY_TARGET} ${ENGINE_FFXAPI_DX12_FRAMEGEN_COPY_TARGET} ${ENGINE_PIX_COPY_TARGET} ${ENGINE_XAUDIO2_COPY_TARGET} ${ENGINE_XESS_COPY_TARGET})

# Icon definition
if(${ZE_PLATFORM_WINDOWS})
    target_compile_definitions(${ENGINE_TARGET} PUBLIC ZE_APPICON=${ZE_ICON_ID})
endif()