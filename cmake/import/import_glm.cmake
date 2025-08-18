# glm
# https://github.com/g-truc/glm
# set(glm_install_dir "D:/develop/opengl/glm/glm-1.0.1-light" CACHE STRING "glm install dir")
function(import_glm glm_install_dir)
    message("import_glm: glm_install_dir:${glm_install_dir}")
    add_library(glm INTERFACE)

    target_include_directories(glm
            INTERFACE ${glm_install_dir}
    )
endfunction()
