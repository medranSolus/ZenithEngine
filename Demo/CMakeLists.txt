cmake_minimum_required(VERSION 3.19)

# Resource embedding
if(WIN32)
    enable_language(RC)
    set(RES_FILE "${BUILD_DIR}/resource.rc")
    file(WRITE ${RES_FILE}
            "${ZE_ICON_ID} ICON \"${DEMO_DIR}/Data/icon.ico\"")
endif()

# Post build data copy
copy_runtime_data(DEMO "Data/Runtime")

# Target files
set(INC_DIR "${DEMO_DIR}/Include")
set(SRC_DIR "${DEMO_DIR}/Source")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${INC_DIR}/*.h")

# Target setup
add_executable(${DEMO_TARGET} WIN32 ${SRC_LIST} ${RES_FILE})
set_target_properties(${DEMO_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ZE_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ZE_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${ZE_BIN_DIR})

target_compile_features(${DEMO_TARGET} PRIVATE ${ZE_CXX_STD})
target_include_directories(${DEMO_TARGET} PRIVATE ${INC_DIR})
target_link_libraries(${DEMO_TARGET} PRIVATE ${ENGINE_TARGET})
add_dependencies(${DEMO_TARGET} ${DEMO_COPY_TARGET})