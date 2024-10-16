include_guard(DIRECTORY)

# Macro for showing default flags used by CMAKE. 
# Relevant when adding new compiler or platform and determining correct set of flags
macro(display_cmake_flags)
	message("========== Standard C language flags ==========")
	message("CMAKE_C_FLAGS_INIT: ${CMAKE_C_FLAGS_INIT}")
	message("CMAKE_C_FLAGS_DEBUG_INIT: ${CMAKE_C_FLAGS_DEBUG_INIT}")
	message("CMAKE_C_FLAGS_RELWITHDEBINFO_INIT: ${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}")
	message("CMAKE_C_FLAGS_MINSIZEREL_INIT: ${CMAKE_C_FLAGS_MINSIZEREL_INIT}")
	message("CMAKE_C_FLAGS_RELEASE_INIT: ${CMAKE_C_FLAGS_RELEASE_INIT}")
	message("")

	
	message("========== Standard C++ language flags ==========")
	message("CMAKE_CXX_FLAGS_INIT: ${CMAKE_CXX_FLAGS_INIT}")
	message("CMAKE_CXX_FLAGS_DEBUG_INIT: ${CMAKE_CXX_FLAGS_DEBUG_INIT}")
	message("CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")
	message("CMAKE_CXX_FLAGS_MINSIZEREL_INIT: ${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
	message("CMAKE_CXX_FLAGS_RELEASE_INIT: ${CMAKE_CXX_FLAGS_RELEASE_INIT}")
	message("")

	
	message("========== Flags used by linker when building executable ==========")
	message("CMAKE_EXE_LINKER_FLAGS_INIT: ${CMAKE_EXE_LINKER_FLAGS_INIT}")
	message("CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT: ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT}")
	message("CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT: ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT}")
	message("CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT: ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT}")
	message("CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT: ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT}")
	message("")

	
	message("========== Flags used by linker when building static library ==========")
	message("CMAKE_STATIC_LINKER_FLAGS_INIT: ${CMAKE_STATIC_LINKER_FLAGS_INIT}")
	message("CMAKE_STATIC_LINKER_FLAGS_DEBUG_INIT: ${CMAKE_STATIC_LINKER_FLAGS_DEBUG_INIT}")
	message("CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO_INIT: ${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO_INIT}")
	message("CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL_INIT: ${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL_INIT}")
	message("CMAKE_STATIC_LINKER_FLAGS_RELEASE_INIT: ${CMAKE_STATIC_LINKER_FLAGS_RELEASE_INIT}")
	message("")

	
	message("========== Flags used by linker when building module ==========")
	message("CMAKE_MODULE_LINKER_FLAGS_INIT: ${CMAKE_MODULE_LINKER_FLAGS_INIT}")
	message("CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT: ${CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT}")
	message("CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT: ${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT}")
	message("CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL_INIT: ${CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL_INIT}")
	message("CMAKE_MODULE_LINKER_FLAGS_RELEASE_INIT: ${CMAKE_MODULE_LINKER_FLAGS_RELEASE_INIT}")
	message("")

	
	message("========== Flags used by linker when building shared library ==========")
	message("CMAKE_SHARED_LINKER_FLAGS_INIT: ${CMAKE_SHARED_LINKER_FLAGS_INIT}")
	message("CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT: ${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT}")
	message("CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT: ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT}")
	message("CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL_INIT: ${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL_INIT}")
	message("CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT: ${CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT}")
	message("")
endmacro()