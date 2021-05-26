include_guard(DIRECTORY)

# Macro for setting external projects build variables
#	PROJECT = prefix of all project variables
#	ADD_BIN_DIR = different path for library file
#	ADD_PDB_DIR = different path for MSVC debug symbols
macro(set_external_cp_cmd_vars PROJECT ADD_BIN_DIR ADD_PDB_DIR)
	set(${PROJECT}_OUT_LIB_NAME "${LIB_PREFIX}${${PROJECT}_TARGET}${LIB_EXT}")
	set(${PROJECT}_OUT_LIB "${EXTERNAL_BIN_DIR}/${${PROJECT}_OUT_LIB_NAME}")

	set(${PROJECT}_CP_CMD "${CMAKE_COMMAND}" -E rename
		"${${PROJECT}_BUILD_DIR}${ADD_BIN_DIR}${${PROJECT}_OUT_LIB_NAME}"
		"${${PROJECT}_OUT_LIB}")

	if(MSVC AND NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
		set(${PROJECT}_CP_CMD "${${PROJECT}_CP_CMD}" &&
			"${CMAKE_COMMAND}" -E rename
				"${${PROJECT}_BUILD_DIR}${ADD_PDB_DIR}CMakeFiles/${${PROJECT}_LIB}.dir/${${PROJECT}_LIB}.pdb"
				"${EXTERNAL_BIN_DIR}/${${PROJECT}_LIB}.pdb")
	endif()
endmacro()

# Macro for loading external projects
#	PROJECT = prefix of all project variables
#	ADD_BIN_DIR = different path for library file
#	ADD_PDB_DIR = different path for MSVC debug symbols
# Set ${PROJECT}_CACHE_ARGS variable to pass options for project
macro(add_external_project PROJECT ADD_BIN_DIR ADD_PDB_DIR)
	set(${PROJECT}_BUILD_DIR "${ZE_BUILD_DIR}/${PROJECT}/")
	set_external_cp_cmd_vars("${PROJECT}" "${ADD_BIN_DIR}" "${ADD_PDB_DIR}")
	if(NOT EXISTS "${${PROJECT}_OUT_LIB}")
		ExternalProject_Add(${${PROJECT}_TARGET}
			SOURCE_DIR ${${PROJECT}_DIR}
			BINARY_DIR ${${PROJECT}_BUILD_DIR}
			STAMP_DIR ${${PROJECT}_BUILD_DIR}
			INSTALL_DIR ${EXTERNAL_BIN_DIR}
			CMAKE_CACHE_ARGS "${${PROJECT}_CACHE_ARGS}"
				"-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
			INSTALL_COMMAND "${${PROJECT}_CP_CMD}")
	elseif(EXISTS ${${PROJECT}_BUILD_DIR})
		file(REMOVE_RECURSE ${${PROJECT}_BUILD_DIR})
	endif()
endmacro()

# Macro for copying external projects inner libraries
#	MAIN_PROJECT = prefix of a project containing libraries
#	PROJECT = prefix of all project variables
#	ADD_BIN_DIR = different path for library file
#	ADD_PDB_DIR = different path for MSVC debug symbols
#	ADD_BUILD_DIR = path to build directory inside MAIN_PROJECT
macro(copy_inner_lib MAIN_PROJECT PROJECT ADD_BIN_DIR ADD_PDB_DIR ADD_BUILD_DIR)
	set(${PROJECT}_BUILD_DIR "${ZE_BUILD_DIR}/${MAIN_PROJECT}/${ADD_BUILD_DIR}")
	set_external_cp_cmd_vars("${PROJECT}" "${ADD_BIN_DIR}" "${ADD_PDB_DIR}" "${ADD_BUILD_DIR}")
	if(NOT EXISTS "${${PROJECT}_OUT_LIB}")
		ExternalProject_Add_Step(${${MAIN_PROJECT}_TARGET} copy${PROJECT}
			DEPENDEES install
			LOG OFF
			COMMAND "${${PROJECT}_CP_CMD}")
	endif()
endmacro()