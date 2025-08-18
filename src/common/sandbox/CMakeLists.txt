# 收集源代码
file(GLOB_RECURSE SRC_LIST
        "*.cpp"
        "*.c++"
        "*.c"
        "*.cc"
)

# 创建公共库（包含common代码）
add_library(sandbox STATIC
        ${SRC_LIST}
)

target_include_directories(sandbox
        PUBLIC ..
)

target_link_libraries(sandbox
        PUBLIC vklite glm
)