include_guard(DIRECTORY)

# Macro for setting external projects build variables
#	PROJECT = prefix of all project variables
#	DATA_DIR = directory holding runtime data to copy
#   DATA_PREFIX = prefix pattern for data to copy
#   DATA_SUFIX = sufix pattern for data to copy
#   DATA_OUT_PREFIX = prefix for outputing copied data
macro(copy_runtime_data PROJECT DATA_DIR DATA_PREFIX DATA_SUFIX DATA_OUT_PREFIX)
    set(${PROJECT}_COPY_TARGET "${PROJECT}_DataCopy")
    file(GLOB_RECURSE ${PROJECT}_DATA_LIST RELATIVE "${${PROJECT}_DIR}" "${DATA_DIR}/${DATA_PREFIX}*${DATA_SUFIX}")

    foreach(FILE IN LISTS ${PROJECT}_DATA_LIST)
        string(REPLACE "${DATA_DIR}/" "${ZE_BIN_DIR}/${DATA_OUT_PREFIX}" FILE_OUT ${FILE})
        list(APPEND ${PROJECT}_DATA_OUT_LIST ${FILE_OUT})
        add_custom_command(OUTPUT ${FILE_OUT} POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            ARGS -E copy "${FILE}" "${FILE_OUT}"
            MAIN_DEPENDENCY "${FILE}"
            WORKING_DIRECTORY "${${PROJECT}_DIR}"
            COMMENT "Copy runtime file: ${FILE}" VERBATIM)
    endforeach()
    add_custom_target(${${PROJECT}_COPY_TARGET} DEPENDS ${${PROJECT}_DATA_OUT_LIST} VERBATIM)
endmacro()