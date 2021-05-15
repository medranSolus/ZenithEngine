cmake_minimum_required(VERSION 3.19)
include(ExternalProject)

######### PROJECT CONFIGURATION #########

set(PROJECT_NAME "Zenith Engine")
project(${PROJECT_NAME} VERSION 0.2)

# Project options
set(ZE_BIN_DIR "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for project data (shaders, resources, executables)")
option(ZE_MODEL_LOADING "Enable model loading module of engine" ON)
option(ZE_BUILD_TOOL "Enable building of command line utility tool" ON)


################ TARGETS ################

set(TOOL_TARGET "ZEdit")

# Libraries
set(COMMON_TARGET "common")
set(ENGINE_TARGET "zenith")

# External libraries
set(ZLIB_LIB "zlibstatic")
set(ASSIMP_LIB "assimp")
set(DXTEX_LIB "DirectXTex")
set(FTYPE_LIB "freetype")

# Inject debug suffix if library use it
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(ZLIB_TARGET "${ZLIB_LIB}d")
	set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mtd")
	set(FTYPE_TARGET "${FTYPE_LIB}d")
else()
	set(ZLIB_TARGET "${ZLIB_LIB}")
	set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mt")
	set(FTYPE_TARGET "${FTYPE_LIB}")
endif()
set(DXTEX_TARGET "${DXTEX_LIB}")


########### MAIN DIRECTORIES ############

set(COMMON_DIR "${PROJECT_SOURCE_DIR}/Common")
set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/Engine")
set(TOOL_DIR "${PROJECT_SOURCE_DIR}/EditTool")
set(BUILD_DIR "${PROJECT_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}")
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(EXTERNAL_BIN_DIR "${EXTERNAL_DIR}/Bin/${CMAKE_BUILD_TYPE}")

# External
set(ASSIMP_DIR "${EXTERNAL_DIR}/Assimp")
set(DXMATH_DIR "${EXTERNAL_DIR}/DirectXMath")
set(DXTEX_DIR "${EXTERNAL_DIR}/DirectXTex")
set(FTYPE_DIR "${EXTERNAL_DIR}/FreeType")
set(IMGUI_DIR "${EXTERNAL_DIR}/ImGui")


########## INCLUDE DIRECTORIES ##########

set(COMMON_INC_DIR "${COMMON_DIR}/Include")
set(ENGINE_INC_DIR "${ENGINE_DIR}/Include")

# External
set(EXTERNAL_CONFIG_INC_DIR "${EXTERNAL_BIN_DIR}")
set(ASSIMP_INC_DIR "${ASSIMP_DIR}/include")
set(DXMATH_INC_DIR "${DXMATH_DIR}/Inc")
set(DXTEX_INC_DIR "${DXTEX_DIR}/DirectXTex")
set(FTYPE_INC_DIR "${FTYPE_DIR}/include")
set(IMGUI_INC_DIR "${IMGUI_DIR}")
set(JSON_INC_DIR ${EXTERNAL_DIR})


######### COMPILE CONFIGURATION #########

set(ZE_CXX_STD cxx_std_20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
link_directories(${EXTERNAL_BIN_DIR})

# Compiler specific lags (remove 4250 later when ECS system created)
if(MSVC)
	# /MP not compatibile with /Yc (pch), option not found on gcc/clang
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /w34265 /wd4250 /sdl /MP /GF /fp:fast /permissive- /Zc:inline /Zc:rvalueCast /nologo /DYNAMICBASE")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /JMC /Od /Oy- /Zi /RTC1")
	set(COMPILER_REL_CXX_FLAGS "/MD /Ot /Oi /Oy /GL /GS /Gy")
else()
	# GCC asumed although they should work on Clang too
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpessimizing-move -Wredundant-move -W4265 -Wno-4250 -pedantic -ffast-math")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
	set(COMPILER_REL_CXX_FLAGS "-O3 -flto -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=2")
endif()
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

