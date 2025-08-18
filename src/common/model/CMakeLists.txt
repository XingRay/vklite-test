file(GLOB_RECURSE MODEL_SRC_LIST
        *.cpp
)

add_library(model ${MODEL_SRC_LIST})

target_include_directories(model
        PUBLIC ..
)

target_link_libraries(model
        PUBLIC tinyobjloader math
)