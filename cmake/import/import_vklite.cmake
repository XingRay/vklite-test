# vklite
# https://github.com/XingRay/vklite
# set(vklite_install_dir "D:/code/git/github/vklite/build/dist/vklite" CACHE STRING "vklite install dir")
function(import_vklite vklite_install_dir)
    message("import_vklite: vklite_install_dir: ${vklite_install_dir}")
    set(vklite_DIR ${vklite_install_dir}/lib/cmake)
    find_package(vklite)
    if (NOT TARGET vklite)
        message(FATAL_ERROR "vklite is not target !")
    endif ()
endfunction()
