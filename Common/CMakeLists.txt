cmake_minimum_required(VERSION 3.19)

# Target files
set(SRC_DIR "${COMMON_DIR}/Source")
set(PCH "${COMMON_INC_DIR}/pch.h")
file(GLOB_RECURSE SRC_LIST
    "${SRC_DIR}/*.cpp"
    "${COMMON_INC_DIR}/*.h")

# Target setup
add_library(${COMMON_TARGET} STATIC ${SRC_LIST})

target_compile_features(${COMMON_TARGET} PUBLIC ${HE_CXX_STD})
target_include_directories(${COMMON_TARGET}
    PUBLIC ${COMMON_INC_DIR} ${DXMATH_INC_DIR} ${DXTEX_INC_DIR})
target_precompile_headers(${COMMON_TARGET} PUBLIC ${PCH})
target_link_libraries(${COMMON_TARGET} PUBLIC ${DXTEX_TARGET})