# build test
message("build: ${CURRENT_TEST_NAME}")
message("PROJECT_NAME: ${PROJECT_NAME}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# 收集源代码
file(GLOB_RECURSE SRC_LIST
        "*.cpp"
        "*.c++"
        "*.c"
        "*.cc"
)

add_executable(${CURRENT_TEST_NAME}
        ${SRC_LIST}
)

target_link_libraries(${CURRENT_TEST_NAME}
        PRIVATE vklite sandbox math util
)

include("../../cmake/util/shader.cmake")
compileShaderToTargetDir(${CURRENT_TEST_NAME} "shader" "shader")
