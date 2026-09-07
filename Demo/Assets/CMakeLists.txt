cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})

# Building assets only in release config
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	setup_assets_vars("${CMAKE_CURRENT_SOURCE_DIR}/Source" "${CMAKE_CURRENT_SOURCE_DIR}/Runtime" "${ZE_BIN_DIR}/Tools")

	# Copy files that don't need special processing for now
	copy_assets("Audio/" "")
	copy_assets("Fonts/" "")
	copy_assets("imgui_default" "")

	generate_brdf("brdf_gen.json" "Lightmaps/")
	merge_cubemaps("Skybox/")
	process_hdris("HDRI/cubemap_hdri_process.json" "HDRI/cubemap_conv.json" "Skybox/" "Lightmaps/")

	add_assets_target("ZenithDemoAssets")
endif()