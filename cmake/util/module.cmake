# 将指定目录(root_dir)下的所有子目录中符合 pattern 的子目录通过 add_subdirectory 指令添加到 cmake 中
function(addSubdirectories root_dir pattern)
    message("addSubdirectories: root_dir:${root_dir}, pattern:${pattern}")

    # 扫描根目录下的所有文件和目录
    file(GLOB dirs
            ${root_dir}/*
    )

    message("sub dirs:${dirs}")

    # 添加所有测试用例
    foreach (dir_PATH IN LISTS dirs)
        message("dir_PATH:${dir_PATH}")
        get_filename_component(dir_NAME ${dir_PATH} NAME)
        message("dir_NAME:${dir_NAME}")
        if (IS_DIRECTORY ${dir_PATH} AND (${dir_NAME} MATCHES ${pattern}))
            set(CURRENT_DIR_NAME ${dir_NAME})
            message("CURRENT_DIR_NAME: ${CURRENT_DIR_NAME}")
            add_subdirectory(${dir_PATH})
        endif ()
    endforeach ()

endfunction()