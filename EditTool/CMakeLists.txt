cmake_minimum_required(VERSION 3.19)

# Target files
set(SRC_DIR "${TOOL_DIR}/Source")
set(INC_DIR "${TOOL_DIR}/Include")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${INC_DIR}/*.h")

if(ZE_CI_JOB)
    set(BUILD_EXCLUDE FALSE)
else()
    set(BUILD_EXCLUDE FALSE)
endif()

# Target setup
add_executable(${TOOL_TARGET} ${SRC_LIST})
set_target_properties(${TOOL_TARGET} PROPERTIES
    EXCLUDE_FROM_ALL ${BUILD_EXCLUDE}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ZE_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ZE_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${ZE_BIN_DIR})

target_compile_features(${TOOL_TARGET} PRIVATE ${ZE_CXX_STD})
target_include_directories(${TOOL_TARGET} PRIVATE ${INC_DIR} ${JSON_INC_DIR})
target_link_libraries(${TOOL_TARGET} PRIVATE ${COMMON_TARGET})