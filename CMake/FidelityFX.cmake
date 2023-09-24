include_guard(DIRECTORY)

# Setup variables needed for compile of FidelityFX SDK effect
if(${ZE_PLATFORM_WINDOWS})
    set(FFXSDK_PLATFORM_NAME "x64")
else()
    set(FFXSDK_PLATFORM_NAME "unknown")
endif()

set(FFXSDK_SHARED_PATH "${FFXSDK_DIR}/src/shared")
set(FFXSDK_HOST_PATH "${FFXSDK_DIR}/include/FidelityFX/host")

file(GLOB FFXSDK_SHARED_SOURCES
    "${FFXSDK_SHARED_PATH}/*.cpp"
    "${FFXSDK_SHARED_PATH}/*.h")
file(GLOB FFXSDK_PUBLIC_SOURCES
    "${FFXSDK_HOST_PATH}/*.h")
    

# Macro for adding FidelityFX SDK effects to project targets
#	EFFECT_NAME = name of given effect
# Returns link targets in variable FFXSDK_TARGETS
macro(add_fidelityfx_target EFFECT_NAME)
    string(TOLOWER ${EFFECT_NAME} FFX${EFFECT_NAME})

    set(FFX${EFFECT_NAME}_DIR "${FFXSDK_DIR}/src/components/${FFX${EFFECT_NAME}}")
    set(FFX${EFFECT_NAME}_LIB "ffx_${FFX${EFFECT_NAME}}_${FFXSDK_PLATFORM_NAME}")
    set(FFX${EFFECT_NAME}_TARGET "${FFX${EFFECT_NAME}_LIB}")
    set(FFXSDK_TARGETS ${FFXSDK_TARGETS} ${FFX${EFFECT_NAME}_TARGET})
endmacro()

# Macro for loading FidelityFX SDK effects as external projects
#	EFFECT_NAME = name of given effect
macro(add_external_fidelityfx_effect EFFECT_NAME)
    set(FFX${EFFECT_NAME}_CACHE_ARGS "-DFFX_${EFFECT_NAME}:BOOL=TRUE"
        "-DFFX_PLATFORM_NAME:STRING=${FFXSDK_PLATFORM_NAME}"
	    "-DFFX_SHARED_SOURCES:STRING=${FFXSDK_SHARED_SOURCES}"
	    "-DFFX_PUBLIC_SOURCES:STRING=${FFXSDK_PUBLIC_SOURCES}"
	    "-DFFX_SHARED_PATH:STRING=${FFXSDK_SHARED_PATH}"
	    "-DFFX_INCLUDE_PATH:STRING=${FFXSDK_DIR}/include"
	    "-DFFX_HOST_PATH:STRING=${FFXSDK_HOST_PATH}"
	    "-DFFX_COMPONENTS_PATH:STRING=${FFXSDK_DIR}/src/components")
    add_external_project(FFX${EFFECT_NAME} "" "" "")
endmacro()