# General flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DZE_NO_DEBUG=0 -D_ZE_MODE_DEBUG")
set(COMPILER_REL_CXX_FLAGS "${COMPILER_REL_CXX_FLAGS} -DZE_NO_DEBUG=1")
set(CMAKE_CXX_FLAGS_RELEASE " ${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_REL_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMPILER_REL_CXX_FLAGS}")

# Shader flags
set(SHADER_FLAGS /nologo /E main /Ges /Zpc)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(SHADER_FLAGS ${SHADER_FLAGS} /Od /Zi)
endif()


############## SUBPROJECTS ##############

add_subdirectory(${COMMON_DIR})
add_subdirectory(${ENGINE_DIR})
if(ZE_BUILD_TOOL)
	add_subdirectory(${TOOL_DIR})
endif()


########### EXTERNAL PROJECTS ###########

if(WIN32)
	set(LIB_EXT ".lib")
	set(LIB_PREFIX "")
else()
	set(LIB_EXT ".a")
	set(LIB_PREFIX "lib")
endif()

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
macro(add_external_project PROJECT ADD_BIN_DIR ADD_PDB_DIR)
	set(${PROJECT}_BUILD_DIR "${BUILD_DIR}/${PROJECT}/")
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
	set(${PROJECT}_BUILD_DIR "${BUILD_DIR}/${MAIN_PROJECT}/${ADD_BUILD_DIR}")
	set_external_cp_cmd_vars("${PROJECT}" "${ADD_BIN_DIR}" "${ADD_PDB_DIR}" "${ADD_BUILD_DIR}")
	if(NOT EXISTS "${${PROJECT}_OUT_LIB}")
		ExternalProject_Add_Step(${${MAIN_PROJECT}_TARGET} copy${PROJECT}
			DEPENDEES install
			LOG OFF
			COMMAND "${${PROJECT}_CP_CMD}")
	endif()
endmacro()

# Assimp
if(${ZE_MODEL_LOADING})
	# No exports set
	set(ASSIMP_CACHE_ARGS "-DBUILD_SHARED_LIBS:BOOL=OFF"
		"-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF"
		"-DASSIMP_BUILD_TESTS:BOOL=OFF"
		"-DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF"
		"-DINJECT_DEBUG_POSTFIX:BOOL=OFF"
		"-DASSIMP_INSTALL:BOOL=OFF"
		"-DASSIMP_BUILD_ZLIB :BOOL=ON"
		"-DASSIMP_INSTALL_PDB:BOOL=OFF"
		"-DASSIMP_NO_EXPORT:BOOL=OFF")
	add_external_project(ASSIMP "lib/" "code/")
	copy_inner_lib(ASSIMP ZLIB "" "" "contrib/zlib/")
	if(NOT EXISTS "${ASSIMP_OUT_LIB}")
		ExternalProject_Add_Step(${ASSIMP_TARGET} headerConfig
			DEPENDEES install
			LOG OFF
			COMMAND "${CMAKE_COMMAND}" -E copy_directory
			"${ASSIMP_BUILD_DIR}/include"
			"${EXTERNAL_BIN_DIR}")
	endif()
endif()

 # DirectXTex
set(DXTEX_CACHE_ARGS "-DBUILD_TOOLS:BOOL=OFF"
	"-DBUILD_DX11:BOOL=OFF"
	"-DBUILD_DX12:BOOL=OFF")
add_external_project(DXTEX "bin/CMake/" "")

# FreeType
set(FTYPE_CACHE_ARGS "-DCMAKE_DISABLE_FIND_PACKAGE_ZLIB:BOOL=ON"
	"-DCMAKE_DISABLE_FIND_PACKAGE_BZip2:BOOL=ON"
	"-DCMAKE_DISABLE_FIND_PACKAGE_PNG:BOOL=ON"
	"-DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz:BOOL=ON"
	"-DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec:BOOL=ON")
add_external_project(FTYPE "" "")