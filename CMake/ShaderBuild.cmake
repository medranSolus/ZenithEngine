include_guard(DIRECTORY)

# Order of macros calls:
#   setup_shader - for creation of variables
#   add_shader_type - multiple calls for each wanted shader type
#   add_shader_target - final call wrapping all previous rules

# Setups variables needed by other shader macros
#   SHADER_DIR = directory containing "Shader" folder with all shader code
#       (each shader type in directory named: VS, GS, PS or CS)
#   BIN_DIR = binary directory
#   FLAGS = flags used for compilation
macro(setup_shader BIN_DIR SHADER_DIR FLAGS)
    set(SD_CSO_DIR "${BIN_DIR}/Shaders")
    set(SD_DIR "${SHADER_DIR}/Shader")
    set(SD_INC_DIR "${SD_DIR}/Common")
    file(GLOB_RECURSE SD_INC_LIST
        "${SD_INC_DIR}/*.hlsli"
        "${EXT_SHADER_INC_DIR}/*.hlsli")
    if(${ZE_PLATFORM_WINDOWS})
        set(SD_APIS "DX11;DX12;VK")
        separate_arguments(SD_FLAGS WINDOWS_COMMAND "${FLAGS}")
    else()
        message(FATAL_ERROR "Unsupporder platform for shader compiling!")
    endif()
endmacro()
 
# Creates commands for shader compilation
#   SD_TYPE = prefix type of shader (VS, GS, PS, CS)
macro(add_shader_type SD_TYPE)
    set(${SD_TYPE}_DIR "${SD_DIR}/${SD_TYPE}")
    file(GLOB_RECURSE ${SD_TYPE}_SRC_LIST "${${SD_TYPE}_DIR}/*.hlsl")
    file(GLOB_RECURSE ${SD_TYPE}_INC_LIST "${${SD_TYPE}_DIR}/*.hlsli")
    
    foreach(API IN LISTS SD_APIS)
        if(${API} STREQUAL "DX11")
            string(TOLOWER "${SD_TYPE}_5_0" ${SD_TYPE}_TYPE_FLAG)
            set(SD_COMPILER "${EXTERNAL_DIR}/fxc.exe")
            set(API_FLAGS "")
            set(SD_EXT "dxbc")
        elseif(${API} STREQUAL "DX12")
            string(TOLOWER "${SD_TYPE}_6_5" ${SD_TYPE}_TYPE_FLAG)
            set(SD_COMPILER "${EXTERNAL_DIR}/dxc.exe")
            if(NOT ${ZE_BUILD_RELEASE})
                set(API_FLAGS "-Qembed_debug")
            else()
                set(API_FLAGS "-O3")
            endif()
            set(SD_EXT "dxil")
        elseif(${API} STREQUAL "VK")
            # https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#introduction
            set(SD_COMPILER "${EXTERNAL_DIR}/dxc.exe")
            set(API_FLAGS "-spirv -fvk-stage-io-order=decl -fvk-use-dx-layout -fvk-use-dx-position-w -fspv-target-env=vulkan1.3 -fspv-flatten-resource-arrays")
            if(${SD_TYPE} STREQUAL "VS" OR ${SD_TYPE} STREQUAL "GS")
                set(API_FLAGS "${API_FLAGS} -fvk-invert-y")
            endif()
            if(NOT ${ZE_BUILD_RELEASE})
                set(API_FLAGS "${API_FLAGS} -Qembed_debug")
            else()
                set(API_FLAGS "${API_FLAGS} -O3")
            endif()
            set(SD_EXT "spv")
        else()
            message(FATAL_ERROR "API <${API}> not supported for shaders!")
        endif()
        
        separate_arguments(API_FLAGS WINDOWS_COMMAND "${API_FLAGS}")
        foreach(SD IN LISTS ${SD_TYPE}_SRC_LIST)
            get_filename_component(SD_NAME ${SD} NAME_WE)
            set(SD_OUT "${SD_CSO_DIR}/${API}/${SD_NAME}.${SD_EXT}")
            list(APPEND SD_LIST ${SD_OUT})
            add_custom_command(OUTPUT ${SD_OUT}
                COMMAND "${SD_COMPILER}" ${SD_FLAGS} ${API_FLAGS} /T ${${SD_TYPE}_TYPE_FLAG} /I "${${SD_TYPE}_DIR}" /I "${SD_INC_DIR}" /D _${API} /D _${SD_TYPE} /Fo "${SD_OUT}" "${SD}"
                MAIN_DEPENDENCY "${SD}"
                DEPENDS "${${SD_TYPE}_INC_LIST}" "${SD_INC_LIST}" "${SD_COMPILER}" VERBATIM)
        endforeach()
    endforeach()
endmacro()

# Create target containing rules for all shaders
#   SD_TARGET = name of target to create
macro(add_shader_target SD_TARGET)
    add_custom_target(${SD_TARGET} ALL DEPENDS ${SD_LIST} VERBATIM)
endmacro()