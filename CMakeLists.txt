set(ZE_CMAKE_VERSION 3.22)
cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})

######### PROJECT CONFIGURATION #########

set(PROJECT_NAME "Zenith Engine")
project(${PROJECT_NAME} VERSION 0.3)

# Project options
set(ZE_BIN_DIR "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for project data (shaders, resources, executables)")
set(ZE_BUILD_DIR "${PROJECT_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for build files")
set(ZE_ICON_ID 101 CACHE STRING "Icon identyficator for Windows builds,
	should be set with icon ID from your .rc file")

option(ZE_MODEL_LOADING "Enable external model loading module of engine" ON)
option(ZE_USE_WIDE_ENTITY_ID "Forces usage of 64bit enitity ID (when large number of enities is possible)" OFF)
option(ZE_DEBUG_FRAME_MEMORY_PRINT "Enable printing of frame buffer memory allocation for supported APIs on debug builds" OFF)
option(ZE_DEBUG_FRAME_NO_ALIASING_MEMORY "Disable packing of frame buffer memory and aliasing it between resources" OFF)
option(ZE_DEBUG_GPU_VALIDATION "Enable additional GPU validation for supported APIs on debug builds for more verbose checks. May slow rendering considerably" OFF)
option(ZE_RENDERER_SINGLE_THREAD "Turns off recording commands by multiple threads in the renderer" ON)
option(ZE_BUILD_TOOL "Enable building of command line utility tool" ON)
option(ZE_BUILD_DEMO "Enable building of technological demo" ON)
option(ZE_NO_DATA "Disable copying of default engine assets" ON)
option(ZE_CI_JOB "Build engine for CI job" OFF)

# CMake modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")
include(ExternalProject)
include(External)
include(CheckTypeSize)

# Architecture check, only x64
check_type_size(void* SIZEOF_VOID_PTR)
if(NOT ${SIZEOF_VOID_PTR} STREQUAL "8")
    message(FATAL_ERROR "Unsupported architecture!")
endif()

# Check current configuration
if(NOT (${ZE_BUILD_DEBUG} OR ${ZE_BUILD_DEVELOPMENT} OR ${ZE_BUILD_PROFILE} OR ${ZE_BUILD_RELEASE}))
	message(FATAL_ERROR "Unsupported build type [${CMAKE_BUILD_TYPE}]!")
endif()
if(NOT (${ZE_PLATFORM_WINDOWS}))
	message(FATAL_ERROR "Building for unsupported platform [${CMAKE_SYSTEM_NAME}]!")
endif()
if(NOT (${ZE_COMPILER_MSVC} OR ${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC}))
	message(FATAL_ERROR "Using unsupported compiler [${CMAKE_CXX_COMPILER}]!")
endif()


########### MAIN DIRECTORIES ############

set(COMMON_DIR "${PROJECT_SOURCE_DIR}/Common")
set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/Engine")
set(TOOL_DIR "${PROJECT_SOURCE_DIR}/EditTool")
set(DEMO_DIR "${PROJECT_SOURCE_DIR}/Demo")
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(EXTERNAL_BIN_DIR "${EXTERNAL_DIR}/Bin/${CMAKE_BUILD_TYPE}")

# External
set(ZLIB_DIR "${EXTERNAL_DIR}/zlib")
set(ASSIMP_DIR "${EXTERNAL_DIR}/Assimp")
set(DXMATH_DIR "${EXTERNAL_DIR}/DirectXMath")
set(FTYPE_DIR "${EXTERNAL_DIR}/FreeType")
set(DXTEX_DIR "${EXTERNAL_DIR}/DirectXTex")
set(PIX_DIR "${EXTERNAL_DIR}/WinPixEventRuntime")
set(IMGUI_DIR "${EXTERNAL_DIR}/ImGui")
set(ENTT_DIR "${EXTERNAL_DIR}/EnTT")
set(AGS_DIR "${EXTERNAL_DIR}/AGS")
set(AGILITY_DIR "${EXTERNAL_DIR}/AgilitySDK")
set(VOLK_DIR "${EXTERNAL_DIR}/volk")
set(LIBPNG_DIR "${EXTERNAL_DIR}/libpng")
set(HARFBUZZ_DIR "${EXTERNAL_DIR}/HarfBuzz")


########## INCLUDE DIRECTORIES ##########

set(COMMON_INC_DIR "${COMMON_DIR}/Include")
set(ENGINE_INC_DIR "${ENGINE_DIR}/Include")

# External
set(EXTERNAL_CONFIG_INC_DIR "${EXTERNAL_BIN_DIR}")
set(ZLIB_INC_DIR "${ZLIB_DIR}")
set(ASSIMP_INC_DIR "${ASSIMP_DIR}/include")
set(DXMATH_INC_DIR "${DXMATH_DIR}/Inc")
set(FTYPE_INC_DIR "${FTYPE_DIR}/include")
set(DXTEX_INC_DIR "${DXTEX_DIR}/DirectXTex")
set(PIX_INC_DIR "${PIX_DIR}/Include")
set(IMGUI_INC_DIR "${IMGUI_DIR}")
set(ENTT_INC_DIR "${ENTT_DIR}/src")
set(AGS_INC_DIR "${AGS_DIR}/ags_lib/inc")
set(AGILITY_INC_DIR "${AGILITY_DIR}/include")
set(VOLK_INC_DIR "${VOLK_DIR}")
set(LIBPNG_INC_DIR "${LIBPNG_DIR}")
set(HARFBUZZ_INC_DIR "${HARFBUZZ_DIR}/src")
set(EXT_INC_DIR "${EXTERNAL_DIR}/Include")
set(EXT_SHADER_INC_DIR "${EXTERNAL_DIR}/ShaderInclude")


############# USING ENGINE ##############

set(ZE_API_DIR "${PROJECT_SOURCE_DIR}/Engine/API") # Global directory for API headers 
set(ZE_API_HEADER "${ZE_API_DIR}/Zenith.h")        # Main engine API header file
set(ZE_BUILD_INC_DIR "${ZE_BUILD_DIR}/Include")    # Directory containing generated headers
set(ZE_INC_DIRS "${ZE_API_DIR}"                    # Collection of all neccessary include directories for linking agains engine library
	"${ZE_BUILD_INC_DIR}"
	"${COMMON_INC_DIR}"
	"${ENGINE_INC_DIR}"
	"${EXTERNAL_CONFIG_INC_DIR}"
	"${ZLIB_INC_DIR}"
	"${ASSIMP_INC_DIR}"
	"${DXMATH_INC_DIR}"
	"${FTYPE_INC_DIR}"
	"${DXTEX_INC_DIR}"
	"${PIX_INC_DIR}"
	"${IMGUI_INC_DIR}"
	"${ENTT_INC_DIR}"
	"${AGS_INC_DIR}"
	"${AGILITY_INC_DIR}"
	"${VOLK_INC_DIR}"
	"${LIBPNG_INC_DIR}"
	"${HARFBUZZ_INC_DIR}"
	"${EXT_INC_DIR}"
	"${EXT_SHADER_INC_DIR}")


################ TARGETS ################

set(TOOL_TARGET "ZEdit")
set(DEMO_TARGET "ZenithDemo")

# Libraries
set(COMMON_TARGET "common")
set(ENGINE_TARGET "zenith")

# External libraries
set(ZLIB_LIB "zlibstatic")
set(ASSIMP_LIB "assimp")
set(FTYPE_LIB "freetype")
set(DXTEX_LIB "DirectXTex")
set(PIX_LIB "WinPixEventRuntime")
set(ENTT_LIB "EnTT::EnTT")
set(AGS_LIB "amd_ags_x64")
set(VOLK_LIB "volk")
set(LIBPNG_LIB "png_static")
set(LIBPNG_TARGET "libpng16_static") # Will need to increase number based on version
set(HARFBUZZ_LIB "harfbuzz")

# Inject debug suffix if library use it
if(${ZE_BUILD_DEBUG})
	if(${ZE_MODEL_LOADING})
		set(ZLIB_TARGET "${ZLIB_LIB}d")
		set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mtd")
	endif()
	set(FTYPE_TARGET "${FTYPE_LIB}d")
	set(LIBPNG_TARGET "${LIBPNG_TARGET}d")
else()
	if(${ZE_MODEL_LOADING})
		set(ZLIB_TARGET "${ZLIB_LIB}")
		set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mt")
	endif()
	set(FTYPE_TARGET "${FTYPE_LIB}")
endif()

# Add marker library
if(${ZE_PLATFORM_WINDOWS} AND NOT ${ZE_BUILD_RELEASE})
	set(PIX_TARGET "${PIX_DIR}/bin/x64/${PIX_LIB}.lib")
else()
	set(PIX_TARGET "")
endif()
set(DXTEX_TARGET "${DXTEX_LIB}")
set(ENTT_TARGET "${ENTT_LIB}")
set(AGS_TARGET "${AGS_DIR}/ags_lib/lib/${AGS_LIB}.lib")
set(VOLK_TARGET "${VOLK_LIB}")
set(HARFBUZZ_TARGET "${HARFBUZZ_LIB}")


######### COMPILE CONFIGURATION #########

set(ZE_CXX_STD cxx_std_23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
link_directories(${EXTERNAL_BIN_DIR})

# Platform specific flags
if(${ZE_PLATFORM_WINDOWS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_UNICODE -DUNICODE -DNOMINMAX -DZE_EXPORT=__declspec(dllexport) -DZE_IMPORT=__declspec(dllimport) -DVK_USE_PLATFORM_WIN32_KHR")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DUSE_PIX")
	set(CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_DEBUG} -DUSE_PIX")
	set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -DUSE_PIX")
elseif(${ZE_PLATFORM_LINUX})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR -DVK_USE_PLATFORM_XCB_KHR -DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_DIRECTFB_EXT -DVK_USE_PLATFORM_XLIB_XRANDR_EXT")
	message(FATAL_ERROR "Building for unsupported platform!")
else()
	message(FATAL_ERROR "Building for unsupported platform!")
endif()

# Compiler specific flags
if(${ZE_COMPILER_MSVC})
	# /MP not compatibile with /Yc (pch), option not found on gcc/clang
	# Move to /EHsc when exceptions will be replaced with error codes
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /sdl /MP /GF /fp:fast /permissive- /Zc:inline /Zc:rvalueCast /nologo /DYNAMICBASE")
	# C4265 - class with virtual functions does not have virutal destructor
	# C4061 - not all enums are explicitly handled in switch where `default` is present
	# C4668 - not defined as preprocessor macro in #if/#elif
	# C4820 - X bytes padding added after data member
	# C4514 - unreferenced inline function has been removed
	# C5045 - compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
	# C4100 - unreferenced formal parameter
	# C4582 - constructor is not implicitly called
	# C4583 - destructor is not implicitly called
	# C4623 - default constructor was implicitly defined as deleted
	# C4625 - copy constructor was implicitly defined as deleted
	# C4626 - assignment operator was implicitly defined as deleted
	# C5026 - move constructor was implicitly defined as deleted
	# C5027 - move assignment operator was implicitly defined as deleted
	# C4365 - signed/unsigned mismatch
	# C4652 - compiler option inconsistent with precompiled header; current command-line option will override that defined in the precompiled header
	#	Note: Caused by removing /Wall from CXX_FLAGS and then setting it for specific files, due to ImGui warnings, maybe /W4 would suffice?
	# C4868 - compiler may not enforce left-to-right evaluation order in braced initializer list
	#	Note: Caused by entt::dense_map, just hide it.
	# C4710 - function not inlined
	# C4711 - function selected for automatic inline expansion
	# C4355 - 'this' used in base member initializer list
	# C5246 - the initialization of a subobject should be wrapped in braces
	#	Note: Caused by not respecing newer initializations aggregate, bug in MSVC
	# C4464 - relative include path contains '..'
	# C5262 - currently MSVC standard library uses a lot of unguarded fall-through switch cases
	#	Note: Enable when fixed by Microsoft since it's bloating output beyond measure.
	# C5264 - currently MSVC standard library contains a lot of unsused variables
	#	Note: Enable when fixed by Microsoft since it's bloating output beyond measure.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /wd4061 /wd4668 /wd4820 /wd4514 /wd5045 /wd4100 /wd4582 /wd4583 /wd4623 /wd4625 /wd4626 /wd5026 /wd5027 /wd4365 /wd4652 /wd4868 /wd4710 /wd4711 /wd4355 /wd5246 /wd4464 /wd5262 /wd5264")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /JMC /Od /Oy- /Zi /RTC1")
	set(CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_DEVELOPMENT} /MD /Ot /Oi /Oy /GL /GS /Gy")
	set(CXX_PERFORMANCE_FLAGS "/MD /Ot /Oi /Oy /GL /GS /Gy")
elseif(${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC})
	if(${ZE_COMPILER_CLANG})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ZE_COMPILER_CLANG")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ZE_COMPILER_GCC")
	endif()
	# GCC asumed although they should work on Clang too
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpessimizing-move -Wredundant-move -pedantic -ffast-math")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
	set(CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_DEVELOPMENT} -O2 -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=2")
	set(CXX_PERFORMANCE_FLAGS "-O3 -flto -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=2")
else()
	message(FATAL_ERROR "Using unsupported compiler!")
endif()
# Same set of flags for PROFILE and RELEASE builds with max optimizations
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${CXX_PERFORMANCE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_PERFORMANCE_FLAGS}")

# General flags with project options
add_definitions(-D_ZE_MODEL_LOADING=$<BOOL:${ZE_MODEL_LOADING}>)
add_definitions(-D_ZE_RENDER_GRAPH_SINGLE_THREAD=$<BOOL:${ZE_RENDERER_SINGLE_THREAD}>)
add_definitions(-D_ZE_DEBUG_FRAME_MEMORY_PRINT=$<BOOL:${ZE_DEBUG_FRAME_MEMORY_PRINT}>)
add_definitions(-D_ZE_DEBUG_FRAME_NO_ALIASING_MEMORY=$<BOOL:${ZE_DEBUG_FRAME_NO_ALIASING_MEMORY}>)
add_definitions(-D_ZE_DEBUG_GPU_VALIDATION=$<BOOL:${ZE_DEBUG_GPU_VALIDATION}>)
add_definitions(-D_ZE_DEBUG_GFX_API=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>>)
add_definitions(-D_ZE_DEBUG_GFX_NAMES=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>>)
add_definitions(-D_ZE_GFX_MARKERS=$<NOT:$<BOOL:${ZE_BUILD_RELEASE}>>)
# Platform type
add_definitions(-D_ZE_PLATFORM_WINDOWS=$<BOOL:${ZE_PLATFORM_WINDOWS}> -D_ZE_PLATFORM_LINUX=$<BOOL:${ZE_PLATFORM_LINUX}> -D_ZE_PLATFORM_ANDROID=$<BOOL:${ZE_PLATFORM_ANDROID}>)
add_definitions(-D_ZE_PLATFORM_FUCHSIA=$<BOOL:${ZE_PLATFORM_FUCHSIA}> -D_ZE_PLATFORM_NSWITCH=$<BOOL:${ZE_PLATFORM_NSWITCH}>)
# Compiler type
add_definitions(-D_ZE_COMPILER_MSVC=$<BOOL:${ZE_COMPILER_MSVC}> -D_ZE_COMPILER_CLANG=$<BOOL:${ZE_COMPILER_CLANG}> -D_ZE_COMPILER_GCC=$<BOOL:${ZE_COMPILER_GCC}>)
# Build type
add_definitions(-D_ZE_MODE_DEBUG=$<BOOL:${ZE_BUILD_DEBUG}> -D_ZE_MODE_DEV=$<BOOL:${ZE_BUILD_DEVELOPMENT}> -D_ZE_MODE_PROFILE=$<BOOL:${ZE_BUILD_PROFILE}> -D_ZE_MODE_RELEASE=$<BOOL:${ZE_BUILD_RELEASE}>)
if(${ZE_USE_WIDE_ENTITY_ID})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENTT_ID_TYPE=std::uint64_t")
endif()
# EnTT is causing errors due to usage of std::aligned_storage, to be removed when it's fixed in lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMGUI_USER_CONFIG=<${ENGINE_INC_DIR}/GUI/ImGuiConfig.h> -DENTT_NOEXCEPTION -DVK_NO_PROTOTYPES -DVK_ENABLE_BETA_EXTENSIONS -D_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING")

# Shader flags
set(SHADER_FLAGS "/nologo /E main /Ges /Zpc /I \"${EXT_SHADER_INC_DIR}\"")
if(${ZE_BUILD_DEBUG} OR ${ZE_BUILD_DEVELOPMENT})
	set(SHADER_FLAGS "${SHADER_FLAGS} /Zi /Od")
endif()


############## SUBPROJECTS ##############

add_subdirectory(${COMMON_DIR})
add_subdirectory(${ENGINE_DIR})
if(${ZE_BUILD_TOOL})
	add_subdirectory(${TOOL_DIR})
endif()
if(${ZE_BUILD_DEMO})
	add_subdirectory(${DEMO_DIR})
endif()


########### EXTERNAL PROJECTS ###########

if(${ZE_PLATFORM_WINDOWS})
	set(LIB_EXT ".lib")
	set(LIB_PREFIX "")
else()
	set(LIB_EXT ".a")
	set(LIB_PREFIX "lib")
endif()

# zlib
set(ZLIB_CACHE_ARGS "")
add_external_project(ZLIB "" "" "")
if(NOT EXISTS "${EXTERNAL_BIN_DIR}/zconf.h")
	ExternalProject_Add_Step(${ZLIB_TARGET} headerConfig
		DEPENDEES install
		LOG OFF
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${ZLIB_BUILD_DIR}/zconf.h"
		"${EXTERNAL_BIN_DIR}")
endif()

 # libpng
set(LIBPNG_CACHE_ARGS "-DPNG_BUILD_ZLIB:BOOL=OFF"
	"-DZLIB_FOUND:BOOL=ON"
	"-DZLIB_INCLUDE_DIRS:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_LIBRARIES:STRING=${ZLIB_TARGET}"
	"-DZLIB_LIBRARY:STRING=${ZLIB_TARGET}"
	"-DPNG_SHARED:BOOL=OFF"
	"-DPNG_STATIC:BOOL=ON"
	"-DPNG_EXECUTABLES:BOOL=OFF"
	"-DPNG_TESTS:BOOL=OFF"
	"-DPNG_FRAMEWORK:BOOL=OFF"
	"-DPNG_DEBUG:BOOL=OFF"
	"-DPNG_HARDWARE_OPTIMIZATIONS:BOOL=ON")
add_external_project(LIBPNG "" "" "ZLIB")
if(NOT EXISTS "${LIBPNG_OUT_LIB}")
	ExternalProject_Add_Step(${LIBPNG_TARGET} headerConfig
		DEPENDEES install
		LOG OFF
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${LIBPNG_BUILD_DIR}/pnglibconf.h"
		"${EXTERNAL_BIN_DIR}")
endif()

# Assimp
if(${ZE_MODEL_LOADING})
	# No exports set
	set(ASSIMP_CACHE_ARGS "-DBUILD_SHARED_LIBS:BOOL=OFF"
		"-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF"
		"-DASSIMP_BUILD_TESTS:BOOL=OFF"
		"-DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF"
		"-DINJECT_DEBUG_POSTFIX:BOOL=OFF"
		"-DASSIMP_INSTALL:BOOL=OFF"
		"-DASSIMP_BUILD_ZLIB:BOOL=ON"
		"-DZLIB_FOUND:BOOL=OFF"
		"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR}"
		"-DASSIMP_INSTALL_PDB:BOOL=OFF"
		"-DASSIMP_NO_EXPORT:BOOL=OFF")
	add_external_project(ASSIMP "lib/" "code/" ZLIB)
	if(NOT EXISTS "${ASSIMP_OUT_LIB}")
		ExternalProject_Add_Step(${ASSIMP_TARGET} headerConfig
			DEPENDEES install
			LOG OFF
			COMMAND "${CMAKE_COMMAND}" -E copy_directory
			"${ASSIMP_BUILD_DIR}/include"
			"${EXTERNAL_BIN_DIR}")
	endif()
endif()

# HarfBuzz
set(HARFBUZZ_CACHE_ARGS "-DBUILD_FRAMEWORK:BOOL=OFF"
	"-DHB_HAVE_DIRECTWRITE:BOOL=ON")
add_external_project(HARFBUZZ "" "" "")

# FreeType
set(FTYPE_CACHE_ARGS "-DFT_DISABLE_ZLIB:BOOL=ON"
	"-DZLIB_FOUND:BOOL=ON"
	"-DZLIB_INCLUDE_DIRS:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_LIBRARIES:STRING=${ZLIB_TARGET}"
	"-DZLIB_LIBRARY:STRING=${ZLIB_TARGET}"
	"-DFT_DISABLE_PNG:BOOL=ON"
	"-DPNG_FOUND:BOOL=ON"
	"-DPNG_INCLUDE_DIRS:STRING=${LIBPNG_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DPNG_INCLUDE_DIR:STRING=${LIBPNG_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DPNG_LIBRARIES:STRING=${LIBPNG_TARGET}"
	"-DPNG_LIBRARY:STRING=${LIBPNG_TARGET}")
add_external_project(FTYPE "" "" "LIBPNG;HARFBUZZ")

 # DirectXTex
set(DXTEX_CACHE_ARGS "-DBUILD_TOOLS:BOOL=OFF"
	"-DBUILD_SAMPLE:BOOL=OFF")
add_external_project(DXTEX "bin/CMake/" "" "")

# EnTT
if(${ZE_COMPILER_MSVC})
	set(ENTT_INCLUDE_NATVIS ON CACHE BOOL "Use debug natvis files" FORCE)
elseif(${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC})
	set(ENTT_USE_SANITIZER ON CACHE BOOL "Enable address sanitizer" FORCE)
endif()
set(ENTT_INCLUDE_HEADERS ON CACHE BOOL "Include header for dependency" FORCE)
add_subdirectory(${ENTT_DIR})

# volk
if(${ZE_PLATFORM_WINDOWS})
	set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR CACHE STRING "Enabled Vulkan for Windows" FORCE)
else()
	message(FATAL_ERROR "Building for unsupported platform!")
endif()
add_subdirectory(${VOLK_DIR})