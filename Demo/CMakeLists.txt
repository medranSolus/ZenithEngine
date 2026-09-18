cmake_minimum_required(VERSION ${ZE_CMAKE_VERSION})

# Resource embedding
if(${ZE_PLATFORM_WINDOWS})
    enable_language(RC)
    set(RES_FILE "${ZE_BUILD_DIR}/resource.rc")
    file(WRITE ${RES_FILE}
        "${ZE_ICON_ID} ICON \"${DEMO_DIR}/Assets/icon.ico\"")
endif()

# Copy or instrument the information about assets location
if(${ZE_STANDALONE_BUILD})
    set(ASSETS_PATH "")
    copy_runtime_data(DEMO_ASSETS "${ASSETS_BUILD_DIR}" "" "" "")
else()
    set(ASSETS_BUILD_DIR "${DEMO_DIR}/Assets/Runtime/")
    file(RELATIVE_PATH ASSETS_PATH "${ZE_BUILD_DIR}" "${ASSETS_BUILD_DIR}")
    set(ASSETS_PATH "${ASSETS_PATH}/")
endif()
add_compile_definitions(ZE_DEMO_ASSETS_PATH="${ASSETS_PATH}")

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
add_dependencies(${DEMO_TARGET} ${DEMO_ASSETS_COPY_TARGET})

# Assets preprocessing
add_subdirectory(Assets)