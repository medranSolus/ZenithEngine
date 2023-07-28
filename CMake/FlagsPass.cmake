include_guard(DIRECTORY)

# Macro for passing arguments into specified function for flags
#	ADD_FUNC = function that appends flags
macro(append_flags ADD_FUNC)
	separate_arguments(GLOBAL_FLAGS NATIVE_COMMAND "${GLOBAL_FLAGS}")
	cmake_language(CALL ${ADD_FUNC} ${GLOBAL_FLAGS})
	if(${ZE_BUILD_DEBUG})
		separate_arguments(DEBUG_FLAGS NATIVE_COMMAND "${DEBUG_FLAGS}")
		cmake_language(CALL ${ADD_FUNC} ${DEBUG_FLAGS})
	elseif(${ZE_BUILD_DEVELOPMENT})
		separate_arguments(DEV_FLAGS NATIVE_COMMAND "${DEV_FLAGS}")
		cmake_language(CALL ${ADD_FUNC} ${DEV_FLAGS})
	elseif(${ZE_BUILD_PROFILE})
		separate_arguments(PROFILE_FLAGS NATIVE_COMMAND "${PROFILE_FLAGS}")
		cmake_language(CALL ${ADD_FUNC} ${PROFILE_FLAGS})
	elseif(${ZE_BUILD_RELEASE})
		separate_arguments(RELEASE_FLAGS NATIVE_COMMAND "${RELEASE_FLAGS}")
		cmake_language(CALL ${ADD_FUNC} ${RELEASE_FLAGS})
	else()
		message(FATAL_ERROR "Unknow build type!")
	endif()
endmacro()

# Function for passing compilation flags based on build type
#	GLOBAL_FLAGS = flags used by all build targets
#	DEBUG_FLAGS = flags used by DEBUG target
#   DEV_FLAGS = flags used by DEVELOPMENT target
#   PROFILE_FLAGS = flags used by PROFILE target
#   RELEASE_FLAGS = flags used by RELEASE target
function(add_compilation_flags GLOBAL_FLAGS DEBUG_FLAGS DEV_FLAGS PROFILE_FLAGS RELEASE_FLAGS)
	append_flags(add_compile_options)
endfunction()

# Function for passing definitions based on build type
#	GLOBAL_FLAGS = flags used by all build targets
#	DEBUG_FLAGS = flags used by DEBUG target
#   DEV_FLAGS = flags used by DEVELOPMENT target
#   PROFILE_FLAGS = flags used by PROFILE target
#   RELEASE_FLAGS = flags used by RELEASE target
function(add_definition_flags GLOBAL_FLAGS DEBUG_FLAGS DEV_FLAGS PROFILE_FLAGS RELEASE_FLAGS)
	append_flags(add_definitions)
endfunction()

# Function for passing linker flags based on build type
#	GLOBAL_FLAGS = flags used by all build targets
#	DEBUG_FLAGS = flags used by DEBUG target
#   DEV_FLAGS = flags used by DEVELOPMENT target
#   PROFILE_FLAGS = flags used by PROFILE target
#   RELEASE_FLAGS = flags used by RELEASE target
function(add_linker_flags GLOBAL_FLAGS DEBUG_FLAGS DEV_FLAGS PROFILE_FLAGS RELEASE_FLAGS)
	append_flags(add_link_options)
endfunction()