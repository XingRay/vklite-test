file(GLOB_RECURSE IMAGE_SRC_LIST
        *.cpp
)

add_library(image ${IMAGE_SRC_LIST})

target_include_directories(image
        PUBLIC ..
)

target_link_libraries(image
        PUBLIC stb vklite
)