# 添加测试用例的公共依赖
add_subdirectory(common)

# 自动添加测试
file(GLOB dirs
        *
)
message("test dirs:${dirs}")

# 添加所有测试用例
foreach (dir_PATH IN LISTS dirs)
    message("dir_PATH:${dir_PATH}")
    get_filename_component(dir_NAME ${dir_PATH} NAME)
    message("dir_NAME:${dir_NAME}")
    if (IS_DIRECTORY ${dir_PATH} AND (${dir_NAME} MATCHES "^test"))
        message("test dir_NAME:${dir_NAME}")
        set(CURRENT_TEST_NAME ${dir_NAME})
        add_subdirectory(${dir_PATH})
    endif ()
endforeach ()