cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})
cmake_policy(SET CMP0091 NEW) # Setting of MSVC runtime library

######### PROJECT CONFIGURATION #########

set(ZE_PROJECT_NAME "Zenith Engine")
set(ZE_VER_MAJOR 0)
set(ZE_VER_MINOR 5)
set(ZE_VER_PATCH 0)
project(${ZE_PROJECT_NAME} VERSION ${ZE_VER_MAJOR}.${ZE_VER_MINOR}.${ZE_VER_PATCH})

# Project options
set(ZE_BIN_DIR "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for project data (shaders, resources, executables)")
set(ZE_BUILD_DIR "${PROJECT_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}" CACHE STRING
	"Output directory for build files")
set(ZE_ICON_ID 101 CACHE STRING "Icon identyficator for Windows builds,
	should be set with icon ID from your .rc file")
set(ZE_NGX_ID 0 CACHE STRING "Unique identifier used for initialization
	of Nvidia NGX interface, should be set with the one assigned from Nvidia per app basis
	(contact Nvidia before release here: https://developer.nvidia.com/sw-notification).
	When set to 0 for development, engine UUID will be used.")

option(ZE_ENABLE_DX11 "Enable DirectX 11 API usage" OFF)
option(ZE_ENABLE_DX12 "Enable DirectX 12 API usage" ON)
option(ZE_ENABLE_GL "Enable OpenGL API usage" OFF)
option(ZE_ENABLE_VK "Enable Vulkan API usage" OFF)
option(ZE_ENABLE_XAUDIO2 "Enable XAudio2 API usage" ON)
option(ZE_ENABLE_OPENAL "Enable OpenAL API usage" ON)
option(ZE_EXTERNAL_MODEL_LOADING "Enable external model loading module of engine, making it possible to load custom file formats" ON)
option(ZE_USE_WIDE_ENTITY_ID "Forces usage of 64bit enitity ID (when large number of enities is possible)" OFF)
option(ZE_RENDERER_SINGLE_THREAD "Turns off recording commands by multiple threads in the renderer" ON)
option(ZE_RENDERER_NO_SPLIT_BARRIERS "Turns off splitting barriers and performs them immediatelly" ON)
option(ZE_RENDERER_CREATION_VALIDATION "Turns on validation during whole process of render graph creation, enabled by default in development builds but can be forced even in release" ON)
option(ZE_BUILD_TOOLS "Enable building of command line utility tool" ON)
option(ZE_BUILD_DEMO "Enable building of technological demo" ON)
option(ZE_NO_DATA "Disable copying of default engine assets" ON)
option(ZE_CI_JOB "Build engine for CI job" OFF)
option(ZE_USE_FFX_API_FSR_SHADERS "Instead of using shader provided by the legacy FidelityFX SDK v1.1.4, use the one supplied with the newest FFX API" ON)

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
	set(ZE_ENABLE_XAUDIO2 OFF CACHE BOOL "XAudio2 not supported for Linux" FORCE)
elseif(NOT (${ZE_PLATFORM_WINDOWS}))
	message(FATAL_ERROR "Building for unsupported platform [${CMAKE_SYSTEM_NAME}]!")
endif()
if(NOT (${ZE_COMPILER_MSVC} OR ${ZE_COMPILER_CLANG} OR ${ZE_COMPILER_GCC}))
	message(FATAL_ERROR "Using unsupported compiler [${CMAKE_CXX_COMPILER}]!")
endif()
if(NOT (${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12} OR ${ZE_ENABLE_VK}))
	message(FATAL_ERROR "No RHI backend enabled, at least one must be active!")
endif()
if(NOT (${ZE_ENABLE_XAUDIO2} OR ${ZE_ENABLE_OPENAL}))
	message(FATAL_ERROR "No AHI backend enabled, at least one must be active!")
endif()

# Check flags provided to CMake by toolchains
if(FALSE)
	display_cmake_flags()
endif()


########### MAIN DIRECTORIES ############

set(COMMON_DIR "${PROJECT_SOURCE_DIR}/Common")
set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/Engine")
set(TOOLS_DIR "${PROJECT_SOURCE_DIR}/Tools")
set(DEMO_DIR "${PROJECT_SOURCE_DIR}/Demo")
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(EXTERNAL_BIN_DIR "${EXTERNAL_DIR}/Bin/${CMAKE_BUILD_TYPE}")

# External
set(AGILITY_DIR "${EXTERNAL_DIR}/AgilitySDK")
set(AGS_DIR "${EXTERNAL_DIR}/AGS")
set(ASSIMP_DIR "${EXTERNAL_DIR}/Assimp")
set(BROTLI_DIR "${EXTERNAL_DIR}/brotli")
set(BZIP2_DIR "${EXTERNAL_DIR}/bzip2")
set(DLSS_DIR "${EXTERNAL_DIR}/DLSS")
set(DSTORE_DIR "${EXTERNAL_DIR}/DirectStorage")
set(DXMATH_DIR "${EXTERNAL_DIR}/DirectXMath")
set(ENTT_DIR "${EXTERNAL_DIR}/EnTT")
set(FTYPE_DIR "${EXTERNAL_DIR}/FreeType")
set(FFXAPI_DIR "${EXTERNAL_DIR}/FidelityFXSDK/Kits/FidelityFX")
set(FFXAPI_DLL_DIR "${FFXAPI_DIR}/signedbin")
set(FFXSDK_DIR "${EXTERNAL_DIR}/FidelityFXSDK_Legacy/sdk")
set(HARFBUZZ_DIR "${EXTERNAL_DIR}/HarfBuzz")
set(IMGUI_DIR "${EXTERNAL_DIR}/ImGui")
set(LIBPNG_DIR "${EXTERNAL_DIR}/libpng")
set(LIBSPNG_DIR "${EXTERNAL_DIR}/libspng")
set(NIS_DIR "${EXTERNAL_DIR}/NvidiaImageScaling")
set(OPENAL_DIR "${EXTERNAL_DIR}/OpenAL")
set(VOLK_DIR "${EXTERNAL_DIR}/volk")
set(PIX_DIR "${EXTERNAL_DIR}/WinPixEventRuntime")
set(XAUDIO2_DIR "${EXTERNAL_DIR}/XAudio2")
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
set(BROTLI_INC_DIR "${BROTLI_DIR}/c/include")
set(BZIP2_INC_DIR "${BZIP2_DIR}")
set(DLSS_INC_DIR "${DLSS_DIR}/include")
set(DSTORE_INC_DIR "${DSTORE_DIR}/include")
set(DXMATH_INC_DIR "${DXMATH_DIR}/Inc")
set(ENTT_INC_DIR "${ENTT_DIR}/src")
set(FTYPE_INC_DIR "${FTYPE_DIR}/include")
set(FFXAPI_INC_DIR "${FFXAPI_DIR}/api/include")
set(FFXAPI_UPSCALE_INC_DIR "${FFXAPI_DIR}/upscalers/include")
set(FFXAPI_FRAMEGEN_INC_DIR "${FFXAPI_DIR}/framegeneration/include")
set(FFXAPI_DENOISER_INC_DIR "${FFXAPI_DIR}/denoisers/include")
set(FFXAPI_RADCACHE_INC_DIR "${FFXAPI_DIR}/radiancecache/include")
set(FFXSDK_INC_DIR "${FFXSDK_DIR}/include")
set(HARFBUZZ_INC_DIR "${HARFBUZZ_DIR}/src")
set(IMGUI_INC_DIR "${IMGUI_DIR}")
set(LIBPNG_INC_DIR "${LIBPNG_DIR}")
set(LIBSPNG_INC_DIR "${LIBSPNG_DIR}/spng")
set(NIS_INC_DIR "${NIS_DIR}/NIS")
set(OPENAL_INC_DIR "${NIS_DIR}/include")
set(VOLK_INC_DIR "${VOLK_DIR}")
set(PIX_INC_DIR "${PIX_DIR}/Include")
set(XAUDIO2_INC_DIR "${XAUDIO2_DIR}/include")
set(XESS_INC_DIR "${XESS_DIR}/inc")
set(QOI_INC_DIR "${QOI_DIR}/include")
set(ZLIB_INC_DIR "${ZLIB_DIR}")


############# CUSTOM CMAKE MODULES ##############

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")
include(BoolAssign)
include(CompileFilter)
include(External)
include(FidelityFX)
include(FlagsDisplay)
include(FlagsPass)
include(PostBuild)
include(ShaderBuild)


############# EXTERNAL FEATURES ##############

set_bool(ZE_AGILITY_ENABLED ${ZE_ENABLE_DX12})
set_bool(ZE_AGS_ENABLED ${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12})
set_bool(ZE_ASSIMP_ENABLED ${ZE_EXTERNAL_MODEL_LOADING})
set_bool(ZE_DLSS_ENABLED (${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12}) OR ${ZE_ENABLE_VK})
set_bool(ZE_DSTORE_ENABLED ${ZE_ENABLE_DX12})
set_bool(ZE_FFXAPI_ENABLED ${ZE_ENABLE_DX12})
set_bool(ZE_FFXAPI_ENABLED_GEN ${ZE_ENABLE_DX12})
set_bool(ZE_VOLK_ENABLED ${ZE_ENABLE_VK})
set_bool(ZE_PIX_ENABLED ${ZE_ENABLE_DX12} OR NOT ${ZE_BUILD_RELEASE})
set_bool(ZE_XESS_ENABLED (${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12}) OR ${ZE_ENABLE_VK})


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
	"${BROTLI_INC_DIR}"
	"${BZIP2_INC_DIR}"
	"${DLSS_INC_DIR}"
	"${DSTORE_INC_DIR}"
	"${DXMATH_INC_DIR}"
	"${DXTEX_INC_DIR}"
	"${ENTT_INC_DIR}"
	"${FTYPE_INC_DIR}"
	"${FFXAPI_INC_DIR}"
	"${FFXAPI_UPSCALE_INC_DIR}"
	"${FFXAPI_FRAMEGEN_INC_DIR}"
	"${FFXAPI_DENOISER_INC_DIR}"
	"${FFXAPI_RADCACHE_INC_DIR}"
	"${FFXSDK_INC_DIR}"
	"${HARFBUZZ_INC_DIR}"
	"${IMGUI_INC_DIR}"
	"${LIBPNG_INC_DIR}"
	"${LIBSPNG_INC_DIR}"
	"${NIS_INC_DIR}"
	"${OPENAL_INC_DIR}"
	"${VOLK_INC_DIR}"
	"${PIX_INC_DIR}"
	"${XAUDIO2_INC_DIR}"
	"${XESS_INC_DIR}"
	"${QOI_INC_DIR}"
	"${ZLIB_INC_DIR}")


################ TARGETS ################

if(${ZE_PLATFORM_WINDOWS})
	set(LIB_EXT ".lib")
	set(SHARED_LIB_EXT ".dll")
	set(LIB_PREFIX "")
else()
	set(LIB_EXT ".a")
	set(SHARED_LIB_EXT ".so")
	set(LIB_PREFIX "lib")
endif()

set(DEMO_TARGET "ZenithDemo")

# Libraries
set(COMMON_TARGET "common")
set(ENGINE_TARGET "zenith")

# External libraries
set(AGS_LIB "amd_ags_x64")
set(ASSIMP_LIB "assimp")
set(BROTLI_LIB "brotlicommon")
set(BROTLIDEC_LIB "brotlidec")
set(BROTLIENC_LIB "brotlienc")
set(BZIP2_LIB "bz2_static")
set(DLSS_LIB "nvsdk_ngx")
set(DSTORE_LIB "dstorage")
set(ENTT_LIB "EnTT::EnTT")
set(FTYPE_LIB "freetype")
set(HARFBUZZ_LIB "harfbuzz")
set(LIBPNG_LIB "png_static")
set(LIBSPNG_LIB "spng_static")
set(OPENAL_LIB "OpenAL32")
set(VOLK_LIB "volk")
set(PIX_LIB "WinPixEventRuntime")
set(XAUDIO2_LIB "xaudio2_9redist")
set(XESS_LIB "libxess")
set(ZLIB_LIB "zlibstatic")

if(${ZE_AGS_ENABLED})
	set(AGS_TARGET "${AGS_DIR}/ags_lib/lib/${AGS_LIB}.lib")
endif()
if(${ZE_ASSIMP_ENABLED})
	set(ASSIMP_TARGET "${ASSIMP_LIB}-vc${MSVC_TOOLSET_VERSION}-mt")
endif()
set(BROTLI_TARGET "${BROTLI_LIB}")
set(BROTLI_TARGETS "${BROTLI_LIB}" "${BROTLIDEC_LIB}" "${BROTLIENC_LIB}")
set(BZIP2_TARGET "${BZIP2_LIB}")
if(${ZE_DLSS_ENABLED})
	if(${ZE_PLATFORM_WINDOWS})
		set(DLSS_TARGET "${DLSS_DIR}/lib/Windows_x86_64/x64/${DLSS_LIB}_d")
		if(${ZE_BUILD_DEBUG})
			set(DLSS_TARGET "${DLSS_TARGET}_dbg")
		endif()
		set(DLSS_TARGET "${DLSS_TARGET}.lib")
	elseif(${ZE_PLATFORM_LINUX})
		set(DLSS_TARGET "${DLSS_DIR}/lib/Linux_x86_64/lib${DLSS_LIB}.a")
	endif()
endif()
if(${ZE_DSTORE_ENABLED})
	set(DSTORE_TARGET "${DSTORE_DIR}/lib/x64/${DSTORE_LIB}.lib")
endif()
set(ENTT_TARGET "${ENTT_LIB}")
set(FTYPE_TARGET "${FTYPE_LIB}")
set(HARFBUZZ_TARGET "${HARFBUZZ_LIB}")
set(LIBPNG_TARGET "libpng16_static") # Will need to increase number based on version
set(LIBSPNG_TARGET "${LIBSPNG_LIB}")
if(${ZE_ENABLE_OPENAL})
	set(OPENAL_TARGET "${OPENAL_LIB}")
endif()
if(${ZE_VOLK_ENABLED})
	set(VOLK_TARGET "${VOLK_LIB}")
endif()
if(${ZE_PIX_ENABLED})
	set(PIX_TARGET "${PIX_DIR}/bin/x64/${PIX_LIB}.lib")
endif()
if(${ZE_ENABLE_XAUDIO2})
	set(XAUDIO2_TARGETS "${XAUDIO2_DIR}/release/lib/x64/${XAUDIO2_LIB}.lib"
		"${XAUDIO2_DIR}/release/lib/x64/xapobaseredist.lib")
endif()
if(${ZE_XESS_ENABLED})
	set(XESS_TARGET "${XESS_DIR}/lib/${XESS_LIB}.lib")
endif()
set(ZLIB_TARGET "zs")

# FidelityFX SDK effects
add_fidelityfx_target(CACAO)
add_fidelityfx_target(DENOISER)
add_fidelityfx_target(FSR1)
add_fidelityfx_target(FSR2)
add_fidelityfx_target(FSR3UPSCALER)
add_fidelityfx_target_dep(SSSR "DENOISER")
add_fidelityfx_target(SSSR)

# Inject debug postfix if library use it
if(${ZE_EXTERNAL_BUILD_DEBUG_POSTFIX})
	if(${ZE_ASSIMP_ENABLED})
		set(ASSIMP_TARGET "${ASSIMP_TARGET}d")
	endif()
	set(FTYPE_TARGET "${FTYPE_TARGET}d")
	set(LIBPNG_TARGET "${LIBPNG_TARGET}d")
	set(ZLIB_TARGET "${ZLIB_TARGET}d")
endif()

set(ENGINE_EXTERNAL_TARGETS ${AGS_TARGET}
	${ASSIMP_TARGET}
	${BROTLI_TARGETS}
	${BZIP2_TARGET}
	${DLSS_TARGET}
	${DSTORE_TARGET}
	${ENTT_TARGET}
	${FFXSDK_TARGETS}
	${FTYPE_TARGET}
	${HARFBUZZ_TARGET}
	${LIBPNG_TARGET}
	${LIBSPNG_TARGET}
	${OPENAL_TARGET}
	${PIX_TARGET}
	${XAUDIO2_TARGETS}
	${XESS_TARGET}
	${VOLK_TARGET}
	${ZLIB_TARGET})


######### COMPILE CONFIGURATION #########

set(ZE_CXX_STD cxx_std_23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
link_directories(${EXTERNAL_BIN_DIR})

# Platform specific flags
if(${ZE_PLATFORM_WINDOWS})
	add_definition_flags(
		"WIN32 WIN64 _WINDOWS _UNICODE UNICODE NOMINMAX ZE_EXPORT=__declspec(dllexport) ZE_IMPORT=__declspec(dllimport) VK_USE_PLATFORM_WIN32_KHR"
		"USE_PIX"
		"USE_PIX"
		"USE_PIX"
		"")
elseif(${ZE_PLATFORM_LINUX})
	add_compile_definitions(VK_USE_PLATFORM_WAYLAND_KHR VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_DIRECTFB_EXT VK_USE_PLATFORM_XLIB_XRANDR_EXT)
	message(FATAL_ERROR "Building for unsupported platform!")
else()
	message(FATAL_ERROR "Building for unsupported platform!")
endif()
 
# Compiler specific flags
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,Development,Profile>:ProgramDatabase>")

if(${ZE_COMPILER_MSVC})
	set(LINKER_DEBUG_ENABLE_FLAGS "/DEBUG /DEBUGTYPE:CV")
	set(LINKER_DEVELOPMENT_FLAGS "/LTCG:OFF /OPT:NOICF")
	add_linker_flags(
		"/MACHINE:x64 /CETCOMPAT:NO /DYNAMICBASE /HIGHENTROPYVA /LARGEADDRESSAWARE /GUARD:NO /MANIFEST:NO /NOLOGO /NXCOMPAT /SAFESEH:NO /VERSION:${ZE_VER_MAJOR}.${ZE_VER_MINOR}"
		"${LINKER_DEBUG_ENABLE_FLAGS} ${LINKER_DEVELOPMENT_FLAGS} /INCREMENTAL /OPT:NOREF"
		"${LINKER_DEBUG_ENABLE_FLAGS} ${LINKER_DEVELOPMENT_FLAGS} /INCREMENTAL:NO /OPT:REF"
		"${LINKER_DEBUG_ENABLE_FLAGS} /INCREMENTAL:NO /LTCG /OPT:REF,NOICF"
		"/DEBUG:NONE /INCREMENTAL:NO /LTCG /OPT:REF,ICF=8 /RELEASE")

	set(CXX_DEBUG_ENABLE_FLAGS "/Zi /Zf")
	set(CXX_DEVELOPMENT_FLAGS "/GL- /Gw- /sdl")
	set(CXX_OPTIMIZATION_FLAGS "/O2 /GL /Gw /Gy /MD /Ob2 /Oi /Ot /Oy /QIntel-jcc-erratum") # Maybe consider /Ob3, TODO: profile
	add_compilation_flags(
		"/arch:AVX2 /favor:blend /EHsc /fp:fast /fpcvt:IA /GA /GF /GR- /GS /guard:cf- /guard:ehcont- /J /JMC- /MP /nologo /openmp:experimental /permissive- /volatile:iso /Zc:enumTypes /Zc:__cplusplus /Zc:externConstexpr /Zc:inline /Zc:referenceBinding /Zc:rvalueCast /Zc:wchar_t /D_CRT_SECURE_NO_WARNINGS"
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
		/wd4864 # Expected 'template' keyword before dependent template name
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
add_compile_definitions(SPNG_SSE=2)
add_compile_definitions(SPNG_STATIC)
add_compile_definitions(ZLIB_CONST)
add_compile_definitions(ENTT_NOEXCEPTION)
add_compile_definitions(ENTT_ID_TYPE=$<IF:$<BOOL:${ZE_USE_WIDE_ENTITY_ID}>,std::uint64_t,std::uint32_t>)
add_compile_definitions(VK_NO_PROTOTYPES)
add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)
add_compile_definitions(IMGUI_USER_CONFIG=<${ENGINE_INC_DIR}/GUI/ImGuiConfig.h>)
add_compile_definitions(ZE_NGX_ID=${ZE_NGX_ID})
# EnTT is causing errors due to usage of std::aligned_storage, to be removed when it's fixed in lib
add_compile_definitions(_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)
# Flags for enabling features per given target
add_compile_definitions(_ZE_FFXAPI_ENABLED=$<BOOL:${ZE_FFXAPI_ENABLED}>)
add_compile_definitions(_ZE_DLSS_ENABLED=$<BOOL:${ZE_DLSS_ENABLED}>)
add_compile_definitions(_ZE_XESS_ENABLED=$<BOOL:${ZE_XESS_ENABLED}>)
# General flags with project options
add_compile_definitions(_ZE_USE_FFX_API_FSR_SHADERS=$<BOOL:${ZE_USE_FFX_API_FSR_SHADERS}>)
add_compile_definitions(_ZE_FSR2_SHADERS_VERSION_MAJOR=2)
add_compile_definitions(_ZE_FSR2_SHADERS_VERSION_MINOR=3)
add_compile_definitions(_ZE_FSR2_SHADERS_VERSION_PATCH=4)
add_compile_definitions(_ZE_FSR3_SHADERS_VERSION_MAJOR=3)
add_compile_definitions(_ZE_FSR3_SHADERS_VERSION_MINOR=1)
add_compile_definitions(_ZE_FSR3_SHADERS_VERSION_PATCH=5)
add_compile_definitions(_ZE_EXTERNAL_MODEL_LOADING=$<BOOL:${ZE_EXTERNAL_MODEL_LOADING}>)
add_compile_definitions(_ZE_RENDERER_CREATION_VALIDATION=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>,$<BOOL:${ZE_RENDERER_CREATION_VALIDATION}>>)
add_compile_definitions(_ZE_RENDERER_NO_SPLIT_BARRIERS=$<BOOL:${ZE_RENDERER_NO_SPLIT_BARRIERS}>)
add_compile_definitions(_ZE_RENDER_GRAPH_SINGLE_THREAD=$<BOOL:${ZE_RENDERER_SINGLE_THREAD}>)
add_compile_definitions(_ZE_DEBUG_GFX_API=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>>)
add_compile_definitions(_ZE_DEBUG_GFX_NAMES=$<OR:$<BOOL:${ZE_BUILD_DEBUG}>,$<BOOL:${ZE_BUILD_DEVELOPMENT}>>)
add_compile_definitions(_ZE_GFX_MARKERS=$<NOT:$<BOOL:${ZE_BUILD_RELEASE}>>)
# Platform type
add_compile_definitions(_ZE_PLATFORM_WINDOWS=$<BOOL:${ZE_PLATFORM_WINDOWS}> _ZE_PLATFORM_LINUX=$<BOOL:${ZE_PLATFORM_LINUX}> _ZE_PLATFORM_ANDROID=$<BOOL:${ZE_PLATFORM_ANDROID}>)
add_compile_definitions(_ZE_PLATFORM_FUCHSIA=$<BOOL:${ZE_PLATFORM_FUCHSIA}> _ZE_PLATFORM_NSWITCH=$<BOOL:${ZE_PLATFORM_NSWITCH}> _ZE_PLATFORM_XBOX_SCARLET=$<BOOL:${ZE_PLATFORM_XBOX_SCARLET}>)
# Compiler type
add_compile_definitions(_ZE_COMPILER_MSVC=$<BOOL:${ZE_COMPILER_MSVC}> _ZE_COMPILER_CLANG=$<BOOL:${ZE_COMPILER_CLANG}> _ZE_COMPILER_GCC=$<BOOL:${ZE_COMPILER_GCC}>)
# RHI type
add_compile_definitions(_ZE_RHI_DX11=$<BOOL:${ZE_ENABLE_DX11}> _ZE_RHI_DX12=$<BOOL:${ZE_ENABLE_DX12}> _ZE_RHI_GL=$<BOOL:${ZE_ENABLE_GL}> _ZE_RHI_VK=$<BOOL:${ZE_ENABLE_VK}>)
# Engine version
add_compile_definitions(_ZE_VERSION_MAJOR=${ZE_VER_MAJOR} _ZE_VERSION_MINOR=${ZE_VER_MINOR} _ZE_VERSION_PATCH=${ZE_VER_PATCH})
# Build type
add_compile_definitions(_ZE_MODE_DEBUG=$<BOOL:${ZE_BUILD_DEBUG}> _ZE_MODE_DEV=$<BOOL:${ZE_BUILD_DEVELOPMENT}> _ZE_MODE_PROFILE=$<BOOL:${ZE_BUILD_PROFILE}> _ZE_MODE_RELEASE=$<BOOL:${ZE_BUILD_RELEASE}>)
add_definition_flags(
	""
	"_DEBUG"
	"NDEBUG"
	"NDEBUG"
	"NDEBUG")

# Shader flags
set(SHADER_FLAGS "/nologo /E main /Ges /Zpc /DZE_USE_FFX_API_FSR2_SHADERS=${ZE_USE_FFX_API_FSR_SHADERS} /DZE_USE_FFX_API_FSR3_SHADERS=${ZE_USE_FFX_API_FSR_SHADERS} /DFFX_GPU=1 /DFFX_HLSL=1 /DNIS_HLSL=1 /I \"${EXT_SHADER_INC_DIR}\" /I \"${FFXSDK_INC_DIR}/FidelityFX/gpu\" /I \"${FFXAPI_DIR}\" /I \"${NIS_INC_DIR}\"")
if(${ZE_BUILD_DEBUG} OR ${ZE_BUILD_DEVELOPMENT} OR ${ZE_BUILD_PROFILE})
	set(SHADER_FLAGS "${SHADER_FLAGS} /Zi")
	if(NOT ${ZE_BUILD_PROFILE})
		set(SHADER_FLAGS "${SHADER_FLAGS} /Od")
	endif()
endif()
set(SHADER_FLAGS "${SHADER_FLAGS} /D_ZE_PLATFORM_WINDOWS=${ZE_PLATFORM_WINDOWS}")
set(SHADER_FLAGS "${SHADER_FLAGS} /D_ZE_PLATFORM_LINUX=${ZE_PLATFORM_LINUX}")
set(SHADER_FLAGS "${SHADER_FLAGS} /D_ZE_PLATFORM_ANDROID=${ZE_PLATFORM_ANDROID}")
set(SHADER_FLAGS "${SHADER_FLAGS} /D_ZE_PLATFORM_FUCHSIA=${ZE_PLATFORM_FUCHSIA}")
set(SHADER_FLAGS "${SHADER_FLAGS} /D_ZE_PLATFORM_NSWITCH=${ZE_PLATFORM_NSWITCH}")
set(SHADER_FLAGS "${SHADER_FLAGS} /D_ZE_PLATFORM_XBOX_SCARLET=${ZE_PLATFORM_XBOX_SCARLET}")


############## SUBPROJECTS ##############

add_subdirectory(${COMMON_DIR})
add_subdirectory(${ENGINE_DIR})
if(${ZE_BUILD_TOOLS})
	add_subdirectory(${TOOLS_DIR})
endif()
if(${ZE_BUILD_DEMO})
	add_subdirectory(${DEMO_DIR})
endif()


########### EXTERNAL PROJECTS ###########

# zlib
set(ZLIB_CACHE_ARGS "-DZLIB_BUILD_TESTING:BOOL=OFF"
	"-DZLIB_BUILD_SHARED:BOOL=OFF")
add_external_project(ZLIB "" "" "" "")
copy_inner_file(ZLIB HeaderConfig "" "zconf.h")

 # libspng
set(LIBSPNG_CACHE_ARGS "-DENABLE_OPT:BOOL=ON"
	"-DSPNG_SHARED:BOOL=OFF"
	"-DSPNG_STATIC:BOOL=ON"
	"-DBUILD_EXAMPLES:BOOL=OFF"
	"-DZLIB_FOUND:BOOL=ON"
	"-DZLIB_INCLUDE_DIRS:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_LIBRARIES:STRING=${ZLIB_TARGET}"
	"-DZLIB_LIBRARY:STRING=${ZLIB_TARGET}")
add_external_project(LIBSPNG "" "" "" "ZLIB")

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
add_external_project(LIBPNG "" "" "" "ZLIB")
copy_inner_file(LIBPNG HeaderConfig "" "pnglibconf.h")

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
	add_external_project(ASSIMP "lib/" "code/" "" "ZLIB")
	if(NOT EXISTS "${ASSIMP_OUT_LIB}")
		ExternalProject_Add_Step(${ASSIMP_TARGET} headerConfig
			DEPENDEES install
			LOG OFF
			COMMAND "${CMAKE_COMMAND}" -E copy_directory
			"${ASSIMP_BUILD_DIR}/include"
			"${EXTERNAL_BIN_DIR}")
	endif()
endif()

# OpenAL
if(${ZE_ENABLE_OPENAL})
	set(OPENAL_CACHE_ARGS "-DALSOFT_UTILS:BOOL=OFF"
		"-DALSOFT_NO_CONFIG_UTIL:BOOL=ON"
		"-DALSOFT_EXAMPLES:BOOL=OFF"
		"-DALSOFT_TESTS:BOOL=OFF"
		"-DALSOFT_INSTALL_CONFIG:BOOL=OFF"
		"-DALSOFT_INSTALL_HRTF_DATA:BOOL=OFF"
		"-DALSOFT_INSTALL_AMBDEC_PRESETS:BOOL=OFF"
		"-DALSOFT_INSTALL_EXAMPLES:BOOL=OFF"
		"-DALSOFT_INSTALL_UTILS:BOOL=OFF"
		"-DALSOFT_UPDATE_BUILD_VERSION:BOOL=OFF"
		"-DALSOFT_EAX:BOOL=OFF"
		"-DALSOFT_SEARCH_INSTALL_DATADIR:BOOL=OFF"
		"-DZLIB_FOUND:BOOL=OFF"
		"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR}"
		"-DZLIB_LIBRARIES:STRING=${ZLIB_TARGET}"
		"-DZLIB_LIBRARY:STRING=${ZLIB_TARGET}")
	add_external_project(OPENAL "" "" "../../" "ZLIB")
	copy_inner_file(OPENAL SharedLib "" "${OPENAL_LIB}${SHARED_LIB_EXT}")
endif()

# HarfBuzz
set(HARFBUZZ_CACHE_ARGS "-DBUILD_FRAMEWORK:BOOL=OFF"
	"-DHB_HAVE_DIRECTWRITE:BOOL=ON")
if(${ZE_COMPILER_MSVC})
	# Fix for compilation error due to too big files
	set(HARFBUZZ_CACHE_ARGS ${HARFBUZZ_CACHE_ARGS}
		"-DCMAKE_CXX_FLAGS:STRING=/bigobj")
endif()
add_external_project(HARFBUZZ "" "" "" "")

# brotli
set(BROTLI_CACHE_ARGS "-DBROTLI_BUNDLED_MODE:BOOL=ON"
	"-DBROTLI_DISABLE_TESTS:BOOL=ON"
	"-DBROTLI_BUILD_TOOLS:BOOL=OFF"
	"-DBUILD_SHARED_LIBS:BOOL=OFF"
	"-DENABLE_SANITIZER:BOOL=OFF")
add_external_project(BROTLI "" "" "" "")
copy_inner_lib(BROTLI BrotliDecoder "" "${BROTLIDEC_LIB}")
copy_inner_lib(BROTLI BrotliEncoder "" "${BROTLIENC_LIB}")

# bzip2
set(BZIP2_CACHE_ARGS "-DENABLE_LIB_ONLY:BOOL=ON"
	"-DENABLE_STATIC_LIB:BOOL=ON"
	"-DENABLE_TESTS:BOOL=OFF"
	"-DENABLE_DOCS:BOOL=OFF"
	"-DENABLE_WERROR:BOOL=OFF"
	"-DENABLE_DEBUG:BOOL=OFF"
	"-DENABLE_SHARED_LIB:BOOL=OFF"
	"-DBROTLI_DISABLE_TESTS:BOOL=ON")
add_external_project(BZIP2 "" "" "" "")

# FreeType
set(FTYPE_CACHE_ARGS "-DSKIP_INSTALL_ALL:BOOL=ON"
	"-DFT_DISABLE_ZLIB:BOOL=OFF"
	"-DFT_REQUIRE_ZLIB:BOOL=ON"
	"-DZLIB_FOUND:BOOL=ON"
	"-DZLIB_INCLUDE_DIRS:STRING=${ZLIB_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DZLIB_INCLUDE_DIR:STRING=${ZLIB_INC_DIR}"
	"-DZLIB_LIBRARIES:STRING=${ZLIB_TARGET}"
	"-DZLIB_LIBRARY:STRING=${ZLIB_TARGET}"
	"-DFT_DISABLE_PNG:BOOL=OFF"
	"-DFT_REQUIRE_PNG:BOOL=ON"
	"-DPNG_FOUND:BOOL=ON"
	"-DPNG_INCLUDE_DIRS:STRING=${LIBPNG_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DPNG_PNG_INCLUDE_DIR:STRING=${LIBPNG_INC_DIR}"
	"-DPNG_LIBRARIES:STRING=${LIBPNG_TARGET}"
	"-DPNG_LIBRARY:STRING=${LIBPNG_TARGET}"
	"-DFT_DISABLE_HARFBUZZ:BOOL=OFF"
	"-DFT_REQUIRE_HARFBUZZ:BOOL=ON"
	"-DFT_DYNAMIC_HARFBUZZ:BOOL=OFF"
	"-DHarfBuzz_FOUND:BOOL=ON"
	"-DHarfBuzz_INCLUDE_DIRS:STRING=${HARFBUZZ_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DHarfBuzz_INCLUDE_DIR:STRING=${HARFBUZZ_INC_DIR}"
	"-DHarfBuzz_LIBRARIES:STRING=${HARFBUZZ_TARGET}"
	"-DHarfBuzz_LIBRARY:STRING=${HARFBUZZ_TARGET}"
	"-DFT_DISABLE_BROTLI:BOOL=OFF"
	"-DFT_REQUIRE_BROTLI:BOOL=ON"
	"-DBROTLIDEC_FOUND:BOOL=ON"
	"-DBROTLIDEC_INCLUDE_DIRS:STRING=${BROTLI_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DBROTLIDEC_INCLUDE_DIR:STRING=${BROTLI_INC_DIR}"
	"-DBROTLIDEC_LIBRARIES:STRING=${BROTLI_TARGETS}"
	"-DBROTLIDEC_LIBRARY:STRING=${BROTLI_TARGETS}"
	"-DFT_DISABLE_BZIP2:BOOL=OFF"
	"-DFT_REQUIRE_BZIP2:BOOL=ON"
	"-DBZIP2_FOUND:BOOL=ON"
	"-DBZIP2_INCLUDE_DIRS:STRING=${BZIP2_INC_DIR};${EXTERNAL_BIN_DIR}"
	"-DBZIP2_INCLUDE_DIR:STRING=${BZIP2_INC_DIR}"
	"-DBZIP2_LIBRARIES:STRING=${BZIP2_TARGET}"
	"-DBZIP2_LIBRARY:STRING=${BZIP2_TARGET}")
add_external_project(FTYPE "" "" "" "LIBPNG;HARFBUZZ;BROTLI;BZIP2")

# FidelityFX SDK
add_external_fidelityfx_effect(CACAO)
add_external_fidelityfx_effect(DENOISER)
add_external_fidelityfx_effect(FSR1)
add_external_fidelityfx_effect(FSR2)
add_external_fidelityfx_effect(FSR3UPSCALER)
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