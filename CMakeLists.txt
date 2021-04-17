cmake_minimum_required(VERSION 3.19)
include(ExternalProject)

######### PROJECT CONFIGURATION #########

set(PROJECT_NAME "Horus Engine")
project(${PROJECT_NAME} VERSION 0.2)

# Project options
option(HE_MODEL_LOADING "Enable model loading module of engine" ON)
option(HE_BUILD_TOOL "Enable building of command line utility tool" ON)


################ TARGETS ################

set(TOOL_TARGET "HEdit")

# Libraries
set(COMMON_TARGET "Common")
set(ENGINE_TARGET "Horus")

# External libraries
set(ASSIMP_LIB "assimp")
set(DXTEX_LIB "DirectXTex")
set(FTYPE_LIB "freetype")

# Inject debug suffix if library use it
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mtd")
	set(FTYPE_TARGET "${FTYPE_LIB}d")
else()
	set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mt")
	set(FTYPE_TARGET "${FTYPE_LIB}")
endif()
set(DXTEX_TARGET "${DXTEX_LIB}")


########### MAIN DIRECTORIES ############

set(COMMON_DIR "${PROJECT_SOURCE_DIR}/Common")
set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/HorusEngine")
set(TOOL_DIR "${PROJECT_SOURCE_DIR}/EditTool")
set(BIN_DIR "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}")
set(BUILD_DIR "${PROJECT_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}")
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(EXTERNAL_BIN_DIR "${EXTERNAL_DIR}/Bin/${CMAKE_BUILD_TYPE}")

# External
set(ASSIMP_DIR "${EXTERNAL_DIR}/Assimp")
set(DXMATH_DIR "${EXTERNAL_DIR}/DirectXMath")
set(DXTEX_DIR "${EXTERNAL_DIR}/DirectXTex")
set(FTYPE_DIR "${EXTERNAL_DIR}/FreeType")


########## INCLUDE DIRECTORIES ##########

set(COMMON_INC_DIR "${COMMON_DIR}/Include")
set(ENGINE_INC_DIR "${ENGINE_DIR}/Include")

# External
set(ASSIMP_INC_DIR "${ASSIMP_DIR}/include")
set(DXMATH_INC_DIR "${DXMATH_DIR}/Inc")
set(DXTEX_INC_DIR "${DXTEX_DIR}/DirectXTex")
set(FTYPE_INC_DIR "${FTYPE_DIR}/include")
set(IMGUI_INC_DIR "${EXTERNAL_DIR}/ImGui")
set(JSON_INC_DIR ${EXTERNAL_DIR})


######### COMPILE CONFIGURATION #########

set(HE_CXX_STD cxx_std_20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
link_directories(${EXTERNAL_BIN_DIR})

# Compiler specific lags (remove 4250 later when ECS system created)
if(MSVC)
	# /MP not compatibile with /Yc (pch), option not found on gcc/clang
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /w34265 /wd4250 /sdl /MP /GF /fp:fast /permissive- /Zc:inline /Zc:rvalueCast /nologo /DYNAMICBASE")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /JMC /Od /Oy- /Zi /RTC1")
	set(COMPILER_REL_CXX_FLAGS "/MD /Ot /Oi /Oy /GL /GS /Gy")
	set(LINKER_FLAGS "/LTCG:STATUS /INCREMENTAL:NO /OPT:ICF /OPT:REF")
else()
	# GCC asumed although they should work on Clang too
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpessimizing-move -Wredundant-move -W4265 -Wno-4250 -pedantic -ffast-math")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
	set(COMPILER_REL_CXX_FLAGS "-O3 -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=2")
	set(LINKER_FLAGS "-flto")
endif()
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

# General flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_NO_DEBUG=0 -D_MODE_DEBUG")
set(COMPILER_REL_CXX_FLAGS "${COMPILER_REL_CXX_FLAGS} -D_NO_DEBUG=1")
set(CMAKE_CXX_FLAGS_RELEASE " ${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_REL_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMPILER_REL_CXX_FLAGS}")

# Shader flags
set(SHADER_FLAGS "/nologo /5.0 /E main /Ges /Zpc")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(SHADER_FLAGS "${SHADER_FLAGS} /Od /Zi")
endif()


############## SUBPROJECTS ##############

add_subdirectory(${COMMON_DIR})
#add_subdirectory(${ENGINE_DIR})
if(HE_BUILD_TOOL)
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

# Macro for loading external projects
#	PROJECT = prefix of all project variables
#	ADD_BIN_DIR = different path for library file
#	ADD_PDB_DIR = different path for MSVC debug symbols
macro(add_external_project PROJECT ADD_BIN_DIR ADD_PDB_DIR)
	set(${PROJECT}_BUILD_DIR "${BUILD_DIR}/${PROJECT}")
	set(${PROJECT}_OUT_LIB_NAME "${LIB_PREFIX}${${PROJECT}_TARGET}${LIB_EXT}")
	set(${PROJECT}_OUT_LIB "${EXTERNAL_BIN_DIR}/${${PROJECT}_OUT_LIB_NAME}")
	if(NOT EXISTS "${${PROJECT}_OUT_LIB}")
		set(${PROJECT}_INST_CMD "${CMAKE_COMMAND}" -E rename
			"${${PROJECT}_BUILD_DIR}/${ADD_BIN_DIR}${${PROJECT}_OUT_LIB_NAME}"
			"${${PROJECT}_OUT_LIB}")

		if(MSVC AND NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
			set(${PROJECT}_INST_CMD "${${PROJECT}_INST_CMD}" &&
				"${CMAKE_COMMAND}" -E rename
					"${${PROJECT}_BUILD_DIR}/${ADD_PDB_DIR}CMakeFiles/${${PROJECT}_LIB}.dir/${${PROJECT}_LIB}.pdb"
					"${EXTERNAL_BIN_DIR}/${${PROJECT}_LIB}.pdb")
		endif()
		ExternalProject_Add(${${PROJECT}_TARGET}
			SOURCE_DIR ${${PROJECT}_DIR}
			BINARY_DIR ${${PROJECT}_BUILD_DIR}
			STAMP_DIR ${${PROJECT}_BUILD_DIR}
			INSTALL_DIR ${EXTERNAL_BIN_DIR}
			CMAKE_CACHE_ARGS "${${PROJECT}_CACHE_ARGS}"
				"-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
			INSTALL_COMMAND "${${PROJECT}_INST_CMD}")
	elseif(EXISTS ${${PROJECT}_BUILD_DIR})
		file(REMOVE_RECURSE ${${PROJECT}_BUILD_DIR})
	endif()
endmacro()

# Assimp
if(${HE_MODEL_LOADING})
	# No exports set
	set(ASSIMP_CACHE_ARGS "-DBUILD_SHARED_LIBS:BOOL=OFF"
		"-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF"
		"-DASSIMP_BUILD_TESTS:BOOL=OFF"
		"-DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF"
		"-DINJECT_DEBUG_POSTFIX:BOOL=OFF"
		"-DASSIMP_INSTALL:BOOL=OFF"
		"-DASSIMP_INSTALL_PDB:BOOL=OFF"
		"-DASSIMP_NO_EXPORT:BOOL=OFF")
	add_external_project(ASSIMP "lib/" "code/")
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