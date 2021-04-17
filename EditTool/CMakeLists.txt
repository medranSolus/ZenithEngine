cmake_minimum_required(VERSION 3.19)

# Target files
set(SRC_DIR "${TOOL_DIR}/Source")
set(INC_DIR "${TOOL_DIR}/Include")
file(GLOB_RECURSE SRC_LIST
    ${SRC_DIR}/*.cpp
    ${INC_DIR}/*.h)

# Target setup
add_executable(${TOOL_TARGET} ${SRC_LIST})
set_target_properties(${TOOL_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BIN_DIR})

target_compile_features(${TOOL_TARGET} PRIVATE ${HE_CXX_STD})
target_include_directories(${TOOL_TARGET} PRIVATE ${INC_DIR} ${JSON_INC_DIR})
target_link_libraries(${TOOL_TARGET} PRIVATE ${COMMON_TARGET})