include_guard(DIRECTORY)

# Macro for setting external projects build variables
#	PROJECT = prefix of all project variables
#	ADD_BIN_DIR = different path for library file
#	ADD_PDB_DIR_PRE = different path for MSVC debug symbols for "CMakeFiles/PROJECT_LIb.dir/" location
#	ADD_PDB_DIR_POST = different path for MSVC debug symbols inside "CMakeFiles/PROJECT_LIb.dir/" location
macro(set_external_cp_cmd_vars PROJECT ADD_BIN_DIR ADD_PDB_DIR_PRE ADD_PDB_DIR_POST)
	set(${PROJECT}_OUT_LIB_NAME "${LIB_PREFIX}${${PROJECT}_TARGET}${LIB_EXT}")
	set(${PROJECT}_OUT_LIB "${EXTERNAL_BIN_DIR}/${${PROJECT}_OUT_LIB_NAME}")

	set(${PROJECT}_CP_CMD "${CMAKE_COMMAND}" -E rename
		"${${PROJECT}_BUILD_DIR}${ADD_BIN_DIR}${${PROJECT}_OUT_LIB_NAME}"
		"${${PROJECT}_OUT_LIB}")

	if(${ZE_COMPILER_MSVC} AND ${ZE_EXTERNAL_BUILD_DEBUG_INFO})
		set(${PROJECT}_CP_CMD "${${PROJECT}_CP_CMD}" &&
			"${CMAKE_COMMAND}" -E rename
				"${${PROJECT}_BUILD_DIR}${ADD_PDB_DIR_PRE}CMakeFiles/${${PROJECT}_LIB}.dir/${ADD_PDB_DIR_POST}${${PROJECT}_LIB}.pdb"
				"${EXTERNAL_BIN_DIR}/${${PROJECT}_LIB}.pdb")
	endif()
endmacro()

# Macro for loading external projects
#	PROJECT = prefix of all project variables
#	ADD_BIN_DIR = different path for library file
#	ADD_PDB_DIR_PRE = different path for MSVC debug symbols for "CMakeFiles/PROJECT_LIb.dir/" location
#	ADD_PDB_DIR_POST = different path for MSVC debug symbols inside "CMakeFiles/PROJECT_LIb.dir/" location
#	DEP_PROJECTS = prefixes of projects on which this eternal project depends (semicolon separated)
# Set ${PROJECT}_CACHE_ARGS variable to pass options for project
macro(add_external_project PROJECT ADD_BIN_DIR ADD_PDB_DIR_PRE ADD_PDB_DIR_POST DEP_PROJECTS)
	set(${PROJECT}_BUILD_DIR "${ZE_BUILD_DIR}/${PROJECT}/")
	set_external_cp_cmd_vars("${PROJECT}" "${ADD_BIN_DIR}" "${ADD_PDB_DIR_PRE}" "${ADD_PDB_DIR_POST}")
	if(NOT EXISTS "${${PROJECT}_OUT_LIB}")
		foreach(DEP_PROJECT IN ITEMS ${DEP_PROJECTS})
			list(APPEND ${PROJECT}_DEPENDENCY "${${DEP_PROJECT}_TARGET}")
		endforeach()
		ExternalProject_Add(${${PROJECT}_TARGET}
			SOURCE_DIR ${${PROJECT}_DIR}
			BINARY_DIR ${${PROJECT}_BUILD_DIR}
			STAMP_DIR ${${PROJECT}_BUILD_DIR}
			INSTALL_DIR ${EXTERNAL_BIN_DIR}
			DEPENDS ${${PROJECT}_DEPENDENCY}
			CMAKE_CACHE_ARGS "${${PROJECT}_CACHE_ARGS}"
				"-DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=${CMAKE_MSVC_RUNTIME_LIBRARY}"
				"-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT:STRING=${CMAKE_MSVC_DEBUG_INFORMATION_FORMAT}"
				"-DCMAKE_BUILD_TYPE:STRING=${ZE_EXTERNAL_BUILD_TYPE}"
				"-DCMAKE_LIBRARY_PATH:STRING=${EXTERNAL_BIN_DIR}"
				"-DCMAKE_INSTALL_PREFIX_PATH:STRING=${EXTERNAL_BIN_DIR}"
				"-DCMAKE_FIND_PACKAGE_HINTS:STRING=${EXTERNAL_BIN_DIR}"
				"-DCMAKE_MODULE_PATH:STRING=${CMAKE_MODULE_PATH}"
			INSTALL_COMMAND "${${PROJECT}_CP_CMD}")
	else()
		add_library(${${PROJECT}_TARGET} STATIC IMPORTED GLOBAL)
		set_target_properties(${${PROJECT}_TARGET} PROPERTIES
			IMPORTED_LOCATION "${${PROJECT}_OUT_LIB}")
		if(EXISTS ${${PROJECT}_BUILD_DIR})
			file(REMOVE_RECURSE ${${PROJECT}_BUILD_DIR})
		endif()
	endif()
endmacro()

# Macro for copying external projects inner libraries
#	PROJECT = prefix of a project containing libraries
#	STEP_NAME = unique name of step to be performed in given project
#	ADD_BIN_DIR = different path for library file
#	LIB_NAME = library to be copied
macro(copy_inner_lib PROJECT STEP_NAME ADD_BIN_DIR LIB_NAME)
	if(NOT EXISTS "${EXTERNAL_BIN_DIR}/${LIB_NAME}")
		ExternalProject_Add_Step(${${PROJECT}_TARGET} copy${STEP_NAME}
			DEPENDEES install
			LOG OFF
			COMMAND "${CMAKE_COMMAND}" -E copy
			"${${PROJECT}_BUILD_DIR}${ADD_BIN_DIR}${LIB_NAME}"
			"${EXTERNAL_BIN_DIR}")
	endif()
endmacro()