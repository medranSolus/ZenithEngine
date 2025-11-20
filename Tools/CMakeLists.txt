cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})

macro(create_tools_project)
    get_filename_component(TOOL_TARGET ${CMAKE_CURRENT_LIST_DIR} NAME)
    
    set(SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/Source")
    set(INC_DIR "${CMAKE_CURRENT_LIST_DIR}/Include")

    # Target files
    set(JSON "${EXT_INC_DIR}/json.hpp")
    file(GLOB_RECURSE SRC_LIST
        "${SRC_DIR}/*.cpp"
        "${INC_DIR}/*.h")

    if(${ZE_CI_JOB})
        set(BUILD_EXCLUDE FALSE)
    else()
        set(BUILD_EXCLUDE TRUE)
    endif()

    # Target setup
    add_executable(${TOOL_TARGET} ${SRC_LIST} ${JSON})
    set_target_properties(${TOOL_TARGET} PROPERTIES
        EXCLUDE_FROM_ALL ${BUILD_EXCLUDE}
        RUNTIME_OUTPUT_DIRECTORY "${ZE_BIN_DIR}/Tools")

    target_compile_features(${TOOL_TARGET} PRIVATE ${ZE_CXX_STD})
    target_include_directories(${TOOL_TARGET} PRIVATE ${INC_DIR} ${EXT_INC_DIR})
    target_link_libraries(${TOOL_TARGET} PRIVATE ${COMMON_TARGET})
endmacro()

add_subdirectory(BrdfGen)
add_subdirectory(CubeConv)
add_subdirectory(TexEdit)