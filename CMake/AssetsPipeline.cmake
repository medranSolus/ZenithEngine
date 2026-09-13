include_guard(DIRECTORY)

# Call this macro first to setup required assets directories
#   SRC = source directory where assets will be held (all outputs in the OUT_DIR will be treated as relative to this path)
#	OUT = base destination directory for assets
#   TOOLS = path to the tools to be used by the commands
macro(setup_assets_vars SRC OUT TOOLS)
    set(ASSETS_SRC_DIR "${SRC}")
    set(ASSETS_OUT_DIR "${OUT}")
    set(ASSETS_TOOLS_PATH "${TOOLS}")
    set(ASSETS_LOG_DIR "${ASSETS_OUT_DIR}/Logs")
    set(ASSETS_TEMP_DIR "${ZE_BUILD_DIR}/AssetsTemp")

    set(TOOL_TEXASM "texassemble${EXEC_EXT}")
    set(TOOL_TEXCONV "texconv${EXEC_EXT}")
    set(TOOL_BRDFGEN "BrdfGen${EXEC_EXT}")
    set(TOOL_CUBECONV "CubeConv${EXEC_EXT}")
    set(TOOL_MATPATCH "MatPatch${EXEC_EXT}")
    set(TOOL_MIPGEN "MipGen${EXEC_EXT}")
    set(TOOL_TEXEDIT "TexEdit${EXEC_EXT}")

    set(TEX_FORMAT_COLOR BC7_UNORM)
    set(TEX_FORMAT_HDR BC6H_UF16)
    set(TEX_FORMAT_NORMAL BC5_UNORM)
    set(TEX_FORMAT_SINGLE BC4_UNORM)

    set(ASSETS_OUTPUTS "")
endmacro()

# Macro for finalizing assets generation under single target
#   TARGET_NAME = name of the target for assets generation
macro(add_assets_target TARGET_NAME)
    # Check that all required tools are created first
    set(REQUIRED_TOOLS "${TOOL_TEXASM};${TOOL_TEXCONV};${TOOL_BRDFGEN};${TOOL_CUBECONV};${TOOL_MATPATCH};${TOOL_MIPGEN};${TOOL_TEXEDIT}")

    file(WRITE "${ASSETS_TEMP_DIR}/ToolCheckInline.cmake"
    "set(MISSING_TOOLS \"\")\n"
    "foreach(TOOL IN LISTS TOOL_NAMES)\n"
    "    if(NOT EXISTS \"\${TOOLS_PATH}/\${TOOL}\")\n"
    "       list(APPEND MISSING_TOOLS \"\${TOOL}\")\n"
    "    endif()\n"
    "endforeach()\n"
    "if(MISSING_TOOLS)\n"
    "   string(REPLACE \";\" \", \" MISSING_TOOLS \"\${MISSING_TOOLS}\")\n"
    "   message(FATAL_ERROR \"Missing tools: \${MISSING_TOOLS}! First build the [ZenithTools] target in Release configuration, then run assets preprocessing again.\")\n"
    "endif()\n")
    
    add_custom_command(OUTPUT "${ASSETS_LOG_DIR}/tools_present.stamp"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${ASSETS_LOG_DIR}/build.stamp"
        COMMAND ${CMAKE_COMMAND} -DTOOLS_PATH:STRING=${ASSETS_TOOLS_PATH} -DTOOL_NAMES=${REQUIRED_TOOLS} -P ${ASSETS_TEMP_DIR}/ToolCheckInline.cmake
        COMMAND ${CMAKE_COMMAND} -E touch "${ASSETS_LOG_DIR}/tools_present.stamp"
        COMMENT "Starting assets processing")
    list(PREPEND ASSETS_OUTPUTS "${ASSETS_LOG_DIR}/tools_present.stamp")

    add_custom_target(${TARGET_NAME} COMMENT "Finished assets processing"
        COMMAND ${CMAKE_COMMAND} -E touch "${ASSETS_LOG_DIR}/build.stamp"
        DEPENDS ${ASSETS_OUTPUTS} VERBATIM)
endmacro()

# Macro for copying assets files into a runtime directory
#   SEARCH_PATH = additional path in the SRC_DIR to search for assets
#   COPY_SUFFIX = additional suffix to search for the correct files to copy
macro(copy_assets SEARCH_PATH COPY_EXT)
    file(GLOB_RECURSE COPY_SRC_LIST RELATIVE "${ASSETS_SRC_DIR}" "${ASSETS_SRC_DIR}/${SEARCH_PATH}*${COPY_SUFFIX}")

    foreach(FILE ${COPY_SRC_LIST})
        set(OUT "${ASSETS_OUT_DIR}/${FILE}")

        add_custom_command(OUTPUT "${OUT}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSETS_SRC_DIR}/${FILE}" "${OUT}"
            DEPENDS "${ASSETS_SRC_DIR}/${FILE}"
            COMMENT "Copying asset file: ${FILE}")
            
        list(APPEND ASSETS_OUTPUTS "${OUT}")
    endforeach()
