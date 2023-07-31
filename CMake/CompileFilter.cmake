include_guard(DIRECTORY)

# Macro for compiling only relevant files for current configuration of OS+RHI
#	SRC_LIST = prefix of all project variables
#	INC_DIR = different path for library file
#	SRC_DIR = different path for MSVC debug symbols
macro(current_config_compile_filter SRC_LIST INC_DIR SRC_DIR)
	list(FILTER SRC_LIST EXCLUDE REGEX \/Platform\/|\/RHI\/)

	if(${ZE_PLATFORM_WINDOWS})
		file(GLOB_RECURSE OS_SRC_LIST
			"${SRC_DIR}/Platform/WinAPI/*.cpp"
			"${INC_DIR}/Platform/WinAPI/*.h")
	else()
		message(FATAL_ERROR "Building for unsupported platform!")
	endif()
	list(APPEND SRC_LIST ${OS_SRC_LIST})

	if(${ZE_ENABLE_DX11} OR ${ZE_ENABLE_DX12})
		file(GLOB_RECURSE DX_SRC_LIST
			"${SRC_DIR}/RHI/DX/*.cpp"
			"${INC_DIR}/RHI/DX/*.h")
		list(APPEND SRC_LIST ${DX_SRC_LIST})

		if(${ZE_ENABLE_DX11})
			file(GLOB_RECURSE DX11_SRC_LIST
				"${SRC_DIR}/RHI/DX11/*.cpp"
				"${INC_DIR}/RHI/DX11/*.h")
			list(APPEND SRC_LIST ${DX11_SRC_LIST})
		endif()
		if(${ZE_ENABLE_DX12})
			file(GLOB_RECURSE DX12_SRC_LIST
				"${SRC_DIR}/RHI/DX12/*.cpp"
				"${INC_DIR}/RHI/DX12/*.h")
			list(APPEND SRC_LIST ${DX12_SRC_LIST})
		endif()
	endif()
	if(${ZE_ENABLE_VK})
		file(GLOB_RECURSE VK_SRC_LIST
			"${SRC_DIR}/RHI/VK/*.cpp"
			"${INC_DIR}/RHI/VK/*.h")
		list(APPEND SRC_LIST ${VK_SRC_LIST})
	endif()
endmacro()