function(copyDirToDir target_name src_dir dst_dir)
    message("copyDirToDir: target_name:${target_name}, src_dir:${src_dir}, dst_dir:${dst_dir}")
    if (NOT EXISTS ${src_dir})
        message("src_dir:${src_dir} not exist !")
        return()
    endif ()
    if (NOT IS_DIRECTORY ${src_dir})
        message("src_dir:${src_dir} is not dir !")
        return()
    endif ()

    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${dst_dir}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${src_dir}"
            "${dst_dir}"
            COMMENT "Copying ${src_dir} to ${dst_dir}"
    )

endfunction()

function(copySourceDirToTargetDir target_name src_dir dst_dir)
    message("copySourceDirToTargetDir: target_name:${target_name}, src_dir:${src_dir}, dst_dir:${dst_dir}")

    set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/${src_dir})
    message("src_dir:${src_dir}")
    set(dst_dir $<TARGET_FILE_DIR:${target_name}>/${dst_dir})
    message("dst_dir:${dst_dir}")

    copyDirToDir(${target_name} ${src_dir} ${dst_dir})

endfunction()

function(copyFileToTargetDir target_name filePath)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${filePath}
            "$<TARGET_FILE_DIR:${target_name}>/"
            COMMENT "Copying file:[${filePath}] to target directory"
    )
endfunction()

function(copyProjectFileToTargetDir filePath)
    copyFileToTargetDir(${PROJECT_NAME} ${filePath})
endfunction()