endmacro()

# Macro for generating BRDF LUT from script file
#   JSON_SCRIPT = script file containing description of the desired LUTs
#   OUT_PATH = directory where the generated LUTs will be placed
macro(generate_brdf JSON_SCRIPT OUT_PATH)
    set(JSON_SCRIPT_PATH "${ASSETS_SRC_DIR}/${JSON_SCRIPT}")
    file(READ "${JSON_SCRIPT_PATH}" JSON_RAW)
    string(JSON BRDF_COUNT LENGTH "${JSON_RAW}")
    math(EXPR BRDF_COUNT "${BRDF_COUNT} - 1")
    
    set(LAST_BRDF_OUT "")
    set(BRDF_OUT_LIST "")
    foreach(BRDF_IDX RANGE "${BRDF_COUNT}")
        string(JSON BRDF_DESC GET "${JSON_RAW}" "${BRDF_IDX}")
        string(JSON BRDF_NAME GET "${BRDF_DESC}" "out")
        
        set(LAST_BRDF_OUT "${ASSETS_OUT_DIR}/${OUT_PATH}${BRDF_NAME}")
        list(APPEND BRDF_OUT_LIST "${LAST_BRDF_OUT}")
        list(APPEND ASSETS_OUTPUTS "${LAST_BRDF_OUT}")
    endforeach()
    
    list(POP_BACK BRDF_OUT_LIST)
    math(EXPR BRDF_COUNT "${BRDF_COUNT} + 1")
    add_custom_command(OUTPUT "${LAST_BRDF_OUT}"
        COMMENT "Generating <${BRDF_COUNT}> BRDF LUTs"
        COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_BRDFGEN} --json ${JSON_SCRIPT_PATH} --log-dir ${ASSETS_LOG_DIR}
        WORKING_DIRECTORY "${ASSETS_OUT_DIR}/${OUT_PATH}"
        BYPRODUCTS "${BRDF_OUT_LIST}"
        DEPENDS "${JSON_SCRIPT_PATH}" VERBATIM)
endmacro()

# Utility function for sorting cubemap sources
function(cubemap_comparator A B RESULT)
    string(SUBSTRING "${A}" 1 1 CHAR_A)
    string(SUBSTRING "${B}" 1 1 CHAR_B)
    if("${CHAR_A}" STRLESS "${CHAR_B}")
        set(${RESULT} TRUE PARENT_SCOPE)
    elseif(${CHAR_A} STREQUAL ${CHAR_B})
        string(SUBSTRING "${A}" 0 1 CHAR_A)
        string(SUBSTRING "${B}" 0 1 CHAR_B)
        if("${CHAR_A}" STRGREATER "${CHAR_B}")
            set(${RESULT} TRUE PARENT_SCOPE)
        else()
            set(${RESULT} FALSE PARENT_SCOPE)
        endif()
    else()
        set(${RESULT} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Macro for creating single cubemap files from 6 faces defined as px, nx, py, ny, pz, nz files of arbitrary extension
#   CUBES_PATH = path to master directory containing cubemaps directories
macro(merge_cubemaps CUBES_PATH)
    file(GLOB CUBES_SRC_LIST LIST_DIRECTORIES TRUE RELATIVE "${ASSETS_SRC_DIR}" "${ASSETS_SRC_DIR}/${CUBES_PATH}*")

    foreach(CUBEMAP_DIR ${CUBES_SRC_LIST})
        file(GLOB CUBEMAP_SRC_LIST RELATIVE "${ASSETS_SRC_DIR}/${CUBEMAP_DIR}" "${ASSETS_SRC_DIR}/${CUBEMAP_DIR}/*")

        # Sanity check for correct number of files
        list(LENGTH CUBEMAP_SRC_LIST CUBEMAP_LIST_LEN)
        if (NOT (${CUBEMAP_LIST_LEN} EQUAL 6))
            message(FATAL_ERROR "Incorrect number [${CUBEMAP_LIST_LEN}] of surfaces in the directory: ${CUBEMAP_DIR}!")
        endif()
        # Sort them so that they will be in correct order
        list(SORT CUBEMAP_SRC_LIST COMPARATOR cubemap_comparator)
        string(REPLACE ";" ";${ASSETS_SRC_DIR}/${CUBEMAP_DIR}/" SURFACE_LIST "${CUBEMAP_SRC_LIST}")

        get_filename_component(OUT_FILE "${CUBEMAP_DIR}" NAME)
        set(CUBEMAP_OUT "${ASSETS_OUT_DIR}/${CUBES_PATH}${OUT_FILE}.dds")

        add_custom_command(OUTPUT "${CUBEMAP_OUT}"
            COMMENT "Merging cubemap surfaces: ${CUBES_PATH}${OUT_FILE}"
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_TEXASM} cube -o ${CUBEMAP_OUT} -nologo -y ${CUBEMAP_SRC_LIST}
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_MIPGEN} --filter 4 --src-org-layer --window-size 7 --cores 16 --source ${CUBEMAP_OUT} --log-dir ${ASSETS_LOG_DIR} --log-file ${OUT_FILE}_mipgen.txt
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_TEXCONV} --format ${TEX_FORMAT_COLOR} -o ${ASSETS_OUT_DIR}/${CUBES_PATH} -nologo -y ${CUBEMAP_OUT}
            WORKING_DIRECTORY "${ASSETS_SRC_DIR}/${CUBEMAP_DIR}"
            DEPENDS "${ASSETS_SRC_DIR}/${CUBEMAP_DIR}/${SURFACE_LIST}")
        list(APPEND ASSETS_OUTPUTS "${CUBEMAP_OUT}")
    endforeach()
