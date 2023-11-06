include_guard(DIRECTORY)

# Order of macros calls:
#   setup_shader - for creation of variables
#   add_shader_permutation - add any required shader permutations
#   add_shader_type - multiple calls for each wanted shader type
#   add_shader_target - final call wrapping all previous rules

# Setups variables needed by other shader macros
#   SHADER_DIR = directory containing "Shader" folder with all shader code
#       (each shader type in directory named: VS, GS, PS or CS)
#   BIN_DIR = binary directory
#   FLAGS = flags used for compilation
macro(setup_shader BIN_DIR SHADER_DIR FLAGS)
    set(SD_LIST "")
    set(SD_DIR_TARGET "GenerateShaderOutDir")
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
    foreach(API IN LISTS SD_APIS)
        file(MAKE_DIRECTORY "${SD_CSO_DIR}/${API}")
    endforeach()
endmacro()
 
# Add permutation to given shader
#   SHADER = name of the shader without extension
#   PERMUTATION = permutation option in format "DEFINE_OPTION:SUFFIX" or "DEFINE_OPT1_1,DEFINE_OPT1_2=VAL:SUFFIX1|DEFINE_OPT2:SUFFIX2" when multi-value permutation is needed
# Every given permutation will result in passed define into shader with additional suffix added to name
macro(add_shader_permutation SHADER PERMUTATION)
    list(APPEND ${SHADER}_PERMUTATIONS "${PERMUTATION}")
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
            string(TOLOWER "${SD_TYPE}_6_6" ${SD_TYPE}_TYPE_FLAG)
            set(SD_COMPILER "${EXTERNAL_DIR}/dxc.exe")
            set(API_FLAGS "-enable-16bit-types -DFFX_HLSL_6_2=1")
            if(NOT ${ZE_BUILD_RELEASE})
                set(API_FLAGS "${API_FLAGS} -Qembed_debug")
            else()
                set(API_FLAGS "${API_FLAGS} -O3")
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
            _add_shader_compile_command("${SD_COMPILER}" "${SD}" "${SD_OUT}" "${SD_NAME}.${SD_EXT}" "${SD_FLAGS};${API_FLAGS}" "${${SD_TYPE}_TYPE_FLAG}" "${SD_TYPE}" "${${SD_TYPE}_DIR}" "${SD_INC_DIR}" "${${SD_TYPE}_INC_LIST}" "${SD_INC_LIST}" "${API}")
            
            # Flatten whole original list of permutations into for correct mask indexing
            string(REPLACE "|" ";" ${SD_NAME}_ORIGINAL_PERMUTATIONS "${${SD_NAME}_PERMUTATIONS}")
            _compile_shader_permutations("${SD_CSO_DIR}/${API}" "${SD_NAME}" "${SD_EXT}" "${${SD_NAME}_ORIGINAL_PERMUTATIONS}" "${SD_NAME}_" "${${SD_NAME}_PERMUTATIONS}" 0 "" FALSE
                "${SD_COMPILER}" "${SD}" "${SD_FLAGS};${API_FLAGS}" "${${SD_TYPE}_TYPE_FLAG}" "${SD_TYPE}" "${${SD_TYPE}_DIR}" "${SD_INC_DIR}" "${${SD_TYPE}_INC_LIST}" "${SD_INC_LIST}" "${API}")
        endforeach()
    endforeach()
endmacro()

# Create target containing rules for all shaders
#   SD_TARGET = name of target to create
macro(add_shader_target SD_TARGET)
    add_custom_target(${SD_TARGET} DEPENDS ${SD_LIST} VERBATIM)
endmacro()

# Internal function for compiling shader
function(_add_shader_compile_command SD_COMPILER SD SD_OUT SD_TARGET SD_FLAGS SHADER_MODEL SD_TYPE SD_TYPE_INC_DIR SD_INC_DIR SD_TYPE_INC_LIST SD_INC_LIST API)
    set(SD_LIST "${SD_LIST};${SD_TARGET}" PARENT_SCOPE)
    add_custom_command(OUTPUT "${SD_OUT}"
        COMMAND "${SD_COMPILER}"
        ARGS ${SD_FLAGS} /T ${SHADER_MODEL} /I "${SD_TYPE_INC_DIR}" /I "${SD_INC_DIR}" /D _${API} /D _${SD_TYPE} /Fo "${SD_OUT}" "${SD}"
        DEPENDS "${SD}" "${SD_TYPE_INC_LIST}" "${SD_INC_LIST}"
        COMMENT "Compiling ${API} shader: ${SD_TARGET}" VERBATIM)
    add_custom_target("${SD_TARGET}" ALL
        DEPENDS "${SD_COMPILER}"
        SOURCES "${SD_OUT}" VERBATIM)
