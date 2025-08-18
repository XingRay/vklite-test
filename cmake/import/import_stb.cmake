# stb
# https://github.com/nothings/stb
# set(stb_install_dir "D:/develop/opengl/stb" CACHE STRING "stb install dir")
function(import_stb stb_install_dir)
    add_library(stb INTERFACE)

    target_include_directories(stb
            INTERFACE ${stb_install_dir}
    )
endfunction()
