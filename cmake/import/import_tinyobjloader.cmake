# tinyobjloader
# https://github.com/tinyobjloader/tinyobjloader
# set(tinyobjloader_install_dir "D:/develop/opengl/tinyobjloader" CACHE STRING "tinyobjloader install dir")
function(import_tinyobjloader tinyobjloader_install_dir)
    add_library(tinyobjloader INTERFACE)
    target_include_directories(tinyobjloader
            INTERFACE ${tinyobjloader_install_dir}
    )
endfunction()