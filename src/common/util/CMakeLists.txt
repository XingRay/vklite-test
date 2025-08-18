file(GLOB_RECURSE UTIL_SRC_LIST
        *.cpp
        *.c++
        *.cxx
        *.c
)

add_library(util
        ${UTIL_SRC_LIST}
)

target_include_directories(util
        PUBLIC ..
)