endmacro()

# Macro for generating skybox textures and environment maps from HDRI images
#   SKYBOX_JSON_SCRIPT = script file containing description of the desired skyboxes along with their mipmaps
#   ENVMAP_JSON_SCRIPT = script file containing description of the desired environment maps
#   SKYBOX_OUT_PATH = directory where the processed skyboxes will be placed
#   ENVMAP_OUT_PATH = directory where the generated environment maps will be placed
macro(process_hdris SKYBOX_JSON_SCRIPT ENVMAP_JSON_SCRIPT SKYBOX_OUT_PATH ENVMAP_OUT_PATH)
    set(SKYBOX_JSON_SCRIPT_PATH "${ASSETS_SRC_DIR}/${SKYBOX_JSON_SCRIPT}")
    set(ENVMAP_JSON_SCRIPT_PATH "${ASSETS_SRC_DIR}/${ENVMAP_JSON_SCRIPT}")
    get_filename_component(HDRI_PATH "${SKYBOX_JSON_SCRIPT}" DIRECTORY)
    set(HDRI_PATH "${ASSETS_SRC_DIR}/${HDRI_PATH}")
    set(SKYBOX_DEST_PATH "${ASSETS_OUT_DIR}/${SKYBOX_OUT_PATH}")
    set(ENVMAP_DEST_PATH "${ASSETS_OUT_DIR}/${ENVMAP_OUT_PATH}")

    # Step 1: generate correct skybox cubemaps with mips
    file(READ "${SKYBOX_JSON_SCRIPT_PATH}" JSON_RAW)
    string(JSON JOB_COUNT LENGTH "${JSON_RAW}")
    math(EXPR JOB_COUNT "${JOB_COUNT} - 1")
    
    set(SKYBOX_OUT_LIST "")
    set(SKYBOX_JSON_TEMP_DIR "${ASSETS_TEMP_DIR}/SkyboxJsons")
    set(SKYBOX_WAIT_LIST "")
    foreach(SKYBOX_IDX RANGE "${JOB_COUNT}")
        string(JSON SKYBOX_DESC GET "${JSON_RAW}" "${SKYBOX_IDX}")
        
        string(JSON TEXEDIT_JOB GET "${SKYBOX_DESC}" "tex-edit")
        string(JSON MIPGEN_JOB GET "${SKYBOX_DESC}" "mip-gen")
        string(JSON HDRI_NAME GET "${TEXEDIT_JOB}" "source")
        string(JSON SKYBOX_NAME GET "${TEXEDIT_JOB}" "out")
        get_filename_component(SKYBOX_NAME_NO_EXT "${SKYBOX_NAME}" NAME_WE)

        set(TEXEDIT_JSON "${SKYBOX_JSON_TEMP_DIR}/${SKYBOX_NAME_NO_EXT}_texedit.json")
        set(MIPGEN_JSON "${SKYBOX_JSON_TEMP_DIR}/${SKYBOX_NAME_NO_EXT}_mipgen.json")
        file(WRITE "${TEXEDIT_JSON}" "${TEXEDIT_JOB}")
        file(WRITE "${MIPGEN_JSON}" "${MIPGEN_JOB}")

        set(SKYBOX_OUT "${SKYBOX_DEST_PATH}${SKYBOX_NAME}")
        set(SKYBOX_WAIT "${ASSETS_LOG_DIR}/${SKYBOX_NAME_NO_EXT}_mipgen.txt")
        add_custom_command(OUTPUT "${SKYBOX_WAIT}"
            COMMENT "Generating skybox from HDRI: ${SKYBOX_NAME}"
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_TEXEDIT} --json ${TEXEDIT_JSON} --source-dir ${HDRI_PATH} --out-dir ${SKYBOX_DEST_PATH} --log-dir ${ASSETS_LOG_DIR} --log-file ${SKYBOX_NAME_NO_EXT}_texedit.txt
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_MIPGEN} --json ${MIPGEN_JSON} --source-dir ${SKYBOX_DEST_PATH} --out-dir ${SKYBOX_DEST_PATH} --log-dir ${ASSETS_LOG_DIR} --log-file ${SKYBOX_NAME_NO_EXT}_mipgen.txt
            DEPENDS "${SKYBOX_JSON_SCRIPT_PATH};${HDRI_PATH}/${HDRI_NAME}" VERBATIM)
        list(APPEND SKYBOX_OUT_LIST "${SKYBOX_OUT}")
        list(APPEND ASSETS_OUTPUTS "${SKYBOX_OUT}")
        list(APPEND SKYBOX_WAIT_LIST "${SKYBOX_WAIT}")
    endforeach()
        
    # Step 2: convolute HDRI skyboxes
    file(READ "${ENVMAP_JSON_SCRIPT_PATH}" JSON_RAW)
    string(JSON JOB_COUNT LENGTH "${JSON_RAW}")
    math(EXPR JOB_COUNT "${JOB_COUNT} - 1")
    
    set(LAST_ENVMAP_OUT "")
    set(ENVMAP_OUT_LIST "")
    foreach(JOB_IDX RANGE "${JOB_COUNT}")
        string(JSON ENVMAP_DESC GET "${JSON_RAW}" "${JOB_IDX}")
        string(JSON ENVMAP_NAME GET "${ENVMAP_DESC}" "out")
        
        set(LAST_ENVMAP_OUT "${ENVMAP_DEST_PATH}${ENVMAP_NAME}")
        list(APPEND ENVMAP_OUT_LIST "${LAST_ENVMAP_OUT}")
        list(APPEND ASSETS_OUTPUTS "${LAST_ENVMAP_OUT}")
    endforeach()
    
    list(POP_BACK ENVMAP_OUT_LIST)
    math(EXPR JOB_COUNT "${JOB_COUNT} + 1")
    set(CONV_FINISH_FILE "${ASSETS_LOG_DIR}/hdri_process_cubeconv.stamp")
    add_custom_command(OUTPUT "${CONV_FINISH_FILE}"
        COMMENT "Convoluting <${JOB_COUNT}> light maps"
        COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_CUBECONV} --json ${ENVMAP_JSON_SCRIPT_PATH} --source-dir ${SKYBOX_DEST_PATH} --out-dir ${ENVMAP_DEST_PATH} --log-dir ${ASSETS_LOG_DIR}
        COMMAND ${CMAKE_COMMAND} -E touch "${CONV_FINISH_FILE}"
        DEPENDS "${ENVMAP_JSON_SCRIPT_PATH};${SKYBOX_WAIT_LIST}" VERBATIM)
    list(APPEND ENVMAP_OUT_LIST "${LAST_ENVMAP_OUT}")
    
    # Step 3: compress results
    foreach(ENVMAP ${ENVMAP_OUT_LIST})        
        get_filename_component(ENVMAP_NAME "${ENVMAP}" NAME)
        add_custom_command(OUTPUT "${ENVMAP}"
            COMMENT "Compressing light map: ${ENVMAP_NAME}"
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_TEXCONV} --format ${TEX_FORMAT_HDR} -o ${ENVMAP_DEST_PATH} -nologo -y ${ENVMAP}
            DEPENDS "${CONV_FINISH_FILE}" VERBATIM)
    endforeach()

    foreach(SKYBOX ${SKYBOX_OUT_LIST})
        get_filename_component(SKYBOX_NAME "${SKYBOX}" NAME)
        add_custom_command(OUTPUT "${SKYBOX}"
            COMMENT "Compressing skybox: ${SKYBOX_NAME}"
            COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_TEXCONV} --format ${TEX_FORMAT_COLOR} -o ${SKYBOX_DEST_PATH} -nologo -y ${SKYBOX}
            DEPENDS "${CONV_FINISH_FILE}" VERBATIM)
    endforeach()
