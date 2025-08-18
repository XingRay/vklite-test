# glfw
# https://github.com/glfw/glfw
# set(glfw_install_dir "D:/develop/opengl/glfw/glfw-3.4.bin.WIN64" CACHE STRING "glfw install dir")
function(import_glfw glfw_install_dir)
    message("import_glfw: glfw_install_dir:${glfw_install_dir}")
    if (IS_DIRECTORY ${glfw_install_dir})
        set(glfw_dir ${glfw_install_dir})
    endif ()
    message("glfw_dir:${glfw_dir}")

    set(GLFW_LIB_DIR ${glfw_dir}/lib-vc2022)
    set(GLFW_INCLUDE_DIR ${glfw_dir}/include)

    add_library(glfw STATIC IMPORTED)

    set_target_properties(glfw PROPERTIES
            IMPORTED_LOCATION "${GLFW_LIB_DIR}/glfw3.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
    )

endfunction()