endfunction()

# Internal function for compiling shader permutations
function(_compile_shader_permutations OUT_DIR SD_NAME SD_EXT PERMUTATIONS CURRENT_NAME CURRENT_PERMUTATIONS CURRENT_MASK CURRENT_FLAGS INNER_CALL
    SD_COMPILER SD SD_FLAGS SHADER_MODEL SD_TYPE SD_TYPE_INC_DIR SD_INC_DIR SD_TYPE_INC_LIST SD_INC_LIST API)
    # Only traverse when there are any perumutations
    list(LENGTH PERMUTATIONS LIST_LEN)
    if(NOT (${LIST_LEN} EQUAL 0))
        # Don't add new permutation on first call since it's default one
        if(${INNER_CALL})
            set(PERM_OUT "${OUT_DIR}/${CURRENT_NAME}.${SD_EXT}")
            _add_shader_compile_command("${SD_COMPILER}" "${SD}" "${PERM_OUT}" "${CURRENT_NAME}.${SD_EXT}" "${SD_FLAGS}${CURRENT_FLAGS}" "${SHADER_MODEL}" "${SD_TYPE}" "${SD_TYPE_INC_DIR}" "${SD_INC_DIR}" "${SD_TYPE_INC_LIST}" "${SD_INC_LIST}" "${API}")
        endif()
        # Go through all permutation variants
        foreach(PERM IN LISTS CURRENT_PERMUTATIONS)
            # Copy list before passing into deeper level and remove current permutation
            set(INNER_PERM ${CURRENT_PERMUTATIONS})
            list(REMOVE_ITEM INNER_PERM "${PERM}")
            
            # Go over every mutli-value permutation (or just one if not present)
            string(REPLACE "|" ";" PERM_LIST "${PERM}")
            list(LENGTH PERM_LIST PERM_LIST_LEN)
            foreach(PERM_ELEMENT IN LISTS PERM_LIST)
                # Only proceed when there is correct formatting in the permutation
                string(FIND "${PERM_ELEMENT}" ":" POS REVERSE)
                if (NOT (${POS} EQUAL -1))
                    # Set mask for visited permutations based on position inside the original list
                    list(FIND PERMUTATIONS "${PERM_ELEMENT}" CURRENT_POS)
                    math(EXPR NEW_MASK "${CURRENT_MASK} | (1 << ${CURRENT_POS})")
                    get_source_file_property(VISITED "${SD}" "${NEW_MASK}${SD_EXT}")
                    if(${VISITED} EQUAL NOTFOUND)
                        set_source_files_properties("${SD}" PROPERTIES "${NEW_MASK}${SD_EXT}" FALSE)
                    endif()

                    # Only visit when given permutation has not been found yet (ignore duplicates)
                    if(NOT ${VISITED})
                        set_source_files_properties("${SD}" PROPERTIES "${NEW_MASK}${SD_EXT}" TRUE)

                        # Get shader define part and resulting suffix
                        string(SUBSTRING "${PERM_ELEMENT}" 0 "${POS}" PERM_DEFINE)
                        string(REPLACE "," ";/D" PERM_DEFINE_LIST "${PERM_DEFINE}")
                        math(EXPR POS "${POS} + 1")
                        string(SUBSTRING "${PERM_ELEMENT}" "${POS}" -1 PERM_SUFFIX)

                        # Process other permutations
                        _compile_shader_permutations("${OUT_DIR}" "${SD_NAME}" "${SD_EXT}" "${PERMUTATIONS}" "${CURRENT_NAME}${PERM_SUFFIX}" "${INNER_PERM}" "${NEW_MASK}" "${CURRENT_FLAGS};/D${PERM_DEFINE_LIST}" TRUE
                            "${SD_COMPILER}" "${SD}" "${SD_FLAGS}" "${SHADER_MODEL}" "${SD_TYPE}" "${SD_TYPE_INC_DIR}" "${SD_INC_DIR}" "${SD_TYPE_INC_LIST}" "${SD_INC_LIST}" "${API}")
                    endif()
                endif()
            endforeach()
        endforeach()
    endif()
endfunction()