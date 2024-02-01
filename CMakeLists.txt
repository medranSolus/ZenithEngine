set(ZE_CMAKE_VERSION 3.22)
cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})
cmake_policy(SET CMP0091 NEW) # Setting of MSVC runtime library

######### PROJECT CONFIGURATION #########

set(ZE_PROJECT_NAME "Zenith Engine")
set(ZE_VER_MAJOR 0)
set(ZE_VER_MINOR 3)
set(ZE_VER_PATCH 0)
project(${ZE_PROJECT_NAME} VERSION ${ZE_VER_MAJOR}.${ZE_VER_MINOR}.${ZE_VER_PATCH})

# Project options
set(ZE_BIN_DIR "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for project data (shaders, resources, executables)")
set(ZE_BUILD_DIR "${PROJECT_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for build files")
set(ZE_ICON_ID 101 CACHE STRING "Icon identyficator for Windows builds,
	should be set with icon ID from your .rc file")

option(ZE_ENABLE_DX11 "Enable DirectX 11 API usage" ON)
option(ZE_ENABLE_DX12 "Enable DirectX 12 API usage" ON)
option(ZE_ENABLE_GL "Enable OpenGL API usage" OFF)
option(ZE_ENABLE_VK "Enable Vulkan API usage" ON)
option(ZE_EXTERNAL_MODEL_LOADING "Enable external model loading module of engine, making it possible to load custom file formats" ON)
option(ZE_USE_WIDE_ENTITY_ID "Forces usage of 64bit enitity ID (when large number of enities is possible)" OFF)
option(ZE_DEBUG_FRAME_MEMORY_PRINT "Enable printing of frame buffer memory allocation for supported APIs on debug builds" OFF)
option(ZE_DEBUG_FRAME_NO_ALIASING_MEMORY "Disable packing of frame buffer memory and aliasing it between resources" OFF)
option(ZE_RENDERER_SINGLE_THREAD "Turns off recording commands by multiple threads in the renderer" ON)
option(ZE_RENDERER_NO_SPLIT_BARRIERS "Turns off splitting barriers and performs them immediatelly" ON)
option(ZE_BUILD_TOOL "Enable building of command line utility tool" ON)
option(ZE_BUILD_DEMO "Enable building of technological demo" ON)
option(ZE_NO_DATA "Disable copying of default engine assets" ON)
option(ZE_CI_JOB "Build engine for CI job" OFF)

# CMake modules
include(ExternalProject)
include(CheckTypeSize)

# Architecture check, only x64
check_type_size(void* SIZEOF_VOID_PTR)
if(NOT ${SIZEOF_VOID_PTR} STREQUAL "8")
    message(FATAL_ERROR "Unsupported architecture!")
endif()

# Check current configuration
if(${ZE_BUILD_DEBUG})
	set(ZE_EXTERNAL_BUILD_DEBUG_INFO TRUE)
	set(ZE_EXTERNAL_BUILD_DEBUG_POSTFIX TRUE)
	set(ZE_EXTERNAL_BUILD_TYPE "Debug")
elseif(${ZE_BUILD_DEVELOPMENT} OR ${ZE_BUILD_PROFILE})
	set(ZE_EXTERNAL_BUILD_DEBUG_INFO TRUE)
	set(ZE_EXTERNAL_BUILD_DEBUG_POSTFIX FALSE)
	set(ZE_EXTERNAL_BUILD_TYPE "RelWithDebInfo")
elseif(${ZE_BUILD_RELEASE})
	set(ZE_EXTERNAL_BUILD_DEBUG_INFO FALSE)
	set(ZE_EXTERNAL_BUILD_DEBUG_POSTFIX FALSE)
	set(ZE_EXTERNAL_BUILD_TYPE "Release")
else()
	message(FATAL_ERROR "Unsupported build type [${CMAKE_BUILD_TYPE}]!")
endif()
if(${ZE_PLATFORM_LINUX})
	set(ZE_ENABLE_DX11 OFF CACHE BOOL "DirectX 11 not supported for Linux" FORCE)
	set(ZE_ENABLE_DX12 OFF CACHE BOOL "DirectX 12 not supported for Linux" FORCE)
elseif(NOT (${ZE_PLATFORM_WINDOWS}))
	message(FATAL_ERROR "Building for unsupported platform [${CMAKE_SYSTEM_NAME}]!")
endif()
if(NOT (${ZE_COMPILER_MSVC} OR ${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC}))
	message(FATAL_ERROR "Using unsupported compiler [${CMAKE_CXX_COMPILER}]!")
endif()
if(NOT (${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12} OR ${ZE_ENABLE_VK}))
	message(FATAL_ERROR "No RHI backend enabled, at least one must be active!")
endif()

# Check flags provided to CMake by toolchains
if(FALSE)
	display_cmake_flags()
endif()


########### MAIN DIRECTORIES ############

set(COMMON_DIR "${PROJECT_SOURCE_DIR}/Common")
set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/Engine")
set(TOOL_DIR "${PROJECT_SOURCE_DIR}/EditTool")
set(DEMO_DIR "${PROJECT_SOURCE_DIR}/Demo")
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(EXTERNAL_BIN_DIR "${EXTERNAL_DIR}/Bin/${CMAKE_BUILD_TYPE}")

# External
set(AGILITY_DIR "${EXTERNAL_DIR}/AgilitySDK")
set(AGS_DIR "${EXTERNAL_DIR}/AGS")
set(ASSIMP_DIR "${EXTERNAL_DIR}/Assimp")
set(DLSS_DIR "${EXTERNAL_DIR}/DLSS")
set(DSTORE_DIR "${EXTERNAL_DIR}/DirectStorage")
set(DXMATH_DIR "${EXTERNAL_DIR}/DirectXMath")
set(ENTT_DIR "${EXTERNAL_DIR}/EnTT")
set(FTYPE_DIR "${EXTERNAL_DIR}/FreeType")
set(FFXSDK_DIR "${EXTERNAL_DIR}/FidelityFXSDK/sdk")
set(HARFBUZZ_DIR "${EXTERNAL_DIR}/HarfBuzz")
set(IMGUI_DIR "${EXTERNAL_DIR}/ImGui")
set(LIBPNG_DIR "${EXTERNAL_DIR}/libpng")
set(LIBSPNG_DIR "${EXTERNAL_DIR}/libspng")
set(NIS_DIR "${EXTERNAL_DIR}/NvidiaImageScaling")
set(VOLK_DIR "${EXTERNAL_DIR}/volk")
set(PIX_DIR "${EXTERNAL_DIR}/WinPixEventRuntime")
set(XESS_DIR "${EXTERNAL_DIR}/XeSS")
set(QOI_DIR "${EXTERNAL_DIR}/qoixx")
set(ZLIB_DIR "${EXTERNAL_DIR}/zlib")


########## INCLUDE DIRECTORIES ##########

set(COMMON_INC_DIR "${COMMON_DIR}/Include")
set(ENGINE_INC_DIR "${ENGINE_DIR}/Include")

# External
set(EXT_INC_DIR "${EXTERNAL_DIR}/Include")
set(EXT_CONFIG_INC_DIR "${EXTERNAL_BIN_DIR}")
set(EXT_SHADER_INC_DIR "${EXTERNAL_DIR}/ShaderInclude")
set(AGILITY_INC_DIR "${AGILITY_DIR}/include")
set(AGS_INC_DIR "${AGS_DIR}/ags_lib/inc")
set(ASSIMP_INC_DIR "${ASSIMP_DIR}/include")
set(DLSS_INC_DIR "${DLSS_DIR}/include")
set(DSTORE_INC_DIR "${DSTORE_DIR}/include")
set(DXMATH_INC_DIR "${DXMATH_DIR}/Inc")
set(ENTT_INC_DIR "${ENTT_DIR}/src")
set(FTYPE_INC_DIR "${FTYPE_DIR}/include")
set(FFXSDK_INC_DIR "${FFXSDK_DIR}/include")
set(HARFBUZZ_INC_DIR "${HARFBUZZ_DIR}/src")
set(IMGUI_INC_DIR "${IMGUI_DIR}")
set(LIBPNG_INC_DIR "${LIBPNG_DIR}")
set(LIBSPNG_INC_DIR "${LIBSPNG_DIR}/spng")
set(NIS_INC_DIR "${NIS_DIR}/NIS")
set(VOLK_INC_DIR "${VOLK_DIR}")
set(PIX_INC_DIR "${PIX_DIR}/Include")
set(XESS_INC_DIR "${XESS_DIR}/inc")
set(QOI_INC_DIR "${QOI_DIR}/include")
set(ZLIB_INC_DIR "${ZLIB_DIR}")


############# CUSTOM CMAKE MODULES ##############

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")
include(CompileFilter)
include(External)
include(FidelityFX)
include(FlagsDisplay)
include(FlagsPass)
include(ShaderBuild)
include(PostBuild)


############# USING ENGINE ##############

set(ZE_API_DIR "${PROJECT_SOURCE_DIR}/Engine/API") # Global directory for API headers 
set(ZE_API_HEADER "${ZE_API_DIR}/Zenith.h")        # Main engine API header file
set(ZE_BUILD_INC_DIR "${ZE_BUILD_DIR}/Include")    # Directory containing generated headers
set(ZE_INC_DIRS "${ZE_API_DIR}"                    # Collection of all neccessary include directories for linking agains engine library
	"${ZE_BUILD_INC_DIR}"
	"${COMMON_INC_DIR}"
	"${ENGINE_INC_DIR}"
	"${EXT_INC_DIR}"
	"${EXT_CONFIG_INC_DIR}"
	"${EXT_SHADER_INC_DIR}"
	"${AGILITY_INC_DIR}"
	"${AGS_INC_DIR}"
	"${ASSIMP_INC_DIR}"
	"${DLSS_INC_DIR}"
	"${DSTORE_INC_DIR}"
	"${DXMATH_INC_DIR}"
	"${DXTEX_INC_DIR}"
	"${ENTT_INC_DIR}"
	"${FTYPE_INC_DIR}"
	"${FFXSDK_INC_DIR}"
	"${HARFBUZZ_INC_DIR}"
	"${IMGUI_INC_DIR}"
	"${LIBPNG_INC_DIR}"
	"${LIBSPNG_INC_DIR}"
	"${NIS_INC_DIR}"
	"${VOLK_INC_DIR}"
	"${PIX_INC_DIR}"
	"${XESS_INC_DIR}"
	"${QOI_INC_DIR}"
	"${ZLIB_INC_DIR}")


################ TARGETS ################

set(TOOL_TARGET "ZEdit")
set(DEMO_TARGET "ZenithDemo")

# Libraries
set(COMMON_TARGET "common")
set(ENGINE_TARGET "zenith")

# External libraries
set(AGS_LIB "amd_ags_x64")
set(ASSIMP_LIB "assimp")
set(DLSS_LIB "nvsdk_ngx")
set(DSTORE_LIB "dstorage")
set(ENTT_LIB "EnTT::EnTT")
set(FTYPE_LIB "freetype")
set(HARFBUZZ_LIB "harfbuzz")
set(LIBPNG_LIB "png_static")
set(LIBSPNG_LIB "spng_static")
set(VOLK_LIB "volk")
set(PIX_LIB "WinPixEventRuntime")
set(XESS_LIB "libxess")
set(ZLIB_LIB "zlibstatic")

if(${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12})
	set(AGS_TARGET "${AGS_DIR}/ags_lib/lib/${AGS_LIB}.lib")
endif()
if(${ZE_EXTERNAL_MODEL_LOADING})
	set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mt")
endif()
if(${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12} OR ${ZE_ENABLE_VK})
	if(${ZE_PLATFORM_WINDOWS})
		set(DLSS_TARGET "${DLSS_DIR}/lib/Windows_x86_64/x86_64/${DLSS_LIB}_s.lib")
	elseif(${ZE_PLATFORM_LINUX})
		set(DLSS_TARGET "${DLSS_DIR}/lib/Linux_x86_64/lib${DLSS_LIB}.a")
	endif()
endif()
if(${ZE_ENABLE_DX12})
	set(DSTORE_TARGET "${DSTORE_DIR}/lib/x64/${DSTORE_LIB}.lib")
endif()
set(ENTT_TARGET "${ENTT_LIB}")
set(FTYPE_TARGET "${FTYPE_LIB}")
set(HARFBUZZ_TARGET "${HARFBUZZ_LIB}")
set(LIBPNG_TARGET "libpng16_static") # Will need to increase number based on version
set(LIBSPNG_TARGET "${LIBSPNG_LIB}")
if(${ZE_ENABLE_VK})
	set(VOLK_TARGET "${VOLK_LIB}")
endif()
if(${ZE_ENABLE_DX12} AND NOT ${ZE_BUILD_RELEASE})
	set(PIX_TARGET "${PIX_DIR}/bin/x64/${PIX_LIB}.lib")
endif()
if(${ZE_ENABLE_DX12})
	set(XESS_TARGET "${XESS_DIR}/lib/${XESS_LIB}.lib")
endif()
set(ZLIB_TARGET "${ZLIB_LIB}")

# FidelityFX SDK effects
add_fidelityfx_target(FSR1)
add_fidelityfx_target(FSR2)
add_fidelityfx_target(CACAO)
add_fidelityfx_target(SSSR)

# Inject debug postfix if library use it
if(${ZE_EXTERNAL_BUILD_DEBUG_POSTFIX})
	if(${ZE_EXTERNAL_MODEL_LOADING})
		set(ASSIMP_TARGET "${ASSIMP_TARGET}d")
	endif()
	set(FTYPE_TARGET "${FTYPE_TARGET}d")
	set(LIBPNG_TARGET "${LIBPNG_TARGET}d")
	set(ZLIB_TARGET "${ZLIB_TARGET}d")
endif()


######### COMPILE CONFIGURATION #########

set(ZE_CXX_STD cxx_std_23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
link_directories(${EXTERNAL_BIN_DIR})

# Platform specific flags
if(${ZE_PLATFORM_WINDOWS})
	add_definition_flags(
		"-DWIN32 -DWIN64 -D_WINDOWS -D_UNICODE -DUNICODE -DNOMINMAX -DZE_EXPORT=__declspec(dllexport) -DZE_IMPORT=__declspec(dllimport) -DVK_USE_PLATFORM_WIN32_KHR"
		"-DUSE_PIX"
		"-DUSE_PIX"
		"-DUSE_PIX"
		"")
elseif(${ZE_PLATFORM_LINUX})
	add_definitions("-DVK_USE_PLATFORM_WAYLAND_KHR -DVK_USE_PLATFORM_XCB_KHR -DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_DIRECTFB_EXT -DVK_USE_PLATFORM_XLIB_XRANDR_EXT")
	message(FATAL_ERROR "Building for unsupported platform!")
else()
	message(FATAL_ERROR "Building for unsupported platform!")
endif()
 
# Compiler specific flags
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,Development,Profile>:ProgramDatabase>")

if(${ZE_COMPILER_MSVC})
	set(LINKER_DEBUG_ENABLE_FLAGS "/DEBUG /DEBUGTYPE:CV")
	set(LINKER_DEVELOPMENT_FLAGS "/INCREMENTAL /LTCG:OFF /OPT:NOICF")
	add_linker_flags(
		"/MACHINE:x64 /CETCOMPAT:NO /DYNAMICBASE /HIGHENTROPYVA /LARGEADDRESSAWARE /GUARD:NO /MANIFEST:NO /NOLOGO /NXCOMPAT /SAFESEH:NO /SUBSYSTEM:WINDOWS /VERSION:${ZE_VER_MAJOR}.${ZE_VER_MINOR}"
		"${LINKER_DEBUG_ENABLE_FLAGS} ${LINKER_DEVELOPMENT_FLAGS} /OPT:NOREF"
		"${LINKER_DEBUG_ENABLE_FLAGS} ${LINKER_DEVELOPMENT_FLAGS} /OPT:REF"
		"${LINKER_DEBUG_ENABLE_FLAGS} /INCREMENTAL:NO /LTCG /OPT:REF,NOICF"
		"/DEBUG:NONE /INCREMENTAL:NO /LTCG /OPT:REF,ICF=8 /RELEASE")

	set(CXX_DEBUG_ENABLE_FLAGS "/Zi /Zf")
	set(CXX_DEVELOPMENT_FLAGS "/GL- /Gw- /sdl")
	set(CXX_OPTIMIZATION_FLAGS "/O2 /GL /Gw /Gy /MD /Ob2 /Oi /Ot /Oy /QIntel-jcc-erratum") # Maybe consider /Ob3, TODO: profile
	add_compilation_flags(
		"/arch:AVX2 /await /favor:blend /EHsc /fp:fast /fpcvt:IA /GA /GF /GR- /GS /guard:cf- /guard:ehcont- /J /JMC- /MP /nologo /openmp:experimental /permissive- /volatile:iso /Zc:__cplusplus /Zc:externConstexpr /Zc:inline /Zc:referenceBinding /Zc:rvalueCast /Zc:wchar_t /D_CRT_SECURE_NO_WARNINGS"
		"${CXX_DEBUG_ENABLE_FLAGS} ${CXX_DEVELOPMENT_FLAGS} /MDd /Gy- /Od /Oi- /Oy-"
		"${CXX_DEBUG_ENABLE_FLAGS} ${CXX_DEVELOPMENT_FLAGS} /MD /Gy /Ob1 /Oi /Oy"
		"${CXX_DEBUG_ENABLE_FLAGS} ${CXX_OPTIMIZATION_FLAGS}"
		"${CXX_OPTIMIZATION_FLAGS}")
	add_compile_options(/Wall
		/wd4061 # Not all enums are explicitly handled in switch where `default` is present
		/wd4100 # Unreferenced formal parameter
		/wd4191 # Using GetProcAddress() causes this and type casting Vulkan functions in volk
		/wd4255 # Would otherwise force declaration of func(void) when no arguments are specified
		/wd4263 # A class function definition has the same name as a virtual function in a base class but not the same number or type of arguments
		/wd4265 # Class with virtual functions does not have virutal destructor
		/wd4355 # 'this' used in base member initializer list
		/wd4365 # Signed/unsigned mismatch
		/wd4464 # Relative include path contains '..'
		/wd4514 # Unreferenced inline function has been removed
		/wd4582 # Constructor is not implicitly called
		/wd4583 # Destructor is not implicitly called
		/wd4623 # Default constructor was implicitly defined as deleted
		/wd4625 # Copy constructor was implicitly defined as deleted
		/wd4626 # Assignment operator was implicitly defined as deleted
		/wd4668 # Not defined as preprocessor macro in #if/#elif
		/wd4710 # Function not inlined
		/wd4711 # Function selected for automatic inline expansion
		/wd4800 # Implicit conversion from 'type' to bool
		/wd4820 # X bytes padding added after data member
		/wd4868 # Compiler may not enforce left-to-right evaluation order in braced initializer list, caused by entt::dense_map, just hide it
		/wd5026 # Move constructor was implicitly defined as deleted
		/wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
		/wd5027 # Move assignment operator was implicitly defined as deleted
		/wd5246 # The initialization of a subobject should be wrapped in braces
		#		Note: Caused by not respecing newer initializations aggregate, bug in MSVC
		/wd5262 # Currently MSVC standard library uses a lot of unguarded fall-through switch cases
		#		Note: Enable when fixed by Microsoft since it's bloating output beyond measure
		/wd5264 # Currently MSVC standard library contains a lot of unsused variables
		#		Note: Enable when fixed by Microsoft since it's bloating output beyond measure
	)
elseif(${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC})
	# GCC asumed although they should work on Clang too
	set(ZE_CXX_FLAGS "${ZE_CXX_FLAGS} -Wall -Wextra -Wpessimizing-move -Wredundant-move -pedantic -ffast-math")
	set(ZE_CXX_FLAGS_DEBUG "${ZE_CXX_FLAGS_DEBUG} -Og")
	set(ZE_CXX_FLAGS_DEVELOPMENT "${ZE_CXX_FLAGS_DEVELOPMENT} -O2 -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=2")
	#set(CXX_PERFORMANCE_FLAGS "-O3 -flto -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=2")
else()
	message(FATAL_ERROR "Using unsupported compiler!")
endif()

# General flags
add_definitions(-DSPNG_SSE=2)
add_definitions(-DSPNG_STATIC)
add_definitions(-DZLIB_CONST)
add_definitions(-DENTT_NOEXCEPTION)
add_definitions(-DENTT_ID_TYPE=$<IF:$<BOOL:${ZE_USE_WIDE_ENTITY_ID}>,std::uint64_t,std::uint32_t>)
add_definitions(-DVK_NO_PROTOTYPES)
add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
add_definitions(-DIMGUI_USER_CONFIG=<${ENGINE_INC_DIR}/GUI/ImGuiConfig.h>)
# EnTT is causing errors due to usage of std::aligned_storage, to be removed when it's fixed in lib
add_definitions(-D_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)
# General flags with project options
add_definitions(-D_ZE_EXTERNAL_MODEL_LOADING=$<BOOL:${ZE_EXTERNAL_MODEL_LOADING}>)
add_definitions(-D_ZE_RENDERER_NO_SPLIT_BARRIERS=$<BOOL:${ZE_RENDERER_NO_SPLIT_BARRIERS}>)
add_definitions(-D_ZE_RENDER_GRAPH_SINGLE_THREAD=$<BOOL:${ZE_RENDERER_SINGLE_THREAD}>)
add_definitions(-D_ZE_DEBUG_FRAME_MEMORY_PRINT=$<BOOL:${ZE_DEBUG_FRAME_MEMORY_PRINT}>)
add_definitions(-D_ZE_DEBUG_FRAME_NO_ALIASING_MEMORY=$<BOOL:${ZE_DEBUG_FRAME_NO_ALIASING_MEMORY}>)
add_definitions(-D_ZE_DEBUG_GFX_API=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>>)
add_definitions(-D_ZE_DEBUG_GFX_NAMES=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>>)
add_definitions(-D_ZE_GFX_MARKERS=$<NOT:$<BOOL:${ZE_BUILD_RELEASE}>>)
# Platform type
add_definitions(-D_ZE_PLATFORM_WINDOWS=$<BOOL:${ZE_PLATFORM_WINDOWS}> -D_ZE_PLATFORM_LINUX=$<BOOL:${ZE_PLATFORM_LINUX}> -D_ZE_PLATFORM_ANDROID=$<BOOL:${ZE_PLATFORM_ANDROID}>)
add_definitions(-D_ZE_PLATFORM_FUCHSIA=$<BOOL:${ZE_PLATFORM_FUCHSIA}> -D_ZE_PLATFORM_NSWITCH=$<BOOL:${ZE_PLATFORM_NSWITCH}>)
# Compiler type
add_definitions(-D_ZE_COMPILER_MSVC=$<BOOL:${ZE_COMPILER_MSVC}> -D_ZE_COMPILER_CLANG=$<BOOL:${ZE_COMPILER_CLANG}> -D_ZE_COMPILER_GCC=$<BOOL:${ZE_COMPILER_GCC}>)
# RHI type
add_definitions(-D_ZE_RHI_DX11=$<BOOL:${ZE_ENABLE_DX11}> -D_ZE_RHI_DX12=$<BOOL:${ZE_ENABLE_DX12}> -D_ZE_RHI_GL=$<BOOL:${ZE_ENABLE_GL}> -D_ZE_RHI_VK=$<BOOL:${ZE_ENABLE_VK}>)
# Engine version
add_definitions(-D_ZE_VERSION_MAJOR=${ZE_VER_MAJOR} -D_ZE_VERSION_MINOR=${ZE_VER_MINOR} -D_ZE_VERSION_PATCH=${ZE_VER_PATCH})
# Build type
add_definitions(-D_ZE_MODE_DEBUG=$<BOOL:${ZE_BUILD_DEBUG}> -D_ZE_MODE_DEV=$<BOOL:${ZE_BUILD_DEVELOPMENT}> -D_ZE_MODE_PROFILE=$<BOOL:${ZE_BUILD_PROFILE}> -D_ZE_MODE_RELEASE=$<BOOL:${ZE_BUILD_RELEASE}>)
add_definition_flags(
	""
	"-D_DEBUG"
	"-DNDEBUG"
	"-DNDEBUG"
	"-DNDEBUG")

# Shader flags
set(SHADER_FLAGS "/nologo /E main /Ges /Zpc /DFFX_GPU=1 /DFFX_HLSL=1 /DNIS_HLSL=1 /I \"${EXT_SHADER_INC_DIR}\" /I \"${FFXSDK_INC_DIR}/FidelityFX/gpu\" /I \"${NIS_INC_DIR}\"")
if(${ZE_BUILD_DEBUG} OR ${ZE_BUILD_DEVELOPMENT} OR ${ZE_BUILD_PROFILE})
	set(SHADER_FLAGS "${SHADER_FLAGS} /Zi")
	if(NOT ${ZE_BUILD_PROFILE})
		set(SHADER_FLAGS "${SHADER_FLAGS} /Od")
	endif()
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

 # libspng
set(LIBSPNG_CACHE_ARGS "-DENABLE_OPT:BOOL=ON"
	"-DSPNG_SHARED:BOOL=OFF"
	"-DSPNG_STATIC:BOOL=ON"
	"-DBUILD_EXAMPLES:BOOL=OFF"
	"-DZLIB_INCLUDE_DIRS:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_LIBRARIES:STRING=${ZLIB_TARGET}"
	"-DZLIB_LIBRARY:STRING=${ZLIB_TARGET}")
add_external_project(LIBSPNG "" "" "ZLIB")

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
if(${ZE_EXTERNAL_MODEL_LOADING})
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
if(${ZE_COMPILER_MSVC})
	# Fix for compilation error due to too big files
	set(HARFBUZZ_CACHE_ARGS ${HARFBUZZ_CACHE_ARGS}
		"-DCMAKE_CXX_FLAGS:STRING=/bigobj")
endif()
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

# FidelityFX SDK
add_external_fidelityfx_effect(FSR1)
add_external_fidelityfx_effect(FSR2)
add_external_fidelityfx_effect(CACAO)
add_external_fidelityfx_effect(SSSR)

# EnTT
if(${ZE_COMPILER_MSVC})
	set(ENTT_INCLUDE_NATVIS ON CACHE BOOL "Use debug natvis files" FORCE)
elseif(${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC})
	set(ENTT_USE_SANITIZER ON CACHE BOOL "Enable address sanitizer" FORCE)
endif()
set(ENTT_INCLUDE_HEADERS ON CACHE BOOL "Include header for dependency" FORCE)
add_subdirectory(${ENTT_DIR})

# volk
if(${ZE_ENABLE_VK})
	if(${ZE_PLATFORM_WINDOWS})
		set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR CACHE STRING "Enabled Vulkan for Windows" FORCE)
	else()
		message(FATAL_ERROR "Building for unsupported platform!")
	endif()
	add_subdirectory(${VOLK_DIR})
endif()