endmacro()

macro(process_models MODELS_PATH)
    file(GLOB MODELS_LIST LIST_DIRECTORIES TRUE RELATIVE "${ASSETS_SRC_DIR}/${MODELS_PATH}" "${ASSETS_SRC_DIR}/${MODELS_PATH}*")

    foreach(MODEL_DIR ${MODELS_LIST})
        set(MODEL_DIR_PATH "${ASSETS_SRC_DIR}/${MODELS_PATH}${MODEL_DIR}")
        set(MODEL_OUT_PATH "${ASSETS_OUT_DIR}/${MODELS_PATH}${MODEL_DIR}")
        file(GLOB SCRIPTS_LIST RELATIVE "${MODEL_DIR_PATH}" "${MODEL_DIR_PATH}/*.json")

        foreach(MODEL_SCRIPT ${SCRIPTS_LIST})
            set(MODEL_SCRIPT_PATH "${MODEL_DIR_PATH}/${MODEL_SCRIPT}")
            file(READ "${MODEL_SCRIPT_PATH}" JSON_RAW)
            string(JSON JOB_COUNT LENGTH "${JSON_RAW}")
            math(EXPR JOB_COUNT "${JOB_COUNT} - 1")

            if ("${MODEL_SCRIPT}" STREQUAL "model_copy.json")
                # Simple copy of the source files
                foreach(COPY_IDX RANGE "${JOB_COUNT}")
                    string(JSON COPY_SRC GET "${JSON_RAW}" "${COPY_IDX}")

                    set(COPY_OUT "${MODEL_OUT_PATH}/${COPY_SRC}")
                    add_custom_command(OUTPUT "${COPY_OUT}"
                        COMMENT "Copying model file: ${MODEL_DIR}/${COPY_SRC}"
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MODEL_DIR_PATH}/${COPY_SRC}" "${COPY_OUT}"
                        DEPENDS "${MODEL_SCRIPT_PATH};${MODEL_DIR_PATH}/${COPY_SRC}")
                    list(APPEND ASSETS_OUTPUTS "${COPY_OUT}")
                endforeach()
            elseif("${MODEL_SCRIPT}" STREQUAL "model_mipgen.json")
                # Generate mipmaps for material textures
                set(TEX_SRC_LIST "")
                set(MIPMAP_OUT_LIST "")
                foreach(MIPMAP_IDX RANGE "${JOB_COUNT}")
                    string(JSON MIPMAP_DESC GET "${JSON_RAW}" "${MIPMAP_IDX}")
                    string(JSON MIPMAP_OUT GET "${MIPMAP_DESC}" "out")
                    string(JSON TEX_SRC GET "${MIPMAP_DESC}" "source")

                    set(LAST_MIPMAP_OUT "${MODEL_OUT_PATH}/${MIPMAP_OUT}")
                    list(APPEND ASSETS_OUTPUTS "${LAST_MIPMAP_OUT}")
                    list(APPEND MIPMAP_OUT_LIST "${LAST_MIPMAP_OUT}")
                    list(APPEND TEX_SRC_LIST "${ASSETS_SRC_DIR}/${MODELS_PATH}${MODEL_DIR}/${TEX_SRC}")
                endforeach()
                
                list(POP_BACK MIPMAP_OUT_LIST)
                math(EXPR JOB_COUNT "${JOB_COUNT} + 1")
                add_custom_command(OUTPUT "${LAST_MIPMAP_OUT}"
                    COMMENT "Generating <${JOB_COUNT}> mipmaps for model: ${MODEL_DIR}"
                    COMMAND ${ASSETS_TOOLS_PATH}/${TOOL_MIPGEN} --json ${MODEL_SCRIPT_PATH} --source-dir ${MODEL_DIR_PATH} --out-dir ${MODEL_OUT_PATH} --log-dir ${ASSETS_LOG_DIR} --log-file ${MODEL_DIR}_mipgen.txt
                    BYPRODUCTS "${MIPMAP_OUT_LIST}"
                    DEPENDS "${MODEL_SCRIPT_PATH};${TEX_SRC_LIST}" VERBATIM)
            elseif("${MODEL_SCRIPT}" STREQUAL "model_info.json")
                # Perform various transformations on model files
            else()
                message("Ignoring unknown assets json file: ${MODEL_DIR}/${MODEL_SCRIPT}")
            endif()
        endforeach()
    endforeach()
endmacro()