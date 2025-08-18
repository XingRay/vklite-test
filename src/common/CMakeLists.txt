file(GLOB dirs
        *
)
message("common dirs:${dirs}")

# 添加所有子模块
foreach (dir_PATH IN LISTS dirs)
    get_filename_component(dir_NAME ${dir_PATH} NAME)
    if (IS_DIRECTORY ${dir_PATH})
        add_subdirectory(${dir_PATH})
    endif ()
endforeach ()
