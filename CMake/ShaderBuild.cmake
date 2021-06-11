include_guard(DIRECTORY)

# Order of macros calls:
#   setup_shader - for creation of variables
#   add_shader_type - multiple calls for each wanted shader type
#   add_shader_target - final call wrapping all previous rules

# Setups variables needed by other shader macros
#   SHADER_DIR = directory containing "Shader" folder with all shader code
#       (each shader type in directory named: VS, GS, PS or CS)
#   BIN_DIR = binary directory
#   FXC = path to shader compiler
#   FLAGS = flags used for compilation
macro(setup_shader BIN_DIR SHADER_DIR FXC FLAGS)
    set(SD_CSO_DIR "${BIN_DIR}/Shaders")
    set(SD_DIR "${SHADER_DIR}/Shader")
    set(SD_INC_DIR "${SD_DIR}/Common")
    file(GLOB_RECURSE SD_INC_LIST "${SD_INC_DIR}/*.hlsli")
    set(SD_FXC ${FXC})
    separate_arguments(SD_FLAGS WINDOWS_COMMAND "${FLAGS}")
endmacro()
 
# Creates commands for shader compilation
#   SD_TYPE = prefix type of shader (VS, GS, PS, CS)
macro(add_shader_type SD_TYPE)
    set(${SD_TYPE}_DIR "${SD_DIR}/${SD_TYPE}")
    file(GLOB_RECURSE ${SD_TYPE}_SRC_LIST "${${SD_TYPE}_DIR}/*.hlsl")
    file(GLOB_RECURSE ${SD_TYPE}_INC_LIST "${${SD_TYPE}_DIR}/*.hlsli")

    string(TOLOWER "${SD_TYPE}_5_0" ${SD_TYPE}_TYPE_FLAG)
    foreach(SD IN LISTS ${SD_TYPE}_SRC_LIST)
        get_filename_component(SD_NAME ${SD} NAME_WE)
        set(SD_OUT "${SD_CSO_DIR}/${SD_NAME}.cso")
        list(APPEND SD_LIST ${SD_OUT})
        add_custom_command(OUTPUT ${SD_OUT}
            COMMAND "${SD_FXC}" ${SD_FLAGS} /T ${${SD_TYPE}_TYPE_FLAG} /I "${${SD_TYPE}_DIR}" /I "${SD_INC_DIR}" /D _${SD_TYPE} /Fo "${SD_OUT}" "${SD}"
            MAIN_DEPENDENCY "${SD}"
            DEPENDS "${${SD_TYPE}_INC_LIST}" "${SD_INC_LIST}" VERBATIM)
    endforeach()
endmacro()

# Create target containing rules for all shaders
#   SD_TARGET = name of target to create
macro(add_shader_target SD_TARGET)
    add_custom_target(${SD_TARGET} ALL DEPENDS ${SD_LIST} VERBATIM)
endmacro()