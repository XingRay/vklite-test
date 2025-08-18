file(GLOB_RECURSE MATH_SRC_LIST
        *.cpp
)

add_library(math ${MATH_SRC_LIST})

target_include_directories(math
        PUBLIC ..
)

target_link_libraries(math
        PUBLIC glm
)