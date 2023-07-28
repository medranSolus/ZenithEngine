cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})
include(PostBuild)

# Resource embedding
if(${ZE_PLATFORM_WINDOWS})
    enable_language(RC)
    set(RES_FILE "${ZE_BUILD_DIR}/resource.rc")
    file(WRITE ${RES_FILE}
            "${ZE_ICON_ID} ICON \"${DEMO_DIR}/Data/icon.ico\"")
endif()

# Post build data copy
if(NOT ${ZE_CI_JOB})
    copy_runtime_data(DEMO "Data/Runtime" "" "" "")
endif()

# Target files
set(INC_DIR "${DEMO_DIR}/Include")
set(SRC_DIR "${DEMO_DIR}/Source")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${INC_DIR}/*.h")

# Target setup
add_executable(${DEMO_TARGET} WIN32 ${SRC_LIST} ${RES_FILE} ${ZE_API_HEADER})
set_target_properties(${DEMO_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZE_BIN_DIR})

target_compile_features(${DEMO_TARGET} PRIVATE ${ZE_CXX_STD})
target_include_directories(${DEMO_TARGET} PRIVATE ${INC_DIR} ${ZE_INC_DIRS})
target_link_libraries(${DEMO_TARGET} PRIVATE ${ENGINE_TARGET})
if(NOT ZE_CI_JOB)
    add_dependencies(${DEMO_TARGET} ${DEMO_COPY_TARGET})
